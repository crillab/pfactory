
#include "Parallel.h"
#include "Communicators.h"

namespace pFactory{
  template <class T>
  Communicator<T>::Communicator(Group* g, T pemptyValue):
    group(g),
    emptyValue(pemptyValue),
    oneWatchs(false){};

  template <class T>
  Communicator<T>::~Communicator() {
    printf("delete communicator\n");
  }

  template <class T>
  MultipleQueuesCommunicator<T>::MultipleQueuesCommunicator(Group* g, T pemptyValue):
    Communicator<T>(g,pemptyValue),
    nbThreads(g->getNbThreads()),
    threadMutexs(g->getNbThreads()),
    vectorOfQueues(g->getNbThreads()),
                
    threadQueuesPointer(g->getNbThreads(),std::vector<unsigned int>(g->getNbThreads())),  
    threadOrdersPointer(g->getNbThreads(),std::vector<OrderPointer*>(g->getNbThreads(),NULL)),
    threadOrdersPointerStart(g->getNbThreads(),NULL),
    threadOrdersPointerEnd(g->getNbThreads(),NULL),

    minQueuesPointer(g->getNbThreads()),
    minSecondQueuesPointer(g->getNbThreads()),
                
    nbSend(g->getNbThreads()),
    nbRecv(g->getNbThreads()),
    nbRecvAll(g->getNbThreads())
    {
      for(unsigned int i=0;i<nbThreads;i++){
	std::vector<OrderPointer*>& ordersPointer=threadOrdersPointer[i];
	//Create OrderPointers and set the idThread
	threadOrdersPointerStart[i]=new OrderPointer();
	for(unsigned int j=0;j<nbThreads;j++)ordersPointer[j]=new OrderPointer(j);
	threadOrdersPointerEnd[i]=new OrderPointer();
                        
	//Set the next pointers 
	threadOrdersPointerStart[i]->next=ordersPointer[0];
	for(unsigned int j=0;j<nbThreads-1;j++)ordersPointer[j]->next=ordersPointer[j+1];
	ordersPointer[nbThreads-1]->next=threadOrdersPointerEnd[i];
                        
	//Set the previous pointers
	threadOrdersPointerEnd[i]->previous=ordersPointer[nbThreads-1];
	for(unsigned int j=nbThreads-1;j>0;j--)ordersPointer[j]->previous=ordersPointer[j-1];
	ordersPointer[0]->previous=threadOrdersPointerStart[i];
                        
	//Delete the current pointeur (swap and delete) 
	ordersPointer[i]->next->previous=ordersPointer[i]->previous;
	ordersPointer[i]->previous->next=ordersPointer[i]->next;
	delete ordersPointer[i];
	ordersPointer[i] = NULL;
                        
      }

    };

  template <class T>
  MultipleQueuesCommunicator<T>::~MultipleQueuesCommunicator(){
    printf("Delete multipleQueuesCommunicator\n");
  }

}
