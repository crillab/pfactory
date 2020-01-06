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

const static int TASK_STOPPED = -3;

void createTask(pFactory::Group& group, unsigned int randomWinnerGroup, unsigned int randomWinnerTask){
  // A task is represented by a C++11 lambda function 
  group.add([randomWinnerGroup, randomWinnerTask, &group](){
      // To simulate the task calculation according to the tasks id
      unsigned int randomNumber = std::experimental::randint(0, (int)group.getTaskId()-1);
      unsigned int nbLoops = group.getTaskId() == randomWinnerTask and group.getGroupId() == randomWinnerGroup?1000:1010+randomNumber;
      for(unsigned int j = 0; j < nbLoops;j++){ 
        if (group.isStopped()) return TASK_STOPPED; // To stop this task during its calculation if the group have to be stopped
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      pFactory::cout() << "Group "<< group.getGroupId() << " - Task " << group.getTaskId() << " (on the thread " << group.getThreadId() << ") finished" << std::endl;
      
      // The return code of the task that has finished 
      return (int)group.getTaskId();
  });
}


int main(){
  
  const static unsigned int nbThreads = 40;
  const static unsigned int nbGroups = 4;
  const static unsigned int nbTasksPerGroup = nbThreads/nbGroups;

  std::vector<pFactory::Group> groups(nbGroups, pFactory::Group(nbTasksPerGroup)); //Mupliple groups
  std::cout << "Example of mutliple groups in concurrence Group with " << nbGroups << " of " << nbTasksPerGroup << " tasks" << std::endl;

  unsigned int randomWinnerGroup = std::experimental::randint(0, (int)nbGroups-1);
  unsigned int randomWinnerTask = std::experimental::randint(0, (int)nbTasksPerGroup-1);
  std::cout << "Random winner will be the task " << randomWinnerTask << " of the group " << randomWinnerGroup << std::endl;
  
  for (unsigned int i = 0;i < nbGroups;i++){
    groups[i].concurrent(); //Concurrency of tasks (the first task to finish its calculation stop all others tasks of its group)
    for (unsigned int j = 0;j < nbTasksPerGroup;j++)
      createTask(groups[i], randomWinnerGroup, randomWinnerTask);
  }

  pFactory::start(groups[0], groups[1], groups[2], groups[3]).concurrent(); //Concurrency of groups (the first group to finish its calculation stop all others groups)
  pFactory::wait(groups);

  // Display the return codes (pFactory keeps the good return code of each task)
  // >=0 : Tasks finished
  // -1 : Tasks not started 
  // -2 : Task that has stopped the calculation
  // -3 : Tasks that have were stopped during their calculation 

  for (unsigned int i = 0;i < nbGroups;i++){
    for (unsigned int j = 0;j < nbTasksPerGroup;j++)
      pFactory::cout() << "Group "<< groups[i].getGroupId() << " - Task " << j << " - Code: " << groups[i].getReturnCodes()[j] << std::endl;
    // To get the winner :
    std::cout << "Winner of the group " << groups[i].getGroupId() << " is the task " << groups[i].getWinner().getTaskId() << " (on the thread " << groups[i].getWinner().getThreadId() << ") - returnCode: " << groups[i].getWinner().getReturnCode() << std::endl;
  }
}
