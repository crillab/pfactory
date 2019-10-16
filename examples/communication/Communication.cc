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


// In this example, each thread share an integer to the others.
// The mutex is used only for displaying data in a smart way.
// Of course, on classical usage, this mutex is useless, you
// can directly share and receive data



// Choose the way you want to receive data
//#define USE_RECVALL

int main() {
    // A group of nbCores threads 
    pFactory::Group group(pFactory::getNbCores());
    pFactory::MultipleQueuesCommunicator<int> integerCommunicator(&group);
    std::mutex m;

    for(unsigned int i = 0; i < pFactory::getNbCores(); i++) {
        // Add as many tasks as threads in the group
        group.add([&, i]() {
            m.lock(); // Mutex is needed only for displaying information in a smart way on the console
            int nb = rand() % 101; // Send a random number
            std::cout << "Task" << i << " sends: " << nb << std::endl;
            integerCommunicator.send(nb);
            m.unlock();

            //Receive and display all numbers of others threads
#ifdef USE_RECVALL
            std::vector<int> data;
            /* With recvAll function */
            integerCommunicator.recvAll(data);
            m.lock(); // Mutex is needed only for displaying information in a smart way on the console
            std::cout << "Task" << i << " receives:";
            for(unsigned int j = 0; j < data.size(); ++j)
                std::cout << data[j] << ' ';
            std::cout << std::endl;
            m.unlock();
#else
            m.lock(); // Mutex is needed only for displaying information in a smart way on the console
            std::cout << "Task" << i << " receives:";
            int data;
            while(integerCommunicator.recv(data) != false)
                std::cout << data << ' ';

            std::cout << std::endl;
            m.unlock();

#endif
            /* With recv function */


            return 0;
        });
    }
    // Start the computation of all tasks
    group.start();
    // Wait until all threads are performed all tasks 
    group.wait();
}
