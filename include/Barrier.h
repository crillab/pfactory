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
#include <condition_variable>
#include <mutex>  

#ifndef barrier_H
#define	barrier_H
namespace pFactory
{
  class Barrier
  {
  private:
    std::mutex m;
    std::condition_variable cv;
    unsigned int nbThreads;
    unsigned int tmpNbThreads;
    unsigned int nbGenerations;
    
  public:
    explicit Barrier(unsigned int p_nbThreads): 
      nbThreads(p_nbThreads), 
      tmpNbThreads(p_nbThreads),
      nbGenerations(0)
      {}

    ~Barrier(){}

    //Return true if the thread is the last thread to call wait 
    bool wait()
    {
      std::unique_lock<std::mutex> lock{m};
      unsigned int tmpNbGenerations = nbGenerations;
      
      if (--tmpNbThreads==0){
        nbGenerations++;//This generation is finished
        tmpNbThreads=nbThreads;//Fix the good number of threads for the next generation
        cv.notify_all();
        return true;
      }
      //Sleep while the generation has not changed
      cv.wait(lock,[&tmpNbGenerations,this] {return tmpNbGenerations != nbGenerations;});
      return false;
    }
  };
}
#endif

