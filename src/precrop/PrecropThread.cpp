#include "PrecropThread.h"

PrecropThread::PrecropThread(int i, char *cmd_arg,ThreadInterface *inter)
{
  int it=0;
  index=i;
  ti=inter;
  strcpy(cmd,cmd_arg);
  Start((void *)(&it));
}

void PrecropThread::Execute(void * arg)
{
  int ret;

  ret=system(cmd);

  ti->ThreadCallback(index,ret);
}

void PrecropThread::Setup()
{
    // Do any setup here
}

