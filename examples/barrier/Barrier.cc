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

int main() {
    unsigned int nbThreads = 8;
    unsigned int nbThreadsBarrier = 4; 
    pFactory::Group group1(nbThreads);
    pFactory::Group group2(nbThreads);


    pFactory::Barrier myBarrier(nbThreadsBarrier);

    for(unsigned int i = 0; i < nbThreads; i++) {
        group1.add([&]() {
            // A group has its own barrier. All threads of a group have to execute the method barrier.wait() in order to synchronize all threads and pass the barrier.
            // If some threads call barrier.wait() and at least one of threads does not call barrier.wait() then some threads are bloked in the barrier.  
            pFactory::cout() << group1.getTask() << "Before the first call!" << std::endl;
            group1.barrier.wait();
            pFactory::cout() << group1.getTask() << "Before the second call and after the first call!" << std::endl;
            group1.barrier.wait();
            pFactory::cout() << group1.getTask() << "After all calls!" << std::endl;
            return 0;
        });
        
        group2.add([&]() {
            if (group2.getTask().getId() < nbThreadsBarrier){
                pFactory::cout() << group2.getTask() << "Before the first call!" << std::endl;
                myBarrier.wait();
                pFactory::cout() << group2.getTask() << "Before the second call and after the first call!" << std::endl;
                myBarrier.wait();
                pFactory::cout() << group2.getTask() << "After all calls!" << std::endl;
            }else{
                pFactory::cout() << group2.getTask() << "Without barrier!" << std::endl;
            }
            return 0;
        });
    }

    std::cout << std::endl << "A barrier of "<< nbThreads << " threads on a group of " << nbThreads << " threads:"<< std::endl;
    group1.start(); 
    group1.wait();
     
    std::cout << std::endl << "A barrier of "<< nbThreadsBarrier << " threads on a group of " << nbThreads << " threads:"<< std::endl;
    group2.start(); 
    group2.wait();

}
