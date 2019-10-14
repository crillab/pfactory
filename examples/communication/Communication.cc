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
#include <mutex>

int main(){
  pFactory::Group group(pFactory::getNbCores());
  
  pFactory::Communicator<int>* integerCommunicator = new pFactory::MultipleQueuesCommunicator<int>(&group, 0);
  std::mutex m;
  
  for(unsigned int i = 0; i < pFactory::getNbCores();i++){
    group.add([&i, &m, &integerCommunicator](){
      integerCommunicator->send(i);
      std::vector<int> data;
      integerCommunicator->recvAll(data);
      m.lock();
      std::cout << "Thread " << i << " :";	
      for(unsigned int i=0; i<data.size(); ++i)
        std::cout << data[i] << ' ';
      std::cout << std::endl;
      m.unlock();
      return 0;
      });
  }
  group.start();
  group.wait();
  delete integerCommunicator;
}
