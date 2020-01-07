
#ifndef Enum_H
#define Enum_H

#include <functional>
#include <climits>

namespace pFactory{
    
    enum class Status{
        notStarted = 1, // Tasks not started yet
        inProgress = 2, // Tasks in progress
        finished = 3, // Tasks that have finished normaly theirs works
        stopped = 4, // Tasks that have were stopped during their calculation 
        stopAllTasks = 5 // Task that has stopped the calculation of others tasks

        
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
            static unsigned int taskCount; //To get the id of a group

            Task(const std::function<int()>& _function):
                id(taskCount++),
                function(_function),
                threadId(UINT_MAX),
                status(Status::notStarted),
                returnCode(0)
            {}
            
            inline void setId(int _id){id=_id;}
            inline void setReturnCode(int _returnCode){returnCode=_returnCode;}
            inline void setStatus(Status _status){status=_status;}
            inline void setThreadId(int _threadId){threadId=_threadId;}

            inline int getReturnCode(){return returnCode;}
            inline Status getStatus(){return status;}
            inline unsigned int getId(){return id;}
            inline unsigned int getThreadId(){return threadId;}

            inline const std::function<int()>& getFunction(){return function;}
            
        private:
            unsigned int id;
            const std::function<int()>& function;
            unsigned int threadId;
            Status status;
            int returnCode;


    };

    inline std::ostream& operator<<(std::ostream& os, Task task)
    {
        os << "[task " << task.getId() << "]";
        return os;
    }

}


#endif