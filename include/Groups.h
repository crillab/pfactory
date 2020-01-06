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

namespace pFactory {
    const int VERBOSE = 0;
    const int TASK_NOT_STARTED = -1;
    
    
    

    
    /* A unique mutex structure to safety display thinks in std::cout or std::cerr */
    static std::mutex std_mutex;
    
    struct cout
    {
            std::unique_lock<std::mutex> lk;
            cout()
                :
                lk(std::unique_lock<std::mutex>(std_mutex))
            {

            }

            template<typename T>
            cout& operator<<(const T& _t)
            {
                std::cout << _t;
                return *this;
            }

            cout& operator<<(std::ostream& (*fp)(std::ostream&))
            {
                std::cout << fp;
                return *this;
            }
    };

    struct cerr
    {
            std::unique_lock<std::mutex> lk;
            cerr()
                :
                lk(std::unique_lock<std::mutex>(std_mutex))
            {

            }

            template<typename T>
            cerr& operator<<(const T& _t)
            {
                std::cerr << _t;
                return *this;
            }

            cerr& operator<<(std::ostream& (*fp)(std::ostream&))
            {
                std::cerr << fp;
                return *this;
            }
    };

    class Group; // Say Group exists without defining it.

    unsigned int getNbCores();

    /* To represent a winner in a concurrential method */
    class Winner {
        public:
            Winner(unsigned int pthreadId, unsigned int ptaskId, unsigned int preturnCode)
                : threadId(pthreadId), taskId(ptaskId), returnCode(preturnCode){}
        
            inline unsigned int getThreadId(){return threadId;}
            inline unsigned int getTaskId(){return taskId;}
            inline unsigned int getReturnCode(){return returnCode;}

            inline void setWinner(unsigned int pthreadId, unsigned int ptaskId, unsigned int preturnCode){
                threadId=pthreadId;
                taskId=ptaskId;
                returnCode=preturnCode;
            }
        
        private:
            unsigned int threadId;
            unsigned int taskId;
            unsigned int returnCode;

            

    };

    /* An instance of the class group represent :
        - a set of threads (std::thread*)
        - a set of tasks (std::function<int())
    */
    class Group {
        static int groupCount; //To get the id of a group

    public:
        //Barrier for the user
        Barrier barrier;

        

        /* The constructor of a group
        \param pnbThreads the number of threads
        \return An instance of group
        */
        explicit Group(unsigned int pnbThreads);

        explicit Group(const Group &) = delete;

        Group const & operator=(Group &&g) = delete;


        ~Group() {
            if (!hasStarted){
                tasks.clear(); //First clean all tasks
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
        void add(const std::function<int()> &function);

        /* Start the execution of tasks by the threads of the group
        A task is considered as completed when its associated lambda function (given in add()) return
        \param concurrent True to kill all tasks as soon as one task is terminated ()
        */
        void start(bool concurrent = false);

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
        inline unsigned int getGroupId() { return idGroup;}
        inline unsigned int getTaskId() { return currentTasksId[getThreadId()];}
        inline unsigned int getNbThreads() { return nbThreads;}
        inline unsigned int getNbLaunchedTasks() { return nbLaunchedTasks;}
        inline std::vector<int> &getReturnCodes() { return returnCodes; }

        //To stop tasks
        inline void stop() { testStop = true; }
        inline bool isStopped() { return testStop; }

        inline Winner& getWinner(){return winner;}

    private:

        void wrapperFunction();

        void wrapperWaitting(unsigned int seconds);
        // Winner of the concurrential method    
        Winner winner;
        
        //General variables for a group
        std::vector<std::thread *> threads;
        std::vector <std::function<int()>> tasks;
        std::vector <std::function<int()>> tasksSave;

        std::vector<unsigned int> currentTasksId;
        std::vector<int> returnCodes;
        

        volatile bool testStop;

        unsigned int idGroup;
        unsigned int nbThreads;
        unsigned int nbTasks;
        unsigned int nbLaunchedTasks;

        //For the concurrent mode
        bool concurrent;

        //For the mutual exclusions
        Barrier *startedBarrier;
        std::mutex tasksMutex;

        //For wait with seconds
        std::thread *waitingThreads;

        bool hasStarted;
        bool hasWaited;

    };

    template<class T=int>
    class SafeQueue {
    public:
        explicit SafeQueue() {};


        inline void push_back(T &ele) {
            mutex.lock();
            queue.push_back(ele);
            mutex.unlock();
        };


        /* Calling this function on an empty container causes undefined behavior. */
        inline T &pop_back() {
            mutex.lock();
            T &ele = queue.back();
            queue.pop_back();
            mutex.unlock();
            return ele;
        };
    private:
        std::mutex mutex;
        std::deque <T> queue;
    };

    template <typename T>
    void start(T& t) // base function
    {
        t.start();
    }
    
    template <typename T, typename... Ts>
    void start(T& t, Ts&... ts) // recursive variadic function
    {
        t.start();
        start(ts...);
    }
    


}


#endif
