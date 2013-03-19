               :#define _MAIN_
               :#include "defines.h"
               :
               :int main(int argc, char *argv[]) { /* main total:  15071 17.2747 */
               :
               :    long oldsec = 0;
               :    struct timeval timestruct;
               :    int retval;
               :
               :    context();
               :
               :    config_init();
               :    if (g.selfstatus == SELF_STATUS_OK)
               :	signal_init();
               :    if (g.selfstatus == SELF_STATUS_OK)
               :	module_init();
               :
               :    if (g.selfstatus == SELF_STATUS_OK)
               :	ioutput(OUTPUT_DEST_LOG | OUTPUT_DEST_NO | OUTPUT_TYPE_INFO, "rb started");
               :
               ://    daemon(1,0);
               :    timestruct.tv_sec = SERVER_LATENCY_SEC; timestruct.tv_usec = SERVER_LATENCY_MLS;
               :    oldsec = g.currenttime.tv_sec;
               :    
   297  0.3404 :    while ((g.serverstatus != SERVER_STATUS_EXIT) && (g.selfstatus == SELF_STATUS_OK )) {
               :
   587  0.6728 :	context();
               :	
  4589  5.2600 :	FD_ZERO(&g.readset);
  4503  5.1614 :	FD_ZERO(&g.writeset);
               :
   461  0.5284 :	if ((gettimeofday(&g.currenttime, (struct timezone *) NULL)) < 0)
               :	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "gettimeofday error: %s", strerror(errno));
               :
   456  0.5227 :	if (oldsec != g.currenttime.tv_sec) {
               :	    timer_wait_update(g.currenttime.tv_sec - oldsec);
               ://	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "sec elapsed: %ld [%s]", g.currenttime.tv_sec - oldsec, g.user_nick);
               :	    oldsec = g.currenttime.tv_sec;
               :	}
               :
   493  0.5651 :	if (g.serverstatus == SERVER_STATUS_CONNECTED)
               :	    FD_SET(g.serversocket, &g.readset);
               :
  1007  1.1542 :	if (g.serverstatus == SERVER_STATUS_TRYING || (g.send_queue_count > 0))
               :	    FD_SET(g.serversocket, &g.writeset);
               :
   384  0.4401 :	if (g.serverstatus == SERVER_STATUS_NOTCONNECTED) {
   343  0.3932 :	    if (!timer_wait(TIMER_SERVER_WAIT_RECONNECT, SERVER_WAIT_RECONNECT))
               :    		if (server_connect() == T_OK) 
               :		    g.serverstatus = SERVER_STATUS_TRYING;
               :	}
               :
   132  0.1513 :	if (g.serverstatus != SERVER_STATUS_NOTCONNECTED) {
     1  0.0011 :	    retval = select(g.serversocket+1, &g.readset, &g.writeset, NULL, &timestruct);
               :	    if (retval < 0) {
               :		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "select error: %s", strerror(errno));
               :		timestruct.tv_sec = SERVER_LATENCY_SEC; timestruct.tv_usec = SERVER_LATENCY_MLS;
               :	    }
               :	}
               :
   480  0.5502 :	if ((timestruct.tv_sec == 0) && (timestruct.tv_usec == 0)) 
   536  0.6144 :	    timestruct.tv_sec = SERVER_LATENCY_SEC; timestruct.tv_usec = SERVER_LATENCY_MLS;
               :
     2  0.0023 :	server_read();
               :
   288  0.3301 :	if (!timer_wait(TIMER_SERVER_EVERY1SEC, 1)) {
               :	    server_send();	
               :	}
   512  0.5869 :	if (!timer_wait(TIMER_SERVER_EVERY20SEC, 20)) {
               :	    module_every20sec();	
               :	}
               :    }
               :
               :    module_deinit();
               :    signal_deinit();
               :
               :    ioutput(OUTPUT_DEST_NO | OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "rb ended");
               :
               :    config_deinit();
               :
               :    exit( (g.selfstatus == SELF_STATUS_OK) ? EXIT_SUCCESS : EXIT_FAILURE );
               :}
/* 
 * Total samples for file : "src/main.c"
 * 
 *  15071 17.2747
 */


/* 
 * Command line: opannotate --source --output-dir=annotated/ ./rb 
 * 
 * Interpretation of command line:
 * Output annotated source file with samples
 * Output all files
 * 
 * CPU: Athlon, speed 1333 MHz (estimated)
 * Counted CPU_CLK_UNHALTED events (Cycles outside of halt state) with a unit mask of 0x00 (No unit mask) count 100000
 */
