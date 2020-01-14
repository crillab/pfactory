
#include <vector> 
#include "Groups.h"
#ifndef Starter_H
#define Starter_H

namespace pFactory{
    

    class Starter {
        public:
            
            Starter():winner(NULL){}
            Starter(Group* _group){push_back(_group);}
            Starter(std::vector<Group>& _groups){push_back(_groups);}

            void push_back(Group* _group){groups.push_back(_group);}
            void push_back(std::vector<Group>& _groups){for (auto& group: _groups)push_back(&group);}

            Starter& concurrent(){
                for (auto& group: groups)group->setConcurrentGroupsModes(true);
                return *this;
            }

            std::vector<Group*>& getGroups(){return groups;}
            
            void setWinner(Group* _winner){winner = _winner;}
            Group* getWinner(){return winner;}


        private:
            
            std::vector<Group*> groups;
            Group* winner;
    };
    

    class Starters {
        public:
            static std::vector<Starter*> starters;
            static Starter* current;
            static bool haveToBeFreed;
            static unsigned int nbRecurrences;
            static std::mutex startersMutex;

            Starters(){}

            static Starter* add(){
                Starter* starter = new Starter();
                if (Starters::haveToBeFreed == false){
                    Starters::haveToBeFreed = true;
                    std::atexit(Starters::free);
                }
                Starters::current = starter;
                starters.push_back(starter);
                return starter;
            }

            

            static void free(){for (auto starter : starters) delete starter;}
    };

    
}
#endif