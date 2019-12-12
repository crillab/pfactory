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
#include "Communicators.h"
#include <sstream> 



int main() {
    unsigned int nbThreads = 8; 
    pFactory::Group group(nbThreads);
    
    std::vector<bool> senders({true, true, true, true, false, false, false, false});
    std::vector<bool> receivers({false, false, false, false, true, true, true, true});
    
    pFactory::Communicator<int> integerCommunicator(&group, senders, receivers);

    
    for(unsigned int i = 0; i < nbThreads; i++) {
        group.add([&, i]() {
            // Warning: i is not the ith thread, use the group.getThreadId() function !
            integerCommunicator.send(group.getThreadId());

            //To wait that all data are sent
            group.barrier.wait();

            //Receive and display all numbers of others threads
            std::vector<int> data;
            std::stringstream msg;

            integerCommunicator.recvAll(data);
	    
	        msg << "Task" << group.getThreadId() << " receives:";
            for(unsigned int j = 0; j < data.size(); ++j)
                msg  << data[j] << ' ';
            pFactory::cout() << msg.str() << std::endl;
            return 0;
        });
    }
    // Start the computation of all tasks
    group.start();
    // Wait until all threads are performed all tasks 
    group.wait();
}
