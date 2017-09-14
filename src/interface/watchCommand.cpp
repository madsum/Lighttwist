#include "watchCommand.h"
#include "lighttwist.h"

bool WatchCommand::m_IsRunning = false;

WatchCommand::WatchCommand()
{
    std::cout<<__PRETTY_FUNCTION__; //commnet it
}

void* WatchCommand::reader_thread_function ( void* aParam )
{
    char *read_buffer;
    int str_len = 0;
    int size = 0;
    int fd=0;
    FILE* file_fp=NULL;
    bool isReplyReady = false;

    lighttwistUI* lw = static_cast<lighttwistUI*> ( aParam );
    str_len = strlen ( PLAY );
    read_buffer = ( char* ) malloc ( sizeof ( unsigned char ) *str_len );

    while ( m_IsRunning )
    {
        file_fp = fopen ( REQUEST_FILE,"a+" );
        fd = fileno ( file_fp );
        isReplyReady = false;

        if ( !fd )
            continue;

        // Get an exclusive lock or continue
        if ( flock ( fd,LOCK_EX ) == -1 )
            continue;

        size = read ( fd, read_buffer, str_len );

        if ( size > 0 )
        {
            if ( memcmp ( read_buffer,PLAY,str_len ) == 0 )
            {
                printf ( "\nI got command #######  %s\n", read_buffer ); //commnet it
                lw->m_IsTimeToPlay = true;
                lw->m_ignore_cb = true;
                Fl::wait();
                isReplyReady = true;
            }
            else if ( memcmp ( read_buffer,STOP,str_len ) == 0 )
            {
                printf ( "\nI got command #######  %s\n", read_buffer ); // commnet it
                lw->m_IsTimeToPlay = false;
                lw->m_ignore_cb = true;
                Fl::wait();
                isReplyReady = true;
            }
            memset ( read_buffer,NULL,str_len );
        }

        if ( isReplyReady )
        {
            FILE *reply_file = fopen ( REPLY_FILE, "a+" );
            int fd_reply = fileno ( reply_file );
            flock ( fd_reply,LOCK_EX );
            ftruncate ( fd_reply,0 );

            if ( !fd_reply )
                printf ( "error: # %d\n", errno );

            int size= write ( fd_reply, "OK", strlen ( "OK" ) );

            flock ( fd_reply,LOCK_UN ); // unlock
            close ( fd_reply );
        }

        ftruncate ( fd,0 );
        flock ( fd,LOCK_UN ); // unlock
        close ( fd );
        sleep ( 1 );

    }
    free ( read_buffer );
    return NULL;
}

void WatchCommand::command_reply ( bool create )
{
    int fd=0;

    if ( create )
    {
        fd = open ( REPLY_FILE,O_RDONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH );
        if ( !fd )
            printf ( "error: # %d\n", errno );

        close ( fd );
    }
    else
    {
        if ( remove ( REPLY_FILE ) != 0 )
            printf ( "error: # %d\n", errno );

    }
}


void WatchCommand::start_stop_thread ( bool start_stop, const lighttwistUI& lw )
{
    m_IsRunning = start_stop;
    std::cout<<__PRETTY_FUNCTION__<<" start_stop = "<<m_IsRunning<<"\n"; //commne it
    if ( m_IsRunning )
    {
        pthread_create ( &m_ReadCommandThread,NULL, WatchCommand::reader_thread_function, ( void* ) &lw );
    }
}
