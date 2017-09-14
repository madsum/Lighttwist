#ifndef LT_THREAD
#define LT_THREAD

#include <pthread.h>
#include <stdio.h>

class Thread
{
   public:
      Thread();
      virtual ~Thread(){};
      int Start(void * arg);
      int WaitForExit();
      int LockData();
      int UnlockData();
   protected:
      int Run(void * arg);
      static void * EntryPoint(void*);
      virtual void Setup();
      virtual void Execute(void*);
      void * getArg() const {return Arg;}
      void setArg(void *a){Arg = a;}
   private:
      pthread_t ThreadId;
      pthread_attr_t Attr;
      pthread_mutex_t Mutex;
      void * Arg;

};

#endif
