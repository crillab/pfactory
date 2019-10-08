
#include "pFactory.h"

int main(){
  pFactory::Group* group = new pFactory::Group(pFactory::getNbCores());
  
  for(unsigned int i = 0; i < pFactory::getNbCores();i++){
    group->add([=](){
	printf("Hello world of %d\n",i);
	return 0;
      });
  }
  group->start();
  group->wait();
  delete group;
}
