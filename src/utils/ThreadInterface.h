#ifndef LT_THREAD_INTERFACE
#define LT_THREAD_INTERFACE

//A class using the Thread class should implement the ThreadInterface class
//There are no interfaces in C++! This class is only meant to show what functions should be implemented
//when extending the ThreadInterface class

class ThreadInterface
{
  public:
  ThreadInterface(){};
  virtual ~ThreadInterface(){};
  virtual void ThreadCallback(int index,int status);
};

#endif
