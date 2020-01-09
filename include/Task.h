
#ifndef Task_H
#define Task_H

#include <functional>
#include <climits>

namespace pFactory{
    
    enum class Status{
        notStarted, // Tasks not started yet
        inProgress, // Tasks in progress
        finished, // Tasks that have finished normaly theirs works
        stopped, // Tasks that have were stopped during their calculation 
        stopAllTasks // Task that has stopped the calculation of others tasks
    };

    inline std::ostream& operator<<(std::ostream& os, Status c)
    {
        switch(c)
        {
            case Status::notStarted: os << "notStarted";    break;
            case Status::inProgress: os << "inProgress"; break;
            case Status::finished: os << "finished";  break;
            case Status::stopped: os << "stopped";   break;
            case Status::stopAllTasks: os << "stopAllTasks";   break;
            default: os.setstate(std::ios_base::failbit);
        }
        return os;
    }
    

    class Task {
        public:
            Task(unsigned int _id, const std::function<int()>& _function):
                id(_id),
                function(_function),
                threadId(UINT_MAX),
                status(Status::notStarted),
                returnCode(INT_MAX)
            {}
            
            inline unsigned int getId(){return id;}
            inline const std::function<int()> getFunction(){return function;}
            
            inline int getReturnCode(){return returnCode;}
            inline Status getStatus(){return status;}
            inline unsigned int getThreadId(){return threadId;}
            
            inline void setStatus(Status _status){status=_status;}
            inline void setReturnCode(int _returnCode){returnCode=_returnCode;}
            inline void setThreadId(int _threadId){threadId=_threadId;}
            
        private:
            const unsigned int id;
            const std::function<int()> function; // Copy the std::function in a task (due to limited scope of the function)
            unsigned int threadId;
            Status status;
            int returnCode;
    
            
    };

    inline std::ostream& operator<<(std::ostream& os, Task task)
    {
        
        os << "[task " << task.getId();
        if (task.getReturnCode() != INT_MAX) os << " - return: " << task.getReturnCode();
        if (task.getThreadId() != UINT_MAX) os << " - thread: " << task.getThreadId();
        os << " - status: " << task.getStatus() << "]";
        return os;
    }

}


#endif