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
#include <experimental/random>

// In this example, we create a group of thread with as many tasks as threads in the group
// It is a model for a concurrent strategy. All tasks are stopped as soon as the first task is finished (the winner).

int main(){
  // A group of nbCores threads 
  pFactory::Group group(pFactory::getNbCores());
  const static int TASK_STOPPED = -3;
  
  unsigned int randomWinner = std::experimental::randint(0, (int)pFactory::getNbCores()-1);
  std::cout << "Random winner will be the task " << randomWinner << std::endl;
  for(unsigned int i = 0; i < pFactory::getNbCores();i++){
    // A task is represented by a C++11 lambda function 
    group.add([&](){
        
        // To simulate the task calculation according to the tasks id
        unsigned int nbLoops = group.getTaskId() == randomWinner?1000:1010+group.getTaskId();
        for(unsigned int j = 0; j < nbLoops;j++){ 
          if (group.isStopped()) return TASK_STOPPED; // To stop this task during its calculation if the group have to be stopped
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        pFactory::cout() << "Task " << group.getTaskId() << " (on the thread " << group.getThreadId() << ") finished" << std::endl;
	      
        // The return code of the task that has finished 
        return (int)group.getTaskId();
      });
  }
  
  // Concurrent mode by adding the parameter true to the method group.start()
  pFactory::start(group.concurrent());
  
  // Wait until all threads are performed all tasks 
  pFactory::wait(group);

  // Display the return codes (pFactory keeps the good return code of each task)
  // >=0 : Tasks finished
  // -1 : Tasks not started 
  // -2 : Task that has stopped the calculation
  // -3 : Tasks that have were stopped during their calculation 

  for(unsigned int i = 0; i < pFactory::getNbCores();i++){
    std::cout << "Task: " << i << " - code: " << group.getReturnCodes()[i] << std::endl;
  }

  // To get the winner :
  std::cout << "Winner: " << group.getWinner().getTaskId() << " (on the thread " << group.getWinner().getThreadId() << ") - returnCode: " << group.getWinner().getReturnCode() << std::endl;
}
