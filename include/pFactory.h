
#ifndef pFactory_H
#define	pFactory_H

#include <sstream> 

#include "Task.h"

#include "Controller.h"
#include "Groups.h"
#include "Barrier.h"
#include "Communicators.h"
#include "Intercommunicators.h"
#include "Safestd.h"



namespace pFactory{
    
        
    unsigned int getNbCores();

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

}

#endif
