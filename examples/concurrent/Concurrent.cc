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

// In this example, we create a group of thread with as many tasks as threads in the group
// It is a model for a concurrent strategy. All tasks are stopped as soon as the first task is finished (the winner).

void swap_int(int* a, int* b){ int tmp = *a; *a=*b; *b=tmp; }

inline int randint_(int upper, int lower){
  return((rand() % (upper-lower+1)) + lower);
}

int randint(int a,int b)
{
  if (b > a) swap_int(&a,&b);
  return randint_(a,b);
}

int main(){
  // A group of nbCores threads 
  pFactory::Group group(pFactory::getNbCores());
  
  unsigned int randomWinner = randint(0, (int)pFactory::getNbCores()-1);
  std::cout << "Random winner will be the task " << randomWinner << std::endl;
  for(unsigned int i = 0; i < pFactory::getNbCores();i++){
    // A task is represented by a C++11 lambda function 
    group.add([&](){
        
        // To simulate the task calculation according to the tasks id
        unsigned int nbLoops = group.getTask().getId() == randomWinner?1000:1010+group.getTask().getId();
        for(unsigned int j = 0; j < nbLoops;j++){ 
          if (group.isStopped()){
             group.getTask().setDescription("stopped during its computation");
             return (int)group.getTask().getId(); // To stop this task during its calculation if the group have to be stopped
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
	      
        // Set the status of the task that has finished 
        group.getTask().setDescription("expected end");
        pFactory::cout() << group.getTask() << std::endl;
        return (int)group.getTask().getId();
      });
  }
  pFactory::Controller controller(group.concurrent()); //Concurrent mode for this group
  controller.start();
  controller.wait();

  // Information displaying 
  std::cout << "Tasks: " << std::endl;
  for(auto &task: group.getTasks())std::cout << task << std::endl;
  std::cout << "Winner: " << std::endl << group.getWinner() << std::endl;
}
