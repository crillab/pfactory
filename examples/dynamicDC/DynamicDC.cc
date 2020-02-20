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
// It is a model for a dynamic divide and conquer (DC) strategy.
// Create tasks that represent subproblems (divide phase) and calculate all theses tasks (conquer phase) 
// In addition, some others tasks can be add during the conquer phase (the division is then called dynamic)

// staticDC: division of subproblems befores theirs calculation
// dynamicDC: division of subproblems during theirs calculation

const static unsigned int nbTasks = 1000;

int algorithm(pFactory::Group& group, bool dynamic){
  // To simulate the task calculation
  for(unsigned int j = 0; j < 100;j++){ 
    if (group.isStopped()){
      group.getTask().setDescription("stopped during its computation");
      return (int)group.getTask().getId();
    } // To stop this task during its calculation if the group have to be stopped
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  // Dynamic divide phase
  if (group.getTask().getId() < nbTasks)
    group.add([&](){return algorithm(group, true);});
  
  // The return code of the task that has finished 
  
  group.getTask().setDescription(dynamic == true? "dynamic expected end": "static expected end");
  pFactory::cout() << group.getTask() << std::endl;
  return (int)group.getTask().getId();
}

int main(){
  // A group of nbCores threads 
  pFactory::Group group(pFactory::getNbCores());

  // First divide phase : add firstly 20 tasks  
  for(unsigned int i = 0; i < 20;i++)
    // A task is represented by a C++11 lambda function 
    group.add([&](){return algorithm(group, false);});
  
  //By default, a group the lastest tasks added (is set to group.popBack()) 
  //To calculate firstly the first tasks (in the order of group.add() methods)
  group.popFront();

  pFactory::Controller controller(group);
  controller.start();// Conquer phase : start the computation of all tasks
  controller.wait();// Wait until all threads are performed all tasks 

  for(auto &task: group.getTasks()) std::cout << task << std::endl;  
}
