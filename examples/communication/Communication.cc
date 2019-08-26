
#include "../../Parallel.h"
#include <mutex>

int main(int argc, char** argv){
  pFactory::Group* group = new pFactory::Group(pFactory::getNbCores());
  pFactory::Communicator<int>* integerCommunicator
    = new pFactory::MultipleQueuesCommunicator<int>(group, 0);
  std::mutex* m = new std::mutex();
  
  for(int i = 0; i < pFactory::getNbCores();i++){
    group->add([=](){
	integerCommunicator->send(i);
	std::vector<int> data;
	integerCommunicator->recvAll(data);
	m->lock();
	std::cout << "Thread " << i << " :";	
	for(int i=0; i<data.size(); ++i)
	  std::cout << data[i] << ' ';
	std::cout << std::endl;
	m->unlock();
	return 0;
      });
  }
  group->start();
  group->wait();
  delete group;
  delete integerCommunicator;
  delete m;
}
