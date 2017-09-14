#ifndef __WATCHCOMMAND_H__
#define __WATCHCOMMAND_H__

#include <pthread.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <string.h> 
#include <stdio.h> 
#include <errno.h> 
#include <unistd.h>


#define PLAY "play"
#define STOP "stop"
#define REQUEST_FILE "/var/www/request"
#define REPLY_FILE "/var/www/reply"

class lighttwistUI;

class WatchCommand
{
public:
    explicit WatchCommand();
    ~WatchCommand()
    {
        m_IsRunning = false;
        pthread_join(m_ReadCommandThread, NULL);
    }

    static void* reader_thread_function(void* aParam);
    void start_stop_thread(bool start_stop, const lighttwistUI& lw);
    void command_reply(bool create );
    inline bool IsThreadRunning()
    {
        return m_IsRunning;
    }
    
private:
    static bool     m_IsRunning;
    pthread_t       m_ReadCommandThread;
};

#endif //__WATCHCOMMAND_H__

