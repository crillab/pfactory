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

const static int TASK_STOPPED = -3;

void swap_int(int* a, int* b){ int tmp = *a; *a=*b; *b=tmp; }

inline int randint_(int upper, int lower){
  return((rand() % (upper-lower+1)) + lower);
}

int randint(int a,int b)
{
    if (b > a) swap_int(&a,&b);
    return randint_(a,b);
}

void createTask(pFactory::Group& group, unsigned int randomWinnerGroup, unsigned int randomWinnerTask){
  // A task is represented by a C++11 lambda function 
  group.add([randomWinnerGroup, randomWinnerTask, &group](){
      // To simulate the task calculation according to the tasks id
      unsigned int randomNumber = randint(0, (int)group.getTask().getId()-1);
      unsigned int nbLoops = group.getTask().getId() == randomWinnerTask and group.getId() == randomWinnerGroup?1000:1010+randomNumber;
      for(unsigned int j = 0; j < nbLoops;j++){ 
        if (group.isStopped()){ 
          group.getTask().setDescription("stopped during its computation");
          return (int)group.getTask().getId(); // To stop this task during its calculation if the group have to be stopped
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      
      group.getTask().setDescription("expected end");
      pFactory::cout() << "[Group "<< group.getId() << "]" << group.getTask() << std::endl;
      // The return code of the task that has finished 
      return (int)group.getTask().getId();
  });
}


int main(){
  
  const static unsigned int nbThreads = 40;
  const static unsigned int nbGroups = 4;
  const static unsigned int nbTasksPerGroup = nbThreads/nbGroups;

  std::vector<pFactory::Group> groups(nbGroups, pFactory::Group(nbTasksPerGroup)); //Mupliple groups
  std::cout << "Example of mutliple groups in concurrence with " << nbGroups << " groups of " << nbTasksPerGroup << " tasks" << std::endl;

  unsigned int randomWinnerGroup = randint(0, (int)nbGroups-1);
  unsigned int randomWinnerTask = randint(0, (int)nbTasksPerGroup-1);
  std::cout << "Random winner will be (but maybe not) the task " << randomWinnerTask << " of the group " << randomWinnerGroup << std::endl;
  
  for (unsigned int i = 0;i < nbGroups;i++){
    groups[i].concurrent(); //Concurrency of tasks (the first task to finish its calculation stops all others tasks of its group)
    for (unsigned int j = 0;j < nbTasksPerGroup;j++)
      createTask(groups[i], randomWinnerGroup, randomWinnerTask);
  }
  
  pFactory::Controller controller(groups);
  controller.start();
  controller.concurrent(); //Concurrency of groups (the first group to finish its calculation stop all others groups)
  controller.wait();

  for (unsigned int i = 0;i < nbGroups;i++){
    for (auto task: groups[i].getTasks())
      std::cout << "[Group " << groups[i].getId() << "]" << task << std::endl;
    // To get the winners of each group (the first task that is finished) :
    std::cout << "[Group " << groups[i].getId() << "] The winner is " << groups[i].getWinner() << std::endl;
  }
  // To get the winner group (the first group that is terminated):
  std::cout << "The winner group is [Group " << controller.getWinner()->getId() << "] with the task " << controller.getWinner()->getWinner() << std::endl;
  
}
