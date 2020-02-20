/**
 *   pFactory, a generic library for designing parallel solvers.
 *   Copyright (C) 2019 Artois University and CNRS
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef communicators_H
#define communicators_H

#include <initializer_list>

namespace pFactory
{

/* Element of a double linked list representing the threads, the position, and their order, within of a std::deque of data */
class OrderPointer
{
public:
    OrderPointer() : next(NULL), previous(NULL), idThread(-1){};
    OrderPointer(int pidThread) : next(NULL), previous(NULL), idThread(pidThread){};

    OrderPointer *next;
    OrderPointer *previous;
    int idThread;
};


/*
 * To communicate between threads some information by copies.
 */
template <class T>
class Communicator
{
protected:
    Group& group; /* Group of threads that have to communicate */ 
    const unsigned int nbThreads; /* Number of threads */ 

    /* Used to know the allowed senders for the communications in the associated group */ 
    std::vector<bool> senders;

    /* Used to know the allowed receivers for the communications in the associated group */ 
    std::vector<bool> receivers;

    /* Data to exchange : one std::deque per thread, the ith std::deque is the data sent by the ith thread */
    std::vector<std::deque<T>> vectorOfQueues; 

    /* One mutex per queue */
    std::vector<std::mutex> threadMutexs;  

    /* for each std::deque of data, the position of each other thread (to know data already received) */
    std::vector<std::vector<unsigned int>> threadQueuesPointer; 

    /* for each std::deque of data, to now the order of threads according to theirs positions (OrderPoiter* is a double linked list) */
    std::vector<std::vector<OrderPointer *>> threadOrdersPointer;
    
    /* First element for each std::vector<OrderPointer *> */ 
    std::vector<OrderPointer *> threadOrdersPointerStart;

    /* Last element for each std::vector<OrderPointer *> */ 
    std::vector<OrderPointer *> threadOrdersPointerEnd;

    /* For each std::deque, the smallest position of a thread (used to know if a thread is the last to recuperate some data) */
    std::vector<unsigned int> minQueuesPointer;

    /* For each std::deque, the second smallest position of a thread (so not the smallest :)) */
    /* Used to know, in the case where one thread is the last to recuperate some data, the limit of these last data */
    std::vector<unsigned int> minSecondQueuesPointer;

    
    std::vector<unsigned int> nbSend;
    std::vector<unsigned int> nbRecv;
    std::vector<unsigned int> nbRecvAll;

    
public:
    Communicator(Group& g, bool withInitialize=true);
    Communicator(Group& g, const std::vector<bool>& senders, const std::vector<bool>& receivers, bool withInitialize=true);
    Communicator(Group& g, std::initializer_list<unsigned int> p_senders, std::initializer_list<unsigned int> p_receivers, bool withInitialize=true);

    void initialize();

    ~Communicator();

    void createOrderPointer(unsigned int queue, unsigned int lenght);
    void deleteOrderPointer();
    void removePointer(unsigned int queue, unsigned int thread);

   
    /*Send a data to others threads
          \param data Data to send
          Warning : user may pass a copŷ
        */
    inline void send(T data)
    {
        const unsigned int threadId = group.getThreadId();
        if (senders[threadId] == false){
            //printf("Warning: no data sent in a send() operation by a thread of a group that not is in the senders !");
            return;
        } 
        threadMutexs[threadId].lock();
        vectorOfQueues[threadId].push_back(data);
        //printf("send data of %d\n",threadId);
        nbSend[threadId] += (nbThreads - 1);
        threadMutexs[threadId].unlock();
    }

    /* Say if there are data to recuperate
         */
    inline bool isEmpty()
    {
        const unsigned int threadId = group.getThreadId();
        for (unsigned int threadIdQueue = 0; threadIdQueue < nbThreads; threadIdQueue++)
        {
            //Browse all queue except the queue of this thread
            if (threadIdQueue != threadId)
            {
                //These adresses don't move, so no mutex here !
                std::deque<T> &deque = vectorOfQueues[threadIdQueue];
                std::vector<unsigned int> &queuePointer = threadQueuesPointer[threadIdQueue];
                if (deque.empty() || queuePointer[threadId] == deque.size())
                    continue;
                return false;
            }
        }
        return true;
    }
    /* Verify the queue of pointers
    */
    inline void assertQueuePointerCriticalSection(unsigned int threadIdQueue){
        assert(threadOrdersPointerStart[threadIdQueue]->next != NULL);
        assert(threadOrdersPointerStart[threadIdQueue]->next->next != NULL);
        assert(threadOrdersPointerStart[threadIdQueue]->previous == NULL);
        assert(threadOrdersPointerEnd[threadIdQueue]->next == NULL);
        assert(threadOrdersPointerEnd[threadIdQueue]->previous != NULL);
    }


    /*  Update the ordersPointer (put the current thread in the end of the queue) 
        Usefull to calculate the minimum and the second minimum faster
    */
    inline void updateOrdersPointer(std::vector<OrderPointer *> &ordersPointer, unsigned int threadIdQueue, unsigned int threadId){
        ordersPointer[threadId]->next->previous = ordersPointer[threadId]->previous; //Removing in the queue
        ordersPointer[threadId]->previous->next = ordersPointer[threadId]->next;     //Removing in the queue
        ordersPointer[threadId]->next = threadOrdersPointerEnd[threadIdQueue];               //Push back in the queue
        ordersPointer[threadId]->previous = threadOrdersPointerEnd[threadIdQueue]->previous; //Push back in the queue
        threadOrdersPointerEnd[threadIdQueue]->previous->next = ordersPointer[threadId];     //Push back in the queue
        threadOrdersPointerEnd[threadIdQueue]->previous = ordersPointer[threadId];           //Push back in the queue
    }

    /*
    *   Pop all data already received by all threads (based on a heuristic)
    */
    inline void popDataReceived(unsigned int minQueuePointer, unsigned int threadIdQueue, std::vector<unsigned int> &queuePointer, std::deque<T> &deque){
        for (unsigned int i = 0; i < minQueuePointer; i++)
        {
            for (unsigned int j = 0; j < nbThreads; j++){
                // Update queuePointer
                if (j != threadIdQueue) 
                    queuePointer[j]--;
            }
            deque.pop_front();
        }
    }


    /* Receive all elements from the communicator.
           \param data: Received elements
           Remark2: When no data is found, nothing is added in these two parameters
        */
    inline void recvAll(std::vector<T> &data)
    {
        std::vector<T> noDataLast;
        recvAll(data, noDataLast, false);
    }

    /* Receive all elements from the communicator.
           \param dataNotLast Elements which have not been received by all threads
           \param dataLast Elements which have been received by all threads (others threads have already received the element)
           Remark1: dataNotLast and dataLast are useful to deal with copy
           Remark2: When no data is found, nothing is added in these two parameters
        */
    inline void recvAll(std::vector<T> &dataNotLast, std::vector<T> &dataLast, bool withDataLast = true)
    {
        const unsigned int threadId = group.getThreadId();
        if (!receivers[threadId]) return; //If this thread is not a receiver, do nothing !

        for (unsigned int threadIdQueue = 0; threadIdQueue < nbThreads; threadIdQueue++)
        {
            //Browse all queues except the queue of this thread and the queues from threads that are not a sender
            if (threadIdQueue != threadId && senders[threadIdQueue])
            {

                //These adresses don't move, so no mutex here !
                std::deque<T> &deque = vectorOfQueues[threadIdQueue];
                std::vector<unsigned int> &queuePointer = threadQueuesPointer[threadIdQueue];
                //Special issue if the watch of thread is at the end or that the vector is empty (no clause to recuperate)
                if (deque.empty() || queuePointer[threadId] == deque.size())
                {
                    continue;
                }

                std::mutex &mutex = threadMutexs[threadIdQueue];
                unsigned int &minQueuePointer = minQueuesPointer[threadIdQueue];
                unsigned int &minSecondQueuePointer = minSecondQueuesPointer[threadIdQueue];
                std::vector<OrderPointer *> &ordersPointer = threadOrdersPointer[threadIdQueue];

                mutex.lock();

                //Verify the queue of pointers
                assertQueuePointerCriticalSection(threadIdQueue);

                //Get the minimum and the second minimum !
                minQueuePointer = queuePointer[threadOrdersPointerStart[threadIdQueue]->next->idThread];

                if (withDataLast){
                    int idSecondQueuePointer = threadOrdersPointerStart[threadIdQueue]->next->next->idThread;
                    minSecondQueuePointer = (idSecondQueuePointer == -1) ? deque.size() : queuePointer[idSecondQueuePointer];

                    //Recuperate clauses that I have to no copy : it is the dataLast thread that take these clauses
                    if (minQueuePointer == queuePointer[threadId])
                    { //I am a minumum : there are may be dataLast clauses with no copy (this thread is at the smallest position)
                        while (queuePointer[threadId] != minSecondQueuePointer)
                        { // warning, that can be equals (severals minimums equals)!
                            dataLast.push_back(deque[queuePointer[threadId]++]);
                            nbRecv[threadId]++;
                        }
                    }
                }
                //Now, recuperate clauses that I have to copy
                while (queuePointer[threadId] != deque.size())
                {
                    dataNotLast.push_back(deque[queuePointer[threadId]++]);
                    nbRecv[threadId]++;
                }
                //At this time, this assertion have to be verify (no more clauses to recuperate)
                assert(queuePointer[threadId] == deque.size());

                //Update the ordersPointer 
                updateOrdersPointer(ordersPointer, threadIdQueue, threadId);

                //pop data already recuperate by all threads
                if (minQueuePointer > 1000) popDataReceived(minQueuePointer, threadIdQueue, queuePointer, deque);

                mutex.unlock();
            }
        }
        nbRecvAll[threadId]++;
    }

    /* Receive only one data
           \param data received
           \return false  if no element is found, true otherwise
        */
    inline bool recv(T &data)
    {
        bool nothing = false;
        return recv(data, nothing);
    }

    /* Receive only one data
            \param data received
            \param isLast true if this thread is the last to recuperate the data T (ie. others have already received the data T)
           \return false  if no element is found, true otherwise
        */
    inline bool recv(T &data, bool &isLast)
    {
        const unsigned int threadId = group.getThreadId();
        for (unsigned int threadIdQueue = 0; threadIdQueue < nbThreads; threadIdQueue++)
        {
            //Browse all queue except the queue of this thread
            if (threadIdQueue != threadId)
            {
                
                unsigned int i = 0;
                std::deque<T> &deque = vectorOfQueues[threadIdQueue];
                std::vector<unsigned int> &queuePointer = threadQueuesPointer[threadIdQueue];
                std::mutex &mutex = threadMutexs[threadIdQueue];
                unsigned int &minQueuePointer = minQueuesPointer[threadIdQueue];

                //Special issue if the watch of thread is at the end or the vector is empty : no data
                if (deque.empty() || queuePointer[threadId] == deque.size())
                {
                    continue;
                }

                mutex.lock();
                
                //Recuperate the data and increment the queuePointer of the thread
                unsigned int positionRet = queuePointer[threadId];
                data = deque[queuePointer[threadId]++];

                //Update the minWatch (usefull to know data to pop)
                minQueuePointer = INT_MAX;
                for (; i < nbThreads; i++)
                    if (i != threadIdQueue && minQueuePointer > queuePointer[i])
                        minQueuePointer = queuePointer[i];
                
                //Find if it is the dataLast or not
                isLast = (positionRet < minQueuePointer) ? true : false;
                
                //pop data already recuperate by all threads
                if (minQueuePointer > 1000) popDataReceived(minQueuePointer, threadIdQueue, queuePointer, deque);
                
                nbRecv[threadId]++;
                mutex.unlock();
                return true;
            }
        }
        return false;
    }

    inline unsigned int getNbSend()
    {
        unsigned int ret = 0;
        for (unsigned int threadId = 0; threadId < nbThreads; threadId++)
        {
            threadMutexs[threadId].lock();
            ret += nbSend[threadId];
            threadMutexs[threadId].unlock();
        }
        return ret;
    };
    inline unsigned int getNbRecv()
    {
        unsigned int ret = 0;
        for (unsigned int threadId = 0; threadId < nbThreads; threadId++)
        {
            threadMutexs[threadId].lock();
            ret += nbRecv[threadId];
            threadMutexs[threadId].unlock();
        }
        return ret;
    };

};


template <class T>
Communicator<T>::Communicator(Group& g, std::initializer_list<unsigned int> p_senders, std::initializer_list<unsigned int> p_receivers, bool withInitialize)
    : Communicator<T>::Communicator(g, false)
{
    for (unsigned int i = 0; i < senders.size(); i++)senders[i] = false;
    for (unsigned int x : p_senders)senders[x] = true;
    for (unsigned int i = 0; i < receivers.size(); i++)receivers[i] = false;
    for (unsigned int x : p_receivers)receivers[x] = true;    
    if (withInitialize == true) initialize();  
}

template <class T>
Communicator<T>::Communicator(Group& g, const std::vector<bool>& p_senders, const std::vector<bool>& p_receivers, bool withInitialize)
    : Communicator<T>::Communicator(g, false)
{
    senders = p_senders;
    receivers = p_receivers;
    if (withInitialize == true) initialize();
}





template <class T>
Communicator<T>::Communicator(Group& g, bool withInitialize)
    : group(g),
      nbThreads(g.getNbThreads()),
      
      senders(std::vector<bool>(nbThreads, true)),
      receivers(std::vector<bool>(nbThreads, true)),
      
      vectorOfQueues(nbThreads),
      threadMutexs(nbThreads),
      threadQueuesPointer(nbThreads, std::vector<unsigned int>(nbThreads)),
      threadOrdersPointer(nbThreads, std::vector<OrderPointer *>(nbThreads, NULL)),
      threadOrdersPointerStart(nbThreads, NULL),
      threadOrdersPointerEnd(nbThreads, NULL),

      minQueuesPointer(nbThreads),
      minSecondQueuesPointer(nbThreads),

      nbSend(nbThreads),
      nbRecv(nbThreads),
      nbRecvAll(nbThreads)
{
    if (withInitialize == true) initialize();
}

/* To delete a pointer (swap and delete)*/
template <class T>
void Communicator<T>::removePointer(unsigned int queue, unsigned int thread){
    threadOrdersPointer[queue][thread]->next->previous = threadOrdersPointer[queue][thread]->previous;
    threadOrdersPointer[queue][thread]->previous->next = threadOrdersPointer[queue][thread]->next;
    delete threadOrdersPointer[queue][thread];
    threadOrdersPointer[queue][thread] = NULL;
}

/* To initialize the double linked list OrderPointer */
template <class T>
void Communicator<T>::createOrderPointer(unsigned int queue, unsigned int lenght){
    std::vector<OrderPointer *> &ordersPointer = threadOrdersPointer[queue];
    //Create OrderPointers 
    threadOrdersPointerStart[queue] = new OrderPointer();
    for (unsigned int j = 0; j < lenght; j++)
        ordersPointer[j] = new OrderPointer(j);
    threadOrdersPointerEnd[queue] = new OrderPointer();
    //Set the next pointers
    threadOrdersPointerStart[queue]->next = ordersPointer[0];
    for (unsigned int j = 0; j < lenght - 1; j++)
        ordersPointer[j]->next = ordersPointer[j + 1];
    ordersPointer[lenght - 1]->next = threadOrdersPointerEnd[queue];

    //Set the previous pointers
    threadOrdersPointerEnd[queue]->previous = ordersPointer[lenght - 1];
    for (unsigned int j = lenght - 1; j > 0; j--)
        ordersPointer[j]->previous = ordersPointer[j - 1];
    ordersPointer[0]->previous = threadOrdersPointerStart[queue];
}

template <class T>
void Communicator<T>::initialize(){
    for (unsigned int i = 0; i < nbThreads; i++)
    {
        if (senders[i]){ //Only if i is a sender thread !
            //Create OrderPointers
            createOrderPointer(i, nbThreads);

            //The ith queue do not need a pointer of itself
            removePointer(i, i);

            //Delete the non-receivers pointer
            for (unsigned int j = 0; j < nbThreads; j++){
                if (!receivers[j] && j != i){
                    removePointer(i, j);
                }
            }
        }
    }
}

template <class T>
void Communicator<T>::deleteOrderPointer(){
    for (unsigned int i = 0; i < nbThreads; i++)
    {
        std::vector<OrderPointer *> &ordersPointer = threadOrdersPointer[i];
        delete threadOrdersPointerStart[i];
        for (unsigned int j = 0; j < nbThreads; j++)
            delete ordersPointer[j];
        delete threadOrdersPointerEnd[i];
    }
} 

template <class T>
Communicator<T>::~Communicator()
{
    deleteOrderPointer();
}
} // namespace pFactory

#endif
