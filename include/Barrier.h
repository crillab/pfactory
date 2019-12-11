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
    std::mutex _mutex;
    std::condition_variable _cv;
    std::size_t _count;
    
  public:
    explicit Barrier(std::size_t count) : _count(count){ }
    void wait()
    {
      std::unique_lock<std::mutex> lock{_mutex};
      if (--_count == 0){
        _cv.notify_all();
      }else{
        _cv.wait(lock,[this] {return _count == 0;});
      }
    }
  };
}
#endif
