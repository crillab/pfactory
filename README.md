# Install

In the directory pFactory, to create the library libpFactory.a:

```console
make -j 
```

# User Manual

In the compilation of your project, add a link to pFactory.
Example of a makefile for the Hello World example:

```make
LFLAGS    = -L../../ -lpFactory -lpthread

helloworld: Helloworld.o
	g++ -o helloworld Helloworld.o $(LFLAGS)

Helloworld.o: Helloworld.cc
	g++ -std=c++14 -O3 -c Helloworld.cc 

clean:
	rm -rf helloworld *.o
```

# Examples

## Example 1: Hello World

```cpp
#include "../../pFactory.h" #This path can changed in your project

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
```


## Example : Communications

```cpp
#include "../../pFactory.h" #This path can changed in your project
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
```