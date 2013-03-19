#define _MAIN_
#include "defines.h"

int main(int argc, char *argv[]) {

    long oldsec = 0;
    struct timeval timestruct;
    int retval;
    
    context();

    config_init(argc, argv);
    if (g.selfstatus == SELF_STATUS_OK)
	signal_init();
    if (g.selfstatus == SELF_STATUS_OK)
	module_init();

    if (g.selfstatus == SELF_STATUS_OK)
	ioutput(OUTPUT_DEST_LOG | OUTPUT_DEST_NO | OUTPUT_TYPE_INFO, "rb %s-%s_%ld started", VERSION_MAYOR, VERSION_MINOR, VERSION_DATE);
	 
//    daemon(1,0);
    timestruct.tv_sec = SERVER_LATENCY_SEC; timestruct.tv_usec = SERVER_LATENCY_USEC;
    oldsec = g.currenttime.tv_sec;

    while ((g.serverstatus != SERVER_STATUS_EXIT) && (g.selfstatus == SELF_STATUS_OK )) {

	context();
	
	FD_ZERO(&g.readset);
	FD_ZERO(&g.writeset);

	if ((gettimeofday(&g.currenttime, (struct timezone *) NULL)) < 0)
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "gettimeofday error: %s", strerror(errno));

	if (oldsec != g.currenttime.tv_sec) {
	    timer_wait_update(g.currenttime.tv_sec - oldsec);
//	    ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_DEBUG, "sec elapsed: %ld [%s]", g.currenttime.tv_sec - oldsec, g.user_nick);
	    oldsec = g.currenttime.tv_sec;
	}

	if ((g.serverstatus == SERVER_STATUS_CONNECTED_1) || (g.serverstatus == SERVER_STATUS_CONNECTED_2))
	    FD_SET(g.serversocket, &g.readset);

	if (g.serverstatus == SERVER_STATUS_TRYING || (g.send_queue_count > 0))
	    FD_SET(g.serversocket, &g.writeset);

	if (g.serverstatus == SERVER_STATUS_NOTCONNECTED) {
	    if (!timer_wait(TIMER_SERVER_WAIT_RECONNECT, g.servertimereconnect))
    		if (server_connect() == T_OK) 
		    g.serverstatus = SERVER_STATUS_TRYING;
	}

	if (g.serverstatus != SERVER_STATUS_NOTCONNECTED) {
	    retval = select(g.serversocket+1, &g.readset, &g.writeset, NULL, &timestruct);
	    if (retval < 0) {
		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "select error: %s", strerror(errno));
		timestruct.tv_sec = SERVER_LATENCY_SEC; timestruct.tv_usec = SERVER_LATENCY_USEC;
		FD_ZERO(&g.readset);
		FD_ZERO(&g.writeset); 
	    }
	}

	if ((timestruct.tv_sec == 0) && (timestruct.tv_usec < 10000)) 
	    timestruct.tv_sec = SERVER_LATENCY_SEC; timestruct.tv_usec = SERVER_LATENCY_USEC;

	server_read();

	if (!timer_wait(TIMER_SERVER_EVERY1SEC, 1)) {
	    server_send();
	    module_every1sec();
	}
	if (!timer_wait(TIMER_SERVER_EVERY20SEC, 20))
	    module_every20sec();
	if (g.serverstatus == SERVER_STATUS_NOTCONNECTED)
	    usleep(SERVER_LATENCY_USEC);
    }

    module_deinit();
    signal_deinit();

    ioutput(OUTPUT_DEST_NO | OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "rb ended");

    config_deinit();

    exit( (g.selfstatus == SELF_STATUS_OK) ? EXIT_SUCCESS : EXIT_FAILURE );
}
