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

// In this example, we create a group of thread saying hello world

int main(){
  // A group of nbCores threads 
  pFactory::Group groupWrongDisplay(pFactory::getNbCores());
  pFactory::Group groupRightDisplay(pFactory::getNbCores());
  
  // Add as many tasks as threads in the group
  for(unsigned int i = 0; i < pFactory::getNbCores();i++){
    // A task is represented by a C++11 lambda function 
    // The capture list [i, &group] meaning that i (resp. group) is captured by value (resp. by reference)  
    groupWrongDisplay.add([i, &groupWrongDisplay](){
        // pFactory::cout() provides a special critical section for displaying information
	      std::cout << "Task " << i << " (on the thread " << groupWrongDisplay.getThreadId() << ") says Hello World on stdout" << std::endl;
	      std::cerr << "Task " << i << " (on the thread " << groupWrongDisplay.getThreadId() << ") says Hello World on stderr" << std::endl;
        return 0;
      });
    
    groupRightDisplay.add([i, &groupRightDisplay](){
        // pFactory::cout() provides a special critical section for displaying information
	      pFactory::cout() << "Task " << i << " (on the thread " << groupRightDisplay.getThreadId() << ") says Hello World on stdout" << std::endl;
	      pFactory::cerr() << "Task " << i << " (on the thread " << groupRightDisplay.getThreadId() << ") says Hello World on stderr" << std::endl;
	      return 0;
      });
  }

  pFactory::cout() << "Without pFactory::cout():" << std::endl;
  groupWrongDisplay.start();
  groupWrongDisplay.wait();

  pFactory::cout() << "With pFactory::cout():" << std::endl;
  groupRightDisplay.start();
  groupRightDisplay.wait();
}
