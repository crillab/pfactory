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
#ifndef intercommunicators_H
#define intercommunicators_H

#include "Communicators.h"

namespace pFactory
{
    
template <class T>
class Intercommunicator : public Communicator<T>
{
    private:
        Group& receiverGroup;

    public:
    
        Intercommunicator(Group& psenderGroup, Group& preceiverGroup)
            :Communicator<T>::Communicator(psenderGroup, std::vector<bool>(psenderGroup.getNbThreads(), true), std::vector<bool>(preceiverGroup.getNbThreads(), true), false),
            receiverGroup(preceiverGroup)
        {
            initialize();
        }

        inline void initialize() {
            unsigned int receiverNbThreads = receiverGroup.getNbThreads();
            for (unsigned int i = 0; i < this->nbThreads; i++)
            {
                if (this->senders[i]){ //Only if i is a sender thread !
                    this->createOrderPointer(i, receiverNbThreads);

                    //Delete the non-receivers pointer
                    for (unsigned int j = 0; j < receiverNbThreads; j++){
                        if (!this->receivers[j]) this->removePointer(i, j);
                    }
                }
            }
        }

        inline void recvAll(std::vector<T> &data)
        {
            std::vector<T> noDataLast;
            recvAll(data, noDataLast, false);
        }
        
        /* Receive all elements from the intercommunicator.
           \param dataNotLast Elements which have not been received by all threads
           \param dataLast Elements which have been received by all threads (others threads have already received the element)
           Remark1: dataNotLast and dataLast are useful to deal with copy
           Remark2: When no data is found, nothing is added in these two parameters
        */
        inline void recvAll(std::vector<T> &dataNotLast, std::vector<T> &dataLast, bool withDataLast = true) 
        {
            const unsigned int threadId = receiverGroup.getThreadId();
            if (!this->receivers[threadId]) return; //If this thread is not a receiver, do nothing !

            for (unsigned int threadIdQueue = 0; threadIdQueue < this->nbThreads; threadIdQueue++)
            {
                //Browse all queues except the queue of this thread and the queues from threads that are not a sender
                if (this->senders[threadIdQueue])
                {

                    //These adresses don't move, so no mutex here !
                    std::deque<T> &deque = this->vectorOfQueues[threadIdQueue];
                    std::vector<unsigned int> &queuePointer = this->threadQueuesPointer[threadIdQueue];
                    //Special issue if the watch of thread is at the end or that the vector is empty (no clause to recuperate)
                    if (deque.empty() || queuePointer[threadId] == deque.size())
                    {
                        continue;
                    }

                    std::mutex &mutex = this->threadMutexs[threadIdQueue];
                    unsigned int &minQueuePointer = this->minQueuesPointer[threadIdQueue];
                    unsigned int &minSecondQueuePointer = this->minSecondQueuesPointer[threadIdQueue];
                    std::vector<OrderPointer *> &ordersPointer = this->threadOrdersPointer[threadIdQueue];

                    mutex.lock();

                    //Verify the queue of pointers
                    this->assertQueuePointerCriticalSection(threadIdQueue);

                    //Get the minimum and the second minimum !
                    minQueuePointer = queuePointer[this->threadOrdersPointerStart[threadIdQueue]->next->idThread];

                    if (withDataLast){
                        int idSecondQueuePointer = this->threadOrdersPointerStart[threadIdQueue]->next->next->idThread;
                        minSecondQueuePointer = (idSecondQueuePointer == -1) ? deque.size() : queuePointer[idSecondQueuePointer];

                        //Recuperate clauses that I have to no copy : it is the dataLast thread that take these clauses
                        if (minQueuePointer == queuePointer[threadId])
                        { //I am a minumum : there are may be dataLast clauses with no copy (this thread is at the smallest position)
                            while (queuePointer[threadId] != minSecondQueuePointer)
                            { // warning, that can be equals (severals minimums equals)!
                                dataLast.push_back(deque[queuePointer[threadId]++]);
                                this->nbRecv[threadId]++;
                            }
                        }
                    }
                    //Now, recuperate clauses that I have to copy
                    while (queuePointer[threadId] != deque.size())
                    {
                        dataNotLast.push_back(deque[queuePointer[threadId]++]);
                        this->nbRecv[threadId]++;
                    }
                    //At this time, this assertion have to be verify (no more clauses to recuperate)
                    assert(queuePointer[threadId] == deque.size());

                    //Update the ordersPointer 
                    this->updateOrdersPointer(ordersPointer, threadIdQueue, threadId);

                    //pop data already recuperate by all threads
                    if (minQueuePointer > 1000) this->popDataReceived(minQueuePointer, threadIdQueue, queuePointer, deque);

                    mutex.unlock();
                }
            }
            this->nbRecvAll[threadId]++;
        }


};

} // namespace pFactory

#endif