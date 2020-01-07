
namespace pFactory{
    /* To represent a winner in a concurrential method */
    class Winner {
        public:
            Winner(unsigned int pthreadId, unsigned int ptaskId, unsigned int preturnCode)
                : threadId(pthreadId), taskId(ptaskId), returnCode(preturnCode){}
        
            Winner()
                : threadId(UINT_MAX),  taskId(UINT_MAX), returnCode(UINT_MAX){}

            inline unsigned int getThreadId(){return threadId;}
            inline unsigned int getTaskId(){return taskId;}
            inline unsigned int getReturnCode(){return returnCode;}

            inline void setWinner(unsigned int pthreadId, unsigned int ptaskId, unsigned int preturnCode){
                threadId=pthreadId;
                taskId=ptaskId;
                returnCode=preturnCode;
            }
        
        private:
            unsigned int threadId;
            unsigned int taskId;
            unsigned int returnCode;

    };
}