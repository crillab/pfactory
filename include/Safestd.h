#ifndef safestd_H
#define	safestd_H

#include <mutex>
#include <iostream>

namespace pFactory {
    /* A unique mutex structure to safety display thinks in std::cout or std::cerr */
    static std::mutex std_mutex;
    
    struct cout
    {
            std::unique_lock<std::mutex> lk;
            cout()
                :
                lk(std::unique_lock<std::mutex>(std_mutex))
            {

            }

            template<typename T>
            cout& operator<<(const T& _t)
            {
                std::cout << _t;
                return *this;
            }

            cout& operator<<(std::ostream& (*fp)(std::ostream&))
            {
                std::cout << fp;
                return *this;
            }
    };

    struct cerr
    {
            std::unique_lock<std::mutex> lk;
            cerr()
                :
                lk(std::unique_lock<std::mutex>(std_mutex))
            {

            }

            template<typename T>
            cerr& operator<<(const T& _t)
            {
                std::cerr << _t;
                return *this;
            }

            cerr& operator<<(std::ostream& (*fp)(std::ostream&))
            {
                std::cerr << fp;
                return *this;
            }
    };

}

#endif