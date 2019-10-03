# Install on Ubuntu

```console
sudo apt install g++
sudo apt install build-essential
```

In the directory pFactory, to create the library libpFactory.a:
```console
make -j 
```

# User Manual

First, include in your c++ code the library. The main method of pFactory is the constructor ```pFactory::Group(unsigned int nbThreads)``` which create a Group object. An instance of the class group represent :
  - a set of threads ```std::thread```
  - a set of tasks ```std::function<int()>```

Tasks are added thanks to the method ```Group::add(std::function<int()> function)``` using C++11 lambdas and are launched by the method ```Group::start(bool concurrent=false)```. Of course, we can have more tasks than threads and in this case, a queue of work is created and all tasks are executed. To finish, the method ```Group::wait()```  waits that all tasks are completed (only one if the concurrent mode is activated in the method ```start```) and join all threads. pFactory allows also to deal with communications between tasks and include a new effective communication algorithm. More explanations are given in the paper:

pFactory: A generic library for designing parallel solvers, the 16th International Conference on Applied Computing (AC 2019), To appear.
Gilles Audemard, Gael Glorian, Jean-Marie Lagniez, Valentin Montmirail and Nicolas Szczepanski

Do not hesitate to contact szczepanski.nicolas@gmail.com if you encounter any problems with pFactory.

Do not forget to add a link to pFactory in the compilation of your project.
Makefile for the Hello World example:

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


## Example 2 : Communications

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