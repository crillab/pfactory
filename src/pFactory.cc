#include "pFactory.h"

namespace pFactory{
    
    unsigned int getNbCores(){
        std::ifstream cpuinfo("/proc/cpuinfo");
        return (!std::thread::hardware_concurrency())?
        (std::count(std::istream_iterator<std::string>(cpuinfo),
            std::istream_iterator<std::string>(),
            std::string("processor"))):
        std::thread::hardware_concurrency();
    }
}