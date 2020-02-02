

#ifndef Controller_H
#define Controller_H


#include <vector> 
#include "Groups.h"

namespace pFactory{
    
    class Controller {
        public:
            static std::mutex mutex;

            Controller():winner(NULL){}
            
            Controller(Group& _group):Controller(){push_back(&_group);}
            Controller(std::vector<Group>& _groups):Controller(){for (auto& group: _groups)push_back(&group);}
            Controller(std::initializer_list<Group*> _groups):Controller(){for (auto& group: _groups)push_back(group);}
            

            void push_back(Group* _group){
                groups.push_back(_group);
                _group->setController(this);
            }

            Controller& start(){
                for (auto& group: groups)group->start();
                return *this;
            }

            Controller& wait(){
                for (auto& group: groups)group->wait();
                return *this;
            }

            Controller& concurrent(){
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

    
}
#endif