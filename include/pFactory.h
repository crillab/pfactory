


#include <sstream> 

#include "Task.h"
#include "Groups.h"
#include "Barrier.h"
#include "Communicators.h"
#include "Intercommunicators.h"
#include "Safestd.h"
#include "Starter.h"

namespace pFactory{
    
    
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

   

    void wait(Group& t) // base function
    {
        t.wait();
    }

    void wait(std::vector<Group>& groups){
        for(unsigned int i = 0; i < groups.size(); i++){
            groups[i].wait();
        }
    }


    template <typename T, typename... Ts>
    void wait(T& t, Ts&... ts) // recursive variadic function
    {
        wait(t);
        wait(ts...);
    }

}

