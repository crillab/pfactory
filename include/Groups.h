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
#ifndef groups_H
#define groups_H

#include <stdio.h>
#include <thread>
#include <vector>
#include <functional>
#include <mutex>
#include <fstream>
#include <iostream>
#include <thread>
#include <algorithm>
#include <vector>
#include <queue>
#include <iterator>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <condition_variable>
#include <climits>
#include <deque>
#include <assert.h>

#include <stdarg.h> 

#include "Barrier.h"
#include "Task.h"


namespace pFactory {
    class Controller;
    const int VERBOSE = 0;

    
    /* An instance of the class group represent :
        - a set of threads (std::thread*)
        - a set of tasks (std::function<int())
    */
    class Group {
        static unsigned int groupCount; //To get the id of a group

    public:
        //Barrier for the user
        Barrier barrier;

        

        /* The constructor of a group
        \param pnbThreads the number of threads
        \return An instance of group
        */
        explicit Group(unsigned int pnbThreads);

        explicit Group(const Group&);

        Group const & operator=(Group &&g) = delete;


        ~Group() {
            if (!hasStarted){
                tasksIdToRun.clear(); //First clean all tasks
                tasks.clear();
                startedBarrier->wait(); //Free the barrier
                wait(); //Join all threads
            } else if (!hasWaited){
                wait(); //Join all threads
            }
            delete startedBarrier;
            delete waitingThreads;
            for(unsigned int i = 0; i < nbThreads; i++)
                delete threads[i];
        }


        /* Add a task to this group of threads
        \param function the task using C++11 lambdas
        */
        void add(const std::function<int()>& function);

        /* Start the execution of tasks by the threads of the group
        A task is considered as completed when its associated lambda function (given in add()) return
        \param concurrent True to kill all tasks as soon as one task is terminated ()
        */
        void start();

        /* Wait that all tasks are completed (only one in concurrent mode)
        and join all threads
        \return The return code of the winner in concurrent mode
        */
        int wait();


        /* Kill all tasks and join all threads
        \return The return code of the winner in concurrent mode
        */
        int kill();

        /* Wait some seconds and verify if the a task incompleted in concurrent mode.
        \param seconds Seconds to wait
        \return -1 if no task has won else the return code of the winner
        */
        int wait(unsigned int seconds);

        /* Wait some seconds and kill all tasks
        \param seconds Seconds to wait
        \return The return code of the winner in concurrent mode
        */
        int waitAndKill(unsigned int seconds);


        /* Reload/Reinit threads and tasks as a constructor call
        */
        void reload();


        /* For a group of N threads, this method return the thread id.
         * The thread id is between 0 and N-1.
         */
        inline unsigned int getThreadId() {
            thread_local static unsigned int threadId = UINT_MAX;
            if(threadId != UINT_MAX)return threadId;
            for(unsigned int i = 0; i < threads.size(); i++) {
                if(threads[i]->get_id() == std::this_thread::get_id()) {
                    threadId = i;
                    return threadId;
                }
            }
            assert(false); // Impossible
            return threadId;
        }
        inline unsigned int getId() const {return idGroup;}
        
        inline unsigned int getNbThreads() const {return nbThreads;}
        inline unsigned int getNbLaunchedTasks() const {return nbLaunchedTasks;}
        inline unsigned int getNbTasks() const {return tasks.size();}
        

        inline std::vector<Task>& getTasks() {return tasks;}
        inline Task& getTask(){return tasks[getTaskId()];}
        
        

        //To stop tasks
        inline void stop() {testStop = true;}
        inline bool isStopped() {return testStop;}

        inline Task& getWinner(){return tasks[winnerId];}

        inline Group& concurrent(){
            concurrentMode = true;
            return *this;
        }

        inline Group& popFront(){
            taskPopFront = true;
            return *this;
        }

        inline Group& popBack(){
            taskPopFront = false;
            return *this;
        }

        inline Controller* getController(){return controller;}
        inline void setController(Controller* _controller){controller = _controller;}
        inline void setConcurrentGroupsModes(bool _concurrentGroupsModes){concurrentGroupsModes=_concurrentGroupsModes;}
    private:

        inline unsigned int getTaskId() {return CurrentTaskIdPerThread[getThreadId()];}
        inline void setTaskStatus(Status _status){tasks[getTaskId()].setStatus(_status);}
        

        void wrapperFunction();

        void wrapperWaitting(unsigned int seconds);
        // Winner of the concurrential method    
        unsigned int winnerId;
        
        //General variables for a group
        std::vector<std::thread*> threads;
        std::vector<Task> tasks;
        std::deque<unsigned int> tasksIdToRun;
        
        std::vector<unsigned int> CurrentTaskIdPerThread;
        

        volatile bool testStop;
        unsigned int idGroup;
        unsigned int nbThreads;
        unsigned int nbLaunchedTasks;
        unsigned int nbTasks;

        //For the concurrent mode
        bool concurrentMode;

        //For the mutual exclusions
        Barrier *startedBarrier;
        std::mutex tasksMutex;

        //For wait with seconds
        std::thread *waitingThreads;

        bool hasStarted;
        bool hasWaited;

        //For the concurrent mode of several groups
        bool concurrentGroupsModes;
        bool taskPopFront;
        Controller* controller;

    };

    

}


#endif
