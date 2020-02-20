/**
 *   pFactory, a generic library for designing parallel solvers.
 *   Copyright (C) 2019 Artois University and CNRS
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <mutex>
#include <chrono>
#include <thread>

#include "Groups.h"
#include "Controller.h"

namespace pFactory{
    unsigned int Group::groupCount = 0;

    Group::Group(unsigned int pnbThreads):
        barrier(pnbThreads),
        winnerId(UINT_MAX),
        CurrentTaskIdPerThread(pnbThreads, 0),
        testStop(false),
        idGroup(Group::groupCount++),
        nbThreads(pnbThreads),
        nbLaunchedTasks(0),
        nbTasks(0),
        concurrentMode(false),
	    startedBarrier(NULL),
	    waitingThreads(NULL),
        hasStarted(false),
        hasWaited(false),
        concurrentGroupsModes(false),
        taskPopFront(false),
        controller(NULL)
    {
        startedBarrier = new Barrier(pnbThreads+1);
        for (unsigned int i = 0;i<pnbThreads;i++)threads.push_back(new std::thread(&Group::wrapperFunction,this));
        if(VERBOSE)
            printf("c [pFactory][Group N°%d] created (threads:%d).\n",idGroup,pnbThreads);
    }

    Group::Group(const Group& toCopy):
        Group(toCopy.getNbThreads())
    {}


    
    void Group::reload(){
        if (!hasStarted or !hasWaited){
            tasksIdToRun.clear(); //First clean all tasks
            tasks.clear();
            startedBarrier->wait(); //Free the barrier
            wait(); //Join all threads
        }
        testStop=false;
        nbLaunchedTasks=0;
        concurrentMode=false;
        hasStarted=false;
        hasWaited=false;
        winnerId = UINT_MAX;
        //The barrier
        delete startedBarrier;
        startedBarrier = new Barrier(nbThreads+1);
    }
    

    void Group::start(){
        if(VERBOSE) {
            printf("c [pFactory][Group N°%d] concurrent mode: %s.\n", idGroup, concurrentMode ? "enabled" : "disabled");
            printf("c [pFactory][Group N°%d] computations in progress (threads:%d - tasks:%d).\n", idGroup, nbThreads, (int)getNbTasks());
        }
        hasStarted=true;
        startedBarrier->wait();
        
    }

    void Group::add(const std::function<int()>& function){
        std::unique_lock<std::mutex> tasksLock(tasksMutex);
        tasks.push_back(Task(nbTasks, function));
        tasksIdToRun.push_back(nbTasks);
        nbTasks++;
        if(VERBOSE)
            printf("c [pFactory][Group N°%d] new task added (threads:%d - tasks:%d).\n",idGroup,nbThreads,(int)getNbTasks());
    }

    int Group::wait(){
        hasWaited = true;
        for(unsigned int i = 0; i < nbThreads; i++){
            if(threads[i]->joinable()){
                threads[i]->join();
                if(VERBOSE)
                    printf("c [pFactory][Group N°%d] Thread N°%d is joined.\n",idGroup,i);
            }
        }
        if(concurrentMode){
            if(VERBOSE)
                printf("c [pFactory][Group N°%d] Return Code of the winner:%d (Thread N°%d)\n",idGroup,getWinner().getReturnCode(),getWinner().getThreadId());
            return getWinner().getReturnCode();
        }
	    return 0;
    }

    int Group::kill(){
        stop();
        return wait();
    }

    void Group::wrapperWaitting(unsigned int seconds){
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        stop();
    }
    
    int Group::wait(unsigned int seconds){
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        std::unique_lock<std::mutex> tasksLock(tasksMutex);
        if(concurrentMode && winnerId != UINT_MAX){
            tasksLock.unlock();   
            return wait();
        }
        return -1;
    }

    int Group::waitAndKill(unsigned int seconds){
        waitingThreads=new std::thread(&Group::wrapperWaitting,this,seconds);
        return wait();
    }


    void Group::wrapperFunction(){
        //Create a wrapper unique lock for the mutex 
        std::unique_lock<std::mutex> tasksLock(tasksMutex,std::defer_lock);
        // wait that the user calls start() para:
        startedBarrier->wait();
        //Take a task
        while(true){
            tasksLock.lock();
            //if there are no more tasks
            if(!tasksIdToRun.size() || testStop){
                return;
            }
            
            //Get a task
            unsigned int taskId = 0;
            if(taskPopFront){
                taskId = tasksIdToRun.front();
                tasksIdToRun.pop_front();
            }else{
                taskId = tasksIdToRun.back();
                tasksIdToRun.pop_back();
            }
            const std::function<int()> function = tasks[taskId].getFunction();
            CurrentTaskIdPerThread[getThreadId()] = tasks[taskId].getId();
            tasks[taskId].setStatus(pFactory::Status::inProgress);
            tasks[taskId].setThreadId(getThreadId());
            nbLaunchedTasks++;
            
            tasksLock.unlock();
            
            //Launch a task  
            if(VERBOSE)
                printf("c [pFactory][Group N°%d] task %d launched on thread %d.\n",getId(),getTaskId(),getThreadId());
            int returnCode = function();  
            
            tasksLock.lock();
            tasks[taskId].setStatus(pFactory::Status::terminated);
            tasks[getTaskId()].setReturnCode(returnCode);
            
            if(concurrentMode && winnerId == UINT_MAX){
                winnerId = getTaskId();
                stop();
                if(concurrentGroupsModes){
                    Controller::mutex.lock();
                    if(controller->getWinner() == NULL){
                         controller->setWinner(this);
                    } 
                    std::vector<Group*> groups = controller->getGroups();
                    for (auto& group: groups) group->stop();
                    Controller::mutex.unlock();
                }
                if(VERBOSE)
                    printf("c [pFactory][Group N°%d] concurent mode: thread %d has won with the task %d.\n",getId(),getThreadId(),getTaskId());
                return;

            }
            tasksLock.unlock();
        }   
    }
}
