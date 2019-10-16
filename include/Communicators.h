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

namespace pFactory
{
struct BaseCommunicator
{
};

/*
 * To communicate between threads some information by copies.
 */
template <class T = int>
class Communicator : public BaseCommunicator
{
public:
	Communicator(Group *g);
	virtual ~Communicator();
	virtual void send(T data) = 0;
	virtual bool recv(T &data, bool &isLast) = 0;
	virtual bool recv(T &data) = 0;
	virtual bool isEmpty() = 0;
	virtual void recvAll(std::vector<T> &dataNotLast, std::vector<T> &dataLast) = 0;
	virtual void recvAll(std::vector<T> &data) = 0;

	virtual unsigned int getNbSend() = 0;
	virtual unsigned int getNbRecv() = 0;

	inline bool isOneWatchs() { return oneWatchs; }
	inline void setOneWatchs(bool p) { oneWatchs = p; }

protected:
	Group *group;
	bool oneWatchs;
};

class OrderPointer
{
public:
	OrderPointer() : next(NULL), previous(NULL), idThread(-1){};
	OrderPointer(int pidThread) : next(NULL), previous(NULL), idThread(pidThread){};

	OrderPointer *next;
	OrderPointer *previous;
	int idThread;
};

template <class T>
class MultipleQueuesCommunicator : public Communicator<T>
{
public:
	MultipleQueuesCommunicator(Group *g);
	~MultipleQueuesCommunicator();

	/*Send a data to others threads
      \param data Data to send
      Warning : user may pass a copŷ
    */
	inline void send(T data)
	{
		const unsigned int threadId = this->group->getThreadId();
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
		const unsigned int threadId = this->group->getThreadId();
		for (unsigned int threadIdQueue = 0; threadIdQueue < nbThreads; threadIdQueue++)
		{
			//Browse all queue except the queue of this thread
			if (threadIdQueue != threadId)
			{
				//These adresses don't move, so no mutex here !
				std::deque<T> &deque = vectorOfQueues[threadIdQueue];
				std::vector<unsigned int> &queuePointer = threadQueuesPointer[threadIdQueue];
				if (deque.empty())
					continue;
				else if (queuePointer[threadId] == deque.size())
					continue;
				else
					return false;
			}
		}
		return true;
	}
	
	/* Receive all elements from the communicator.
       \param data: Received elements  
       Remark2: When no data is found, nothing is added in these two parameters
    */
	inline void recvAll(std::vector<T> &data)
	{
		const unsigned int threadId = this->group->getThreadId();
		for (unsigned int threadIdQueue = 0; threadIdQueue < nbThreads; threadIdQueue++)
		{
			//Browse all queue except the queue of this thread
			if (threadIdQueue != threadId)
			{
				//These adresses don't move, so no mutex here !
				std::deque<T> &deque = vectorOfQueues[threadIdQueue];
				std::vector<unsigned int> &queuePointer = threadQueuesPointer[threadIdQueue];

				//Special issue if the watch of thread is at the end (the vector is empty)
				if (deque.empty() || queuePointer[threadId] == deque.size())
				{
					//mutex.unlock();
					continue;
				}
				std::mutex &mutex = threadMutexs[threadIdQueue];

				unsigned int &minQueuePointer = minQueuesPointer[threadIdQueue];
				std::vector<OrderPointer *> &ordersPointer = threadOrdersPointer[threadIdQueue];
				mutex.lock();

				//Verify the queue of pointers
				assert(threadOrdersPointerStart[threadIdQueue]->next != NULL);
				assert(threadOrdersPointerStart[threadIdQueue]->next->next != NULL);
				assert(threadOrdersPointerStart[threadIdQueue]->previous == NULL);

				assert(threadOrdersPointerEnd[threadIdQueue]->next == NULL);
				assert(threadOrdersPointerEnd[threadIdQueue]->previous != NULL);

				//Get the minimum and the second minimum !
				minQueuePointer = queuePointer[threadOrdersPointerStart[threadIdQueue]->next->idThread];

				//Now, recuperate clauses that I have to copy
				while (queuePointer[threadId] != deque.size())
				{
					data.push_back(deque[queuePointer[threadId]++]);
					nbRecv[threadId]++;
				}
				//At this time, this assertion have to be verify (no more clauses to recuperate)
				assert(queuePointer[threadId] == deque.size());

				//Update the ordersPointer (put the current thread in the end of the queue) (usefull to calculate the minimum and the second minimum faster)
				ordersPointer[threadId]->next->previous = ordersPointer[threadId]->previous; //Removing in the queue
				ordersPointer[threadId]->previous->next = ordersPointer[threadId]->next;	 //Removing in the queue

				ordersPointer[threadId]->next = threadOrdersPointerEnd[threadIdQueue];				 //Push back in the queue
				ordersPointer[threadId]->previous = threadOrdersPointerEnd[threadIdQueue]->previous; //Push back in the queue
				threadOrdersPointerEnd[threadIdQueue]->previous->next = ordersPointer[threadId];	 //Push back in the queue
				threadOrdersPointerEnd[threadIdQueue]->previous = ordersPointer[threadId];			 //Push back in the queue

				//pop data already recuperate by all threads
				if (minQueuePointer > 1000)
				{
					for (unsigned int i = 0; i < minQueuePointer; i++)
					{
						for (unsigned int j = 0; j < nbThreads; j++)
						{
							if (j != threadIdQueue)
								queuePointer[j]--;
						}
						deque.pop_front();
					}
				}

				mutex.unlock();
			}
		}
		nbRecvAll[threadId]++;
	}

	/* Receive all elements from the communicator.
       \param dataNotLast Elements which have not been received by all threads 
       \param dataLast Elements which have been received by all threads (others threads have already received the element)
       Remark1: dataNotLast and dataLast are useful to deal with copy 
       Remark2: When no data is found, nothing is added in these two parameters
    */
	inline void recvAll(std::vector<T> &dataNotLast, std::vector<T> &dataLast)
	{
		const unsigned int threadId = this->group->getThreadId();
		for (unsigned int threadIdQueue = 0; threadIdQueue < nbThreads; threadIdQueue++)
		{
			//Browse all queue except the queue of this thread
			if (threadIdQueue != threadId)
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
				assert(threadOrdersPointerStart[threadIdQueue]->next != NULL);
				assert(threadOrdersPointerStart[threadIdQueue]->next->next != NULL);
				assert(threadOrdersPointerStart[threadIdQueue]->previous == NULL);

				assert(threadOrdersPointerEnd[threadIdQueue]->next == NULL);
				assert(threadOrdersPointerEnd[threadIdQueue]->previous != NULL);

				//Get the minimum and the second minimum !
				minQueuePointer = queuePointer[threadOrdersPointerStart[threadIdQueue]->next->idThread];
				int idSecondQueuePointer = threadOrdersPointerStart[threadIdQueue]->next->next->idThread;
				minSecondQueuePointer = (idSecondQueuePointer == -1) ? deque.size() : queuePointer[idSecondQueuePointer];

				//Recuperate clauses that I have to no copy : it is the dataLast thread that take these clauses
				if (minQueuePointer == queuePointer[threadId])
				{ //I am a minumum : there are may be dataLast clauses with no copy
					while (queuePointer[threadId] != minSecondQueuePointer)
					{ // warning, that can be equals (severals minimums equals)!
						dataLast.push_back(deque[queuePointer[threadId]++]);
						nbRecv[threadId]++;
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

				//Update the ordersPointer (put the current thread in the end of the queue) (usefull to calculate the minimum and the second minimum faster)
				ordersPointer[threadId]->next->previous = ordersPointer[threadId]->previous; //Removing in the queue
				ordersPointer[threadId]->previous->next = ordersPointer[threadId]->next;	 //Removing in the queue

				ordersPointer[threadId]->next = threadOrdersPointerEnd[threadIdQueue];				 //Push back in the queue
				ordersPointer[threadId]->previous = threadOrdersPointerEnd[threadIdQueue]->previous; //Push back in the queue
				threadOrdersPointerEnd[threadIdQueue]->previous->next = ordersPointer[threadId];	 //Push back in the queue
				threadOrdersPointerEnd[threadIdQueue]->previous = ordersPointer[threadId];			 //Push back in the queue

				//pop data already recuperate by all threads
				if (minQueuePointer > 1000)
				{
					for (unsigned int i = 0; i < minQueuePointer; i++)
					{
						for (unsigned int j = 0; j < nbThreads; j++)
						{
							if (j != threadIdQueue)
								queuePointer[j]--;
						}
						deque.pop_front();
					}
				}

				mutex.unlock();
			}
		}
		nbRecvAll[threadId]++;
	}

	/* Receive only one data
	   \param data received
       \return false  if no element is found, true otherwise
    */
	inline bool recv(T& data)
	{
		bool nothing = false;
		return recv(data, nothing);
	}

	/* Receive only one data 
	    \param data received
		\param isLast true if this thread is the last to recuperate the data T (ie. others have already received the data T)
       \return false  if no element is found, true otherwise
	*/
	inline bool recv(T& data, bool &isLast)
	{

		const unsigned int threadId = this->group->getThreadId();
		for (unsigned int threadIdQueue = 0; threadIdQueue < nbThreads; threadIdQueue++)
		{
			//printf("Browse:%d\n",threadIdQueue);
			if (threadIdQueue != threadId)
			{
				//Browse all queue except the queue of this thread
				unsigned int i = 0;
				std::deque<T> &deque = vectorOfQueues[threadIdQueue];
				std::vector<unsigned int> &queuePointer = threadQueuesPointer[threadIdQueue];
				std::mutex &mutex = threadMutexs[threadIdQueue];
				unsigned int &minQueuePointer = minQueuesPointer[threadIdQueue];

				mutex.lock();
				//Special issue if the watch of thread is at the end or the vector is empty : no data
				if (deque.empty() || queuePointer[threadId] == deque.size())
				{
					mutex.unlock();
					continue;
				}
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
				if (minQueuePointer > 1000)
				{
					for (i = 0; i < minQueuePointer; i++)
					{
						for (unsigned int j = 0; j < nbThreads; j++)
						{
							if (j != threadIdQueue)
								queuePointer[j]--;
						}
						deque.pop_front();
					}
				}
				//printf("Recuperate %d from %d\n",ret,threadId);
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

private:
	const unsigned int nbThreads;
	std::vector<std::mutex> threadMutexs;

	std::vector<std::deque<T>> vectorOfQueues;
	std::vector<std::vector<unsigned int>> threadQueuesPointer;
	std::vector<std::vector<OrderPointer *>> threadOrdersPointer;
	std::vector<OrderPointer *> threadOrdersPointerStart;
	std::vector<OrderPointer *> threadOrdersPointerEnd;

	std::vector<unsigned int> minQueuesPointer;
	std::vector<unsigned int> minSecondQueuesPointer;

	std::vector<unsigned int> nbSend;
	std::vector<unsigned int> nbRecv;
	std::vector<unsigned int> nbRecvAll;
};

} // namespace pFactory

#endif
