
#include <vector> 
#include "Groups.h"
#ifndef Starter_H
#define Starter_H

namespace pFactory{
    

    class Starter {
        public:
            static unsigned int nbRecurrences;
            
            Starter(){}

            Starter(Group* _group){
                push_back(_group);
            }

            Starter(std::vector<Group>& _groups){
                push_back(_groups);
            }


            void push_back(Group* _group){
                groups.push_back(_group);
            }

            void push_back(std::vector<Group>& _groups){
                for (auto& group: _groups)push_back(&group);
            }

            void concurrent(){
                std::cout << "Concurrent" << std::endl;
            }

        private:
            std::vector<Group*> groups;


    };
    

    class Starters {
        public:
            static std::vector<Starter*> starters;
            static Starter* current;

            Starters(){}

            Starter* add(){
                Starter* starter = new Starter();
                Starters::current = starter;
                starters.push_back(starter);
                return starter;
            }

            void clean(){
                for (auto starter : starters) delete starter;
            }
    };

    unsigned int Starter::nbRecurrences = 0;
    Starter* Starters::current = NULL;
    std::vector<Starter*> Starters::starters = std::vector<Starter*>();

    Starters _starters = Starters();
    bool callCleanStarterHandler = false;
    void cleanStarterHandler(){_starters.clean();}
    void doCleanStarterHandler(){
        if (!callCleanStarterHandler){
            std::atexit(cleanStarterHandler);
            callCleanStarterHandler = true;
        }
    }

    

    void start(Group& t) // base function
    {
        if (Starter::nbRecurrences == 0){
            doCleanStarterHandler();
            _starters.add();
        }
        Starter::nbRecurrences++;
        Starters::current->push_back(&t);
        t.setStarter(Starters::current);
        t.start();
    }

    Starter& start(std::vector<Group>& groups){
        doCleanStarterHandler();
        _starters.add();
        Starters::current->push_back(groups);
        for(auto& group: groups){
            group.start();
            group.setStarter(Starters::current);
        }
        return *Starters::current;
    }

    template <typename T, typename... Ts>
    Starter& start(T& t, Ts&... ts) // recursive variadic function
    {   
        
        
        start(t);
        start(ts...);
        Starter::nbRecurrences--;
        return *Starters::current;
    }

 
}

#endif