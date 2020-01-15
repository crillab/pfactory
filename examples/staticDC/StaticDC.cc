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

// In this example, we create a group of thread with a lot of tasks.
// It is a model for a static divide and conquer strategy.
// Create tasks that represent subproblems (divide phase) and calculate all theses tasks (conquer phase) 

int main(){
  // A group of nbCores threads 
  pFactory::Group group(pFactory::getNbCores());
  

  // Divide phase : add as many tasks as you want
  const static unsigned int nbTasks = 1000;

  for(unsigned int i = 0; i < nbTasks;i++){
    // A task is represented by a C++11 lambda function 
    group.add([&](){
        
        // You can stop the process calculation when you want thanks to the method group.stop() 
        // It is very usefull if a task induced the interruption of all others tasks 
        if (group.getTask().getId() == 500){
          pFactory::cout() << "Task " << group.getTask().getId() << " (on the thread " << group.getThreadId() << ") stops the group!" << std::endl;
          group.stop();
          group.getTask().setDescription("stop all others tasks");
          return (int)group.getTask().getId();
        }

        // To simulate the task calculation
        for(unsigned int j = 0; j < 50;j++){ 
          if (group.isStopped()){
            group.getTask().setDescription("stopped during its computation");
            return (int)group.getTask().getId();
          } // To stop this task during its calculation if the group have to be stopped
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        // The return code of the task that has finished 
        group.getTask().setDescription("expected end");
        pFactory::cout() << group.getTask() << std::endl;
        return (int)group.getTask().getId();
      });
  }
  
  pFactory::Controller controller(group);
  controller.start();// Conquer phase : start the computation of all tasks
  controller.wait();// Wait until all threads are performed all tasks 

  for(auto &task: group.getTasks()) std::cout << task << std::endl;  
}
