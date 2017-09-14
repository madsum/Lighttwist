#ifndef __MY_LOG__
#define __MY_LOG__

#include <stdio.h>
#include <stdarg.h>     
#include <assert.h>
#include <stdlib.h>
#include <string.h>
//#if 0
#define _DEBUG

#ifdef _DEBUG
#define MY_LOG_0(format)\
        my_log(format);
#define MY_LOG_1(format, one)\
        my_log(format, one);
#define MY_LOG_2(format, one, two)\
        my_log(format, one, two);
#define MY_LOG_3(format, one, two,three)\
        my_log(format, one, two,three);
#define MY_LOG_4(format, one, two,three,four)\
        my_log(format, one, two,three,four);
#define MY_LOG_5(format, one, two,three,four,five)\
        my_log(format, one, two,three,four,five);

#else

#define MY_LOG_0(format)
#define MY_LOG_1(format,my_logmy_logone)
#define MY_LOG_2(format,one,two)
#define MY_LOG_3(format,one,two,three)
#define MY_LOG_4(format,one,two,three,four)
#define MY_LOG_5(format,one,two,three,four,five)

#endif /* _DEBUG */
//#endif

static void my_log(char* pFormat, ...)
  {
  int ret = -1;
  FILE* fp;
  ///char* file = "c:\\temp\\log\\mylog.log";
  char* file = "/home/masum/log/my_log.log";

  fp = fopen (file,"a+");               
  
  if (pFormat)
    {
    va_list ap;
    va_start(ap, pFormat);
    vfprintf(fp, pFormat, ap);
    fflush(fp);
    va_end(ap);
    }
  fwrite("\n",1,1,fp);
  fclose (fp);
  }

#endif __MY_LOG__

