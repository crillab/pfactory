
#include "../../Parallel.h"
#include "../../Communicators.h"

int main(int argc, char** argv){
  pFactory::Group* group = new pFactory::Group(pFactory::getNbCores());
  pFactory::Communicator<int>* integerCommunicator
    = new pFactory::MultipleQueuesCommunicator<int>(group, 0);
  
  for(int i = 0; i < pFactory::getNbCores();i++){
    group->add([=](){
	printf("Hello world of %d\n",i);
	return 0;
      });
  }
  group->start();
  group->wait();
  delete group;
  delete integerCommunicator;
}
