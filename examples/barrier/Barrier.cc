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
    unsigned int nbThreadsBarrier = 4; 
    pFactory::Group group1(nbThreads);
    pFactory::Group group2(nbThreads);


    pFactory::Barrier myBarrier(nbThreadsBarrier);

    for(unsigned int i = 0; i < nbThreads; i++) {
        group1.add([&]() {
            // A group has its own barrier. All threads of a group have to execute the method barrier.wait() in order to synchronize all threads and pass the barrier.
            // If some threads call barrier.wait() and at least one of threads does not call barrier.wait() then some threads are bloked in the barrier.  
            pFactory::cout() << "Before the first call to the barrier !" << std::endl;
            group1.barrier.wait();
            pFactory::cout() << "Before the second call to the barrier and after the first call!" << std::endl;
            group1.barrier.wait();
            pFactory::cout() << "After all calls to the barrier!" << std::endl;
            return 0;
        });
        
        group2.add([&]() {
            pFactory::cout() << "True id: " <<std::this_thread::get_id() << "- id: " << group2.getThreadId() << std::endl;
            
            //pFactory::cout() << "Launch " << group2.getThreadId() << " limit:" << nbThreadsBarrier << std::endl;
            if (group2.getThreadId() < nbThreadsBarrier){
                //pFactory::cout() << "Before the first call to the barrier !" << std::endl;
                myBarrier.wait();
                //pFactory::cout() << "Before the second call to the barrier and after the first call!" << std::endl;
                myBarrier.wait();
                //pFactory::cout() << "After all calls to the barrier!" << std::endl;
            }else{
                //pFactory::cout() << "Without barrier! " << group2.getThreadId() << std::endl;
            }
            return 0;
        });
    }

    std::cout << "A barrier of "<< nbThreads << " threads on the group of " << nbThreads << " threads:"<< std::endl;
    group1.start(); 
    group1.wait();

    std::cout << "A barrier of "<< nbThreadsBarrier << " threads on the group of " << nbThreads << " threads:"<< std::endl;
    group2.start(); 
    group2.wait();

}
