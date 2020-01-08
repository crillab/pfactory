


#include <sstream> 

#include "Task.h"
#include "Groups.h"
#include "Barrier.h"
#include "Communicators.h"
#include "Intercommunicators.h"
#include "Safestd.h"

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

    class Starter {
        public:
            static Starter* current;
            static unsigned int nbRecurrences;

            Starter(std::vector<Group>& _groups)
            {
               for(unsigned int i = 0; i < _groups.size(); i++){
                   groups.push_back(&_groups[i]);
               }
            };

            Starter(){}
            
            void push_back(Group* group){
                groups.push_back(group);
            }

            void concurrent(){
                std::cout << "Concurrent" << std::endl;
            }

        private:
            std::vector<Group*> groups;


    };
    Starter* Starter::current = NULL;
    unsigned int Starter::nbRecurrences = 0;

    void start(Group& t) // base function
    {
        if (Starter::nbRecurrences == 0){
            if (Starter::current != NULL) delete Starter::current;
            Starter::current = new Starter();
        }
        Starter::current->push_back(&t);
        t.start();
    }

    Starter& start(std::vector<Group>& groups){
        Starter* starter = new Starter(groups);
        for(unsigned int i = 0; i < groups.size(); i++)
            groups[i].start();
        return *starter;
    }

    template <typename T, typename... Ts>
    Starter& start(T& t, Ts&... ts) // recursive variadic function
    {   
        
        Starter::nbRecurrences++;
        start(t);
        start(ts...);
        Starter::nbRecurrences--;
        return *Starter::current;
    }

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

