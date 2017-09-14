#include "Thread.h"

Thread::Thread(){}

int Thread::Start(void * arg)
{
   pthread_mutex_init(&Mutex, NULL);

   pthread_attr_init(&Attr);
   pthread_attr_setdetachstate(&Attr, PTHREAD_CREATE_JOINABLE);

   setArg(arg); // store user data
   int code=0;
   code = pthread_create(&ThreadId,&Attr,Thread::EntryPoint,this);
   if (code) fprintf(stderr,"Error starting new thread.\n");

   pthread_attr_destroy(&Attr);

   pthread_mutex_destroy(&Mutex);

   return code;
}

int Thread::Run(void * arg)
{
   Setup();
   Execute( arg );
   pthread_exit((void *) 0);
   return 0;
}

int Thread::WaitForExit()
{
  int ret;
  void *status;
  ret = pthread_join(ThreadId, &status);
  if (ret) fprintf(stderr,"Error joining thread.\n");
  return ret;
}

int Thread::LockData()
{
  pthread_mutex_lock(&Mutex);
  return 0;
}

int Thread::UnlockData()
{
   pthread_mutex_unlock(&Mutex);
  return 0;
}

/*static */
void * Thread::EntryPoint(void * pthis)
{
   Thread * pt = (Thread*)pthis;
   pt->Run( (void *)(NULL) );
   return NULL;
}

void Thread::Execute(void *arg)
{
        // Do execution here
}

void Thread::Setup()
{
        // Do setup here
}

