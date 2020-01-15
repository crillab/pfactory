


#include <sstream> 

#include "Task.h"

#include "Controller.h"
#include "Groups.h"
#include "Barrier.h"
#include "Communicators.h"
#include "Intercommunicators.h"
#include "Safestd.h"

namespace pFactory{
    
    
    std::mutex Controller::mutex;
    
    unsigned int getNbCores(){
        std::ifstream cpuinfo("/proc/cpuinfo");
        return (!std::thread::hardware_concurrency())?
        (std::count(std::istream_iterator<std::string>(cpuinfo),
            std::istream_iterator<std::string>(),
            std::string("processor"))):
        std::thread::hardware_concurrency();
    }

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

