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
#include "Groups.h"
#include "../include/Barrier.h"
#include <iostream>
#include <mutex>
#include <chrono>
#include <thread>



namespace pFactory{
    
    int Group::groupCount = 0;
    
    Group::Group(unsigned int pnbThreads):
        barrier(pnbThreads),
        winner(UINT_MAX, UINT_MAX, UINT_MAX),
        currentTasksId(pnbThreads, 0),
        testStop(false),
        idGroup(Group::groupCount++),
        nbThreads(pnbThreads),
        nbTasks(0),
        nbLaunchedTasks(0),
        concurrentMode(false),
	    startedBarrier(NULL),
	    waitingThreads(NULL),
        hasStarted(false),
        hasWaited(false)
    {
        startedBarrier = new Barrier(pnbThreads+1);
        for (unsigned int i = 0;i<pnbThreads;i++)threads.push_back(new std::thread(&Group::wrapperFunction,this));
        if(VERBOSE)
            printf("c [pFactory][Group N°%d] created (threads:%d).\n",idGroup,pnbThreads);
    }


    
    void Group::reload(){
        if (!hasStarted or !hasWaited){
            tasks.clear(); //First clean all tasks
            startedBarrier->wait(); //Free the barrier
            wait(); //Join all threads
        }
        testStop=false;
        nbLaunchedTasks=0;
        concurrentMode=false;
        hasStarted=false;
        hasWaited=false;
        winner.setWinner(UINT_MAX, UINT_MAX, UINT_MAX);
        //The barrier
        delete startedBarrier;
        startedBarrier = new Barrier(nbThreads+1);
        //Threads
        threads.clear();
        for (unsigned int i = 0;i<nbThreads;i++)threads.push_back(new std::thread(&Group::wrapperFunction,this));
        //Tasks
        tasks.clear();
        for (auto task : tasksSave)tasks.push_back(task);
        if(VERBOSE)
            printf("c [pFactory][Group N°%d] reload (threads:%d).\n",idGroup,nbThreads);
    }
    

    void Group::start(){
        if(VERBOSE) {
            printf("c [pFactory][Group N°%d] concurrent mode: %s.\n", idGroup, concurrentMode ? "enabled" : "disabled");
            printf("c [pFactory][Group N°%d] computations in progress (threads:%d - tasks:%d).\n", idGroup, nbThreads, (int) tasks.size());
        }
        hasStarted=true;
        startedBarrier->wait();
        
    }

    void Group::add(const std::function<int()> &function){

        nbTasks++;
        tasks.push_back(function);
        tasksSave.push_back(function);
        returnCodes.push_back(TASK_NOT_STARTED);
        if(VERBOSE)
            printf("c [pFactory][Group N°%d] new task added (threads:%d - tasks:%d).\n",idGroup,nbThreads,(int)tasks.size());
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
                printf("c [pFactory][Group N°%d] Return Code of the winner:%d (Thread N°%d)\n",idGroup,winner.getReturnCode(),winner.getThreadId());
            return winner.getReturnCode();
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
        if(concurrentMode && winner.getThreadId() != UINT_MAX){
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
        thread_local static unsigned int currentTaskId = 0;
        // wait that the user calls start() para:
        startedBarrier->wait();
        //Take a task
        while(true){
            tasksLock.lock();
            //if there are no more tasks
            if(!tasks.size() || testStop){
                return;
            }
            //Get a task
            std::function<int()> function = tasks.back();
            tasks.pop_back();
            currentTasksId[getThreadId()] = nbLaunchedTasks;
            currentTaskId = nbLaunchedTasks;
            nbLaunchedTasks++;
            tasksLock.unlock();
            //Launch a task  
            if(VERBOSE)
                printf("c [pFactory][Group N°%d] task %d launched on thread %d.\n",getGroupId(),getTaskId(),getThreadId());
            int returnCode = function();  
            tasksLock.lock();
            returnCodes[currentTaskId]=returnCode;
            tasksLock.unlock();
            
            //We have kill all others threads ! 
            tasksLock.lock(); 
            if(concurrentMode && winner.getThreadId() == UINT_MAX){
                winner.setWinner(getThreadId(), getTaskId(), returnCode);
                stop();
                if(VERBOSE)
                    printf("c [pFactory][Group N°%d] concurent mode: thread %d has won with the task %d.\n",getGroupId(),getGroupId(),getTaskId());
                return;

            }
            tasksLock.unlock();
        }   
    }
}
