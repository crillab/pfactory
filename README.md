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
 - autoreconf (package autoconf)
 - C++11
 - pthread library

### Installation 
 

In the directory pFactory:
```console
./bootstrap
```
```console
./configure
```
```console
make
```

The created library ```libpFactory.a``` is in the directory ```lib/```

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

// In this example, we create a group of thread saying hello world

int main()
{
  // A group of nbCores threads
  pFactory::Group group(pFactory::getNbCores());
  for (unsigned int i = 0; i < pFactory::getNbCores(); i++)
  {
    // Add as many tasks as threads in the group
    group.add([=]() {
      printf("Hello world of thread (or task) %d\n", i);
      return 0;
    });
  }
  // Start the computation of all tasks
  group.start();
  // Wait until all threads are performed all tasks
  group.wait();
}
```


### Example 2 : Communications

```cpp
#include "pFactory.h"
#include <mutex>

// In this example, each thread share an integer to the others.
// The mutex is used only for displaying data in a smart way.
// Of course, on classical usage, this mutex is useless, you
// can directly share and receive data

int main()
{
    // A group of nbCores threads
    pFactory::Group group(pFactory::getNbCores());
    pFactory::MultipleQueuesCommunicator<int> integerCommunicator(&group);
    std::mutex m;

    for (unsigned int i = 0; i < pFactory::getNbCores(); i++)
    {
        // Add as many tasks as threads in the group
        group.add([&, i]() {
            // Send a random number
            m.lock();
            int nb = rand() % 101;
            std::cout << "Thread (or task)" << i << " sends: " << nb << std::endl;
            integerCommunicator.send(nb);
            m.unlock();

            //Receive and display all numbers of others threads
            std::vector<int> data;
            integerCommunicator.recvAll(data);
            m.lock();
            std::cout << "Thread (or task)" << i << " receives:";
            for (unsigned int j = 0; j < data.size(); ++j)
                std::cout << data[j] << ' ';
            std::cout << std::endl;
            m.unlock();
            return 0;
        });
    }
    // Start the computation of all tasks
    group.start();
    // Wait until all threads are performed all tasks
    group.wait();
}
```

### Example 3
You can also [download]() an implementation of the [SAT solver glucose](https://www.labri.fr/perso/lsimon/glucose/) in parallel mode (aka named syrup)
using the library pFacory. Such implementation integrates clauses sharing mechanism.


## AUTHORS:

Main author: 
 - Nicolas Szczepanski - szczepanski.nicolas@gmail.com


Contributors
 - Gilles Audemard - audemard@cril.fr
 - Gael Glorian - glorian@cril.fr
 - Jean-Marie Lagniez - jmlagniez@gmail.com
 - Valentin Montmirail - valentin.montmirail@gmail.com



## Contact
Do not hesitate to contact pfactory@cril.fr if you encounter any problems with pFactory.
