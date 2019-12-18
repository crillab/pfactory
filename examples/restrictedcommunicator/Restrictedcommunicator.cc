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
#include "pFactory.h"

void communication(pFactory::Group& group, pFactory::Communicator<int>& communicator){
   
    communicator.send(group.getThreadId());

    //To wait that all data are sent
    group.barrier.wait();
    //Receive and display all numbers of others threads
    std::vector<int> data;
    std::stringstream msg;

    communicator.recvAll(data);
    msg << "Thread (not task!) " << group.getThreadId() << " receives: ";
    for(unsigned int j = 0; j < data.size(); ++j)
        msg  << data[j] << ' ';
    pFactory::cout() << msg.str() << std::endl;
}

int main() {
    unsigned int nbThreads = 8; 
    pFactory::Group group(nbThreads);
    
    pFactory::Communicator<int> integerCommunicator1(group, {0,1,2,3}, {4,5,6,7});
    pFactory::Communicator<int> integerCommunicator2(group, {0,1,2,3,4,5}, {2,3,4,5,6,7});
    pFactory::Communicator<int> integerCommunicator3(group, {0,1}, {6,7});
    pFactory::Communicator<int> integerCommunicator4(group, {1,2,3,4,5,6,7}, {0});
    
    for(unsigned int i = 0; i < nbThreads; i++) {
        group.add([&]() {
            if (group.getThreadId() == 0)
                std::cout << "Test {0,1,2,3}, {4,5,6,7}:" << std::endl; 
            communication(group, integerCommunicator1);
            group.barrier.wait();
            if (group.getThreadId() == 0)
                std::cout << "Test {0,1,2,3,4,5}, {2,3,4,5,6,7}:" << std::endl;
            communication(group, integerCommunicator2);
            group.barrier.wait();
            if (group.getThreadId() == 0)
                std::cout << "Test {0,1}, {6,7}:" << std::endl;
            communication(group, integerCommunicator3);
            group.barrier.wait();
            if (group.getThreadId() == 0)
                std::cout << "Test {1,2,3,4,5,6,7}, {0}:" << std::endl;
            communication(group, integerCommunicator4);
            return 0;
        });
    }
    // Start the computation of all tasks
    group.start();
    // Wait until all threads are performed all tasks 
    group.wait();
}
