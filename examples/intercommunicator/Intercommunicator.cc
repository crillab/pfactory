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

// In this example, each thread share an integer to the others.
// The mutex is used only for displaying data in a smart way.
// Of course, on classical usage, this mutex is useless, you
// can directly share and receive data



// Choose the way you want to receive data
//#define USE_RECVALL

int main() {
    static const unsigned int nbTasks = pFactory::getNbCores()/2;
    pFactory::Group group1(nbTasks);
    pFactory::Group group2(nbTasks);
    
    pFactory::Intercommunicator<int> integerIntercommunicator(group1, group2);
    pFactory::Barrier mybarrier(pFactory::getNbCores());

    for(unsigned int i = 0; i < nbTasks; i++) {
        group1.add([&]() {
            pFactory::cout() << "[" << group1.getId() << "]" << group1.getTask() << " sends: " << group1.getTask().getId() << std::endl;
            
            integerIntercommunicator.send(group1.getTask().getId());
            mybarrier.wait(); //To ensure that all messages are sent before the receive operation.
            return 0;
        });

        group2.add([&]() {
            mybarrier.wait(); //To ensure that all messages are sent before the receive operation.

            /* With recvAll function */
            std::vector<int> data;
            std::stringstream msg;
            integerIntercommunicator.recvAll(data);
            msg << "[" << group2.getId() << "]" << group2.getTask() << "receives:";
            for(unsigned int j = 0; j < data.size(); ++j)
                msg << data[j] << ' ';
            pFactory::cout() << msg.str() << std::endl;
            return 0;
        });


    }

    pFactory::Controller controller({&group1, &group2});
    controller.start();
    controller.wait();

}
