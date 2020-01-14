


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


    std::mutex Starters::startersMutex;
    Starter* Starters::current = NULL;

    std::vector<Starter*> Starters::starters = std::vector<Starter*>();
    bool Starters::haveToBeFreed = false;
    unsigned int Starters::nbRecurrences = 0;
    Starters starters = Starters();
    
    Starter* start(Group& t) // base function
    {
        if (Starters::nbRecurrences == 0){starters.add();}
        Starters::nbRecurrences++;
        Starters::current->push_back(&t);
        t.setStarter(Starters::current);
        t.start();
        return Starters::current;
    }

    Starter* start(std::vector<Group>& groups){
        starters.add();
        Starters::current->push_back(groups);
        for(auto& group: groups){
            group.start();
            group.setStarter(Starters::current);
        }
        return Starters::current;
    }

    template <typename T, typename... Ts>
    Starter* start(T& t, Ts&... ts) // recursive variadic function
    {   
        start(t);
        start(ts...);
        Starters::nbRecurrences--;
        return Starters::current;
    }

}

