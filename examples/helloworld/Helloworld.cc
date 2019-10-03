
#include "../../pFactory.h"

int main(int argc, char** argv){
  pFactory::Group* group = new pFactory::Group(pFactory::getNbCores());
  
  for(int i = 0; i < pFactory::getNbCores();i++){
    group->add([=](){
	printf("Hello world of %d\n",i);
	return 0;
      });
  }
  group->start();
  group->wait();
  delete group;
}
