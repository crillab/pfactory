
#include "pFactory.h"

class Solver{};



int main(){
  std::vector<Solver*> solvers;
  for(unsigned int i = 0; i < pFactory::getNbCores();i++)solvers.push_back(new Solver());
  pFactory::Group* group = new pFactory::Group(pFactory::getNbCores());
  pFactory::SafeQueue<Solver*>* availableSolvers = new pFactory::SafeQueue<Solver*>();
  
  for(unsigned int i = 0; i < pFactory::getNbCores();i++){
    availableSolvers->push_back(solvers[i]);
    group->add([=](){
	Solver* currentSolver = availableSolvers->pop_back();
	printf("Hello world of %d\n",i);
	availableSolvers->push_back(currentSolver);
	return 0;
      });
  }
  group->start();
  group->wait();
}
