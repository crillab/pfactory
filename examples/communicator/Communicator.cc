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

// In this example, each thread share an integer to the others.
// The mutex is used only for displaying data in a smart way.
// Of course, on classical usage, this mutex is useless, you
// can directly share and receive data



// Choose the way you want to receive data
//#define USE_RECVALL

int main() {
    // A group of nbCores threads 
    pFactory::Group group(pFactory::getNbCores());
    pFactory::Communicator<int> integerCommunicator(group);
    
    for(unsigned int i = 0; i < pFactory::getNbCores(); i++) {
        // Add as many tasks as threads in the group
        group.add([&]() {
            // pFactory::cout() provides a special critical section for displaying information in a smart way on the console
            pFactory::cout() << group.getTask() << " sends: " << group.getTask().getId() << std::endl;
            
            // group.getThreadId() return the id of a thread that execute this function for a group
            // group.getTaskId() return the id of this task for a group
            integerCommunicator.send(group.getTask().getId());
            
            // A group has a barrier to wait all tasks at the same moment of the execution 
            // Here, this barrier is used to wait that all tasks are sent their data 
            group.barrier.wait();

            std::stringstream msg;
#ifdef USE_RECVALL
            /* With recvAll function */
            std::vector<int> data;
            integerCommunicator.recvAll(data);
            msg << group.getTask() << " receives:";
            for(unsigned int j = 0; j < data.size(); ++j)
                msg << data[j] << ' ';
            pFactory::cout() << msg.str() << std::endl;
            
#else
            /* With recv function */
            msg << group.getTask() << " receives:";
            int data;
            while(integerCommunicator.recv(data) != false)
                msg << data << ' ';
            pFactory::cout() << msg.str() << std::endl;
#endif
            


            return 0;
        });
    }
    // Start the computation of all tasks
    group.start();
    // Wait until all threads are performed all tasks 
    group.wait();
}
