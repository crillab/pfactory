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
    static const unsigned int nbTasks = pFactory::getNbCores()/2;
    pFactory::Group group1(nbTasks);
    pFactory::Group group2(nbTasks);
    
    pFactory::Intercommunicator<int> integerInterommunicator(group1, group2);
    
    for(unsigned int i = 0; i < nbTasks; i++) {
        // Add as many tasks as threads in the group
        group1.add([&]() {
            // pFactory::cout() provides a special critical section for displaying information in a smart way on the console
            pFactory::cout() << "Task" << group1.getTaskId() << " (on the thread "<< group1.getThreadId() <<") sends: " << group1.getTaskId() << std::endl;

        
            return 0;
        });
    }
    // Start the computation of all tasks
    group1.start();
    // Wait until all threads are performed all tasks 
    group1.wait();
}
