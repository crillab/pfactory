
#ifndef Task_H
#define Task_H

#include <functional>
#include <climits>

namespace pFactory{
    
    enum class Status{
        notStarted, // Tasks not started yet
        inProgress, // Tasks in progress
        terminated, // Tasks that have finished normaly theirs works
    };

    inline std::ostream& operator<<(std::ostream& os, Status c)
    {
        switch(c)
        {
            case Status::notStarted: os << "notStarted";    break;
            case Status::inProgress: os << "inProgress"; break;
            case Status::terminated: os << "terminated";  break;
            default: os.setstate(std::ios_base::failbit);
        }
        return os;
    }
    

    class Task {
        public:
            Task():
                id(0),
                function(),
                threadId(UINT_MAX),
                status(Status::notStarted),
                returnCode(INT_MAX),
                description(std::string("empty task"))
            {}

            Task(unsigned int _id, const std::function<int()>& _function):
                id(_id),
                function(_function),
                threadId(UINT_MAX),
                status(Status::notStarted),
                returnCode(INT_MAX),
                description(std::string(""))
            {}

            
            
            inline unsigned int getId() const {return id;}
            inline std::function<int()> const getFunction(){return function;}
            
            inline int getReturnCode() const {return returnCode;}
            inline Status getStatus() const {return status;}
            inline unsigned int getThreadId() const {return threadId;}
            inline std::string& getDescription() {return description;}

            inline void setStatus(Status _status){status=_status;}
            inline void setReturnCode(int _returnCode){returnCode=_returnCode;}
            inline void setThreadId(int _threadId){threadId=_threadId;}
            inline void setDescription(std::string _description){description = _description;}


        private:
            const unsigned int id;
            const std::function<int()> function; // Copy the std::function in a task (due to limited scope of the function)
            unsigned int threadId;
            Status status;
            int returnCode;
            std::string description;
    
            
    };

    inline std::ostream& operator<<(std::ostream& os, Task task)
    {
        
        os << "[task " << task.getId();
        if (task.getReturnCode() != INT_MAX) os << " - return: " << task.getReturnCode();
        if (task.getThreadId() != UINT_MAX) os << " - thread: " << task.getThreadId();
        if (!task.getDescription().empty()) os << " - description: " << task.getDescription();
        os << " - status: " << task.getStatus() << "]";
        return os;
    }
}


#endif