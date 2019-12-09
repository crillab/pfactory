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
#include <mutex>



int main() {
    pFactory::Group group(pFactory::getNbCores());
    
    
    pFactory::Communicator<int> integerCommunicator(&group);
    
    std::mutex m;

    for(unsigned int i = 0; i < pFactory::getNbCores(); i++) {
        group.add([&, i]() {
            int nb = rand() % 101; // Send a random number
            integerCommunicator.send(nb);

            //Receive and display all numbers of others threads
            std::vector<int> data;
            /* With recvAll function */
            integerCommunicator.recvAll(data);
	    
	    m.lock(); // Mutex is needed only for displaying information in a smart way on the console
            std::cout << "Task" << i << " receives:";
            for(unsigned int j = 0; j < data.size(); ++j)
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
