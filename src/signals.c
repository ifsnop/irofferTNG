#include "defines.h"

void signal_alarm(int sig) {

    signal(SIGALRM, signal_alarm);
    sig = sig;
    context();

    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "got alarm!");
    return;
}

void signal_shutdown(int sig) {

    signal(SIGINT, signal_shutdown);
    signal(SIGTERM, signal_shutdown);
    sig = sig;
    context();

    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "got signal shutdown (SIGINT/TERM) !");
    g.serverstatus = SERVER_STATUS_EXIT;

    return;
}

void signal_crash(int sig) {

    signal(SIGBUS, SIG_DFL);  //crash
    signal(SIGABRT, SIG_DFL); //crash
    signal(SIGILL, SIG_DFL);  //crash
    signal(SIGFPE, SIG_DFL);  //crash
    signal(SIGSEGV, SIG_DFL); //crash
    sig = sig;

    ioutput(OUTPUT_DEST_LOG | OUTPUT_DEST_NO | OUTPUT_TYPE_INFO, "got signal crash (SIGBUS/ABRT/ILL/FPE/SEGV) !");
#ifdef DEBUG_ENABLED
    ioutput(OUTPUT_DEST_CORE | OUTPUT_TYPE_DEBUG, "-------------------------------------------");
    ioutput(OUTPUT_DEST_LOG | OUTPUT_DEST_NO | OUTPUT_DEST_CORE | OUTPUT_TYPE_DEBUG, "context trace");
    context_dump(OUTPUT_DEST_CORE);
    ioutput(OUTPUT_DEST_CORE | OUTPUT_TYPE_DEBUG, "-------------------------------------------");
    ioutput(OUTPUT_DEST_LOG | OUTPUT_DEST_NO | OUTPUT_DEST_CORE | OUTPUT_TYPE_DEBUG, "memory trace");
    ioutput(OUTPUT_DEST_LOG | OUTPUT_DEST_CORE | OUTPUT_TYPE_DEBUG, "%i memory units allocated", g.meminfo_depth);
    memory_dump(OUTPUT_DEST_CORE);
    ioutput(OUTPUT_DEST_CORE | OUTPUT_TYPE_DEBUG, "-------------------------------------------");    
#endif
    abort();
    /* will crash when we leave the function because returning signal handling back to default */
}

void signal_user1(int sig) {

    signal(SIGUSR1, signal_user1);
    sig = sig;
    context();

    ioutput(OUTPUT_DEST_LOG | OUTPUT_DEST_NO | OUTPUT_TYPE_DEBUG, "changing servers");
    server_write((char *) "QUIT :changing servers", SERVER_SEND_NOW);
    FD_CLR(g.serversocket, &g.readset);
    close(g.serversocket);
    module_serverquit();
    g.serverstatus = SERVER_STATUS_NOTCONNECTED;
    return;
}

#ifdef DEBUG_ENABLED
void signal_user2(int sig) {
int i;

    signal(SIGUSR2, signal_user2);
    sig = sig;
    context();

    ioutput(OUTPUT_DEST_CORE | OUTPUT_TYPE_DEBUG, "-------------------------------------------");
    ioutput(OUTPUT_DEST_LOG | OUTPUT_DEST_NO | OUTPUT_DEST_CORE | OUTPUT_TYPE_DEBUG, "context trace");
    context_dump(OUTPUT_DEST_CORE);
    ioutput(OUTPUT_DEST_CORE | OUTPUT_TYPE_DEBUG, "-------------------------------------------");
    ioutput(OUTPUT_DEST_LOG | OUTPUT_DEST_NO | OUTPUT_DEST_CORE | OUTPUT_TYPE_DEBUG, "memory trace");
    ioutput(OUTPUT_DEST_LOG | OUTPUT_DEST_CORE | OUTPUT_TYPE_DEBUG, "%i memory units allocated", g.meminfo_depth);
    memory_dump(OUTPUT_DEST_CORE);
    ioutput(OUTPUT_DEST_CORE | OUTPUT_TYPE_DEBUG, "-------------------------------------------");    
    ioutput(OUTPUT_DEST_LOG | OUTPUT_DEST_NO | OUTPUT_DEST_CORE | OUTPUT_TYPE_DEBUG, "timer queue dump");
    for (i=0; i<TIMER_QUEUE_SIZE; i++)
	ioutput(OUTPUT_DEST_CORE | OUTPUT_TYPE_DEBUG, "timer %d/%d value (%d)", i, TIMER_QUEUE_SIZE, g.timer_wait_queue[i]);
    ioutput(OUTPUT_DEST_CORE | OUTPUT_TYPE_DEBUG, "-------------------------------------------");    
    module_dumpstatus();
    return;
}
#endif

void signal_init(void) {

    context();
    
    signal(SIGALRM, signal_alarm);
    signal(SIGINT, signal_shutdown);
    signal(SIGTERM, signal_shutdown);
    signal(SIGUSR1, signal_user1);
#if DEBUG_ENABLED
    signal(SIGUSR2, signal_user2);
#endif
    signal(SIGPIPE, SIG_IGN);
    signal(SIGBUS, signal_crash);  //crash
    signal(SIGABRT, signal_crash); //crash
    signal(SIGILL, signal_crash);  //crash
    signal(SIGFPE, signal_crash);  //crash
    signal(SIGSEGV, signal_crash); //crash
    return;
}

void signal_deinit() {

    context();
    
    signal(SIGALRM, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
    signal(SIGBUS, SIG_DFL);  
    signal(SIGABRT, SIG_DFL);
    signal(SIGILL, SIG_DFL); 
    signal(SIGFPE, SIG_DFL); 
    signal(SIGSEGV, SIG_DFL);
    return;
}
