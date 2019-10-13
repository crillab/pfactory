# pFactory - A generic library for designing parallel solvers
pFactory is a parallel library designed to support and facilitate the implementation of parallel solvers in C++. It provides robust implementations of
parallel algorithms and allows seamlessly sharing mechanisms, divide-and-conquer or portfolio methods.
pFactory is not related to a specific problem and can very easily be incorporated in order to solve any kind of combinatorial problem (SAT, CSP, MAXSAT...).

To make user-friendly the usage of communications, our library contains an object called Communicator<T> using templates.
Using such object, it is possible to share any kind of informations (vector, int...). A dedicated and powerful algorithms
to exchange such informations is provided. 

More informations are given in the following paper:

pFactory: A generic library for designing parallel solvers, the 16th International Conference on Applied Computing (AC 2019), To appear.
Gilles Audemard, Gael Glorian, Jean-Marie Lagniez, Valentin Montmirail and Nicolas Szczepanski


## Installation instructions

### Dependencies
This libary needs: 
 - C++11
 - pthread library

### Installation 
 

In the directory pFactory, to create the library libpFactory.a:
```console
make -j 
```

## User Manual

The main method of pFactory is the constructor ```pFactory::Group(unsigned int nbThreads)``` which create a Group object. 
An instance of the class group represents:
  - a set of threads ```std::thread```
  - a set of tasks ```std::function<int()>```

Tasks are added thanks to the method ```Group::add(std::function<int()> function)``` 
using C++11 lambdas and are launched by the method ```Group::start(bool concurrent=false)```. 
Of course, we can have more tasks than threads and in this case, a queue of work is 
created and all tasks are executed. To finish, the method ```Group::wait()```  waits 
that all tasks are completed (only one if the concurrent mode is activated in the method ```start```) 
and join all threads. 

The library contains an efficient sharing mechanism.
Once the group object created, you can link a communicator that is in charge of the communication 
between threads. To this end: 
- Create a communicator (in this example, the communicator can share integers between thraeds): 
``` pFactory::Communicator<int>* integerCommunicator    = new pFactory::MultipleQueuesCommunicator<int>(group);```. 
    Variable ```group``` is an  instance of ````Factory::Group``` object defined above. 
- Send int to other threads using the method ```send```:
- Receive integers from other threads using the method ```recAll```. 




## Examples

### Example 1: Hello World

```cpp
#include "pFactory.h" 

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


### Example 2 : Communications

```cpp
#include "pFactory.h" 
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

### Example 3
You can also [download]() an implementation of the [SAT solver glucose](https://www.labri.fr/perso/lsimon/glucose/) in parallel mode (aka named syrup)
using the library pFacory. Such implementation integrates clauses sharing mechanism.



## contact
Do not hesitate to contact szczepanski.nicolas@gmail.com if you encounter any problems with pFactory.
