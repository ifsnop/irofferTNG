#include "defines.h"

void config_parsecmd(int argc, char *argv[]) {
int retval;
const char *options = "vc:"; 

    context();
    
    while ( (retval = getopt( argc, argv, options )) != T_ERROR ) {
	switch (retval) {
	    case 'v': {
		    printf("rb %s-%s_%ld\n", VERSION_MAYOR, VERSION_MINOR, VERSION_DATE);
		    exit( EXIT_SUCCESS );
		break;
	    }
	    case 'c': {
		g.conf_file = mycalloc(strlen(optarg) + 1);
		strcpy(g.conf_file, optarg);		    
		break;
	    }
	}
    }    

    return;
}

void config_init(int argc, char *argv[]) {
bool enabled = false;
unsigned int i;
int fd;
long log_level;

    context();
    
    g.conf_file = T_UNDEF;
    g.log_level = OUTPUT_TYPE_DEBUG_EXTRA + OUTPUT_TYPE_DEBUG 
		    + OUTPUT_TYPE_INFO + OUTPUT_TYPE_WARN
		    + OUTPUT_TYPE_SQL + OUTPUT_TYPE_CRITICAL;
    g.servertimereconnect = SERVER_WAIT_RECONNECT;
    g.selfstatus = SELF_STATUS_OK;
    g.send_queue_ptr = NULL;
    g.send_queue_count = 0;
    g.serverstatus = SERVER_STATUS_NOTCONNECTED;
    g.serversocket = T_UNDEF;
    g.linecut = T_UNDEF;
    g.usenatip = T_UNDEF;
    g.mysql_connected = T_UNDEF;
    g.module_list = (struct module_list_t *) T_UNDEF;
    g.mysql_table_irc_users = T_UNDEF;
    g.mysql_user = T_UNDEF;
    g.mysql_pass = T_UNDEF;
    g.mysql_host = T_UNDEF;
    pthread_mutex_init(&g.mutex_db, NULL);
    timeval_reset(&g.startuptime);
    timeval_reset(&g.currenttime);
    timeval_reset(&g.lastservercontact);
    FD_ZERO(&g.readset);
    FD_ZERO(&g.writeset);
#if DEBUG_ENABLED    
    g.meminfo_count = 0;
    g.meminfo_depth = 0;
#endif
    
    if ((gettimeofday(&g.currenttime, (struct timezone *) NULL)) < 0) {
        ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "gettimeofday error: %s", strerror(errno));
	exit(-1);
    }

    timeval_update(&g.startuptime, g.currenttime);

    if (argc != -1) {	//first call
	config_parsecmd(argc, argv);
	if (g.conf_file == T_UNDEF) { //default value
    	    g.conf_file = mycalloc(strlen("rb.conf") + 1);
	    strcpy(g.conf_file, "rb.conf");
	}
    }

    if (setlocale(LC_ALL, "") == NULL) {
	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_WARN, "failed to set locale: %s", strerror(errno));
    }
    ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_INFO, "parsing logfile");
    if (!cfg_open(g.conf_file)) {
	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "please create %s or check for duplicate entry or use the -c switch", g.conf_file);
	exit(EXIT_FAILURE);
    }	
    cfg_get_bool(&enabled, "enabled");
    if (enabled == false) {
	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "please modify the %s file", g.conf_file);
	exit(EXIT_FAILURE);
    }	
    if (!cfg_get_str(&g.log_file, "log_file"))
	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "rb.conf problem: no log_file specified");
    if (!cfg_get_int(&log_level, "log_level"))
	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_DEBUG, "rb.conf problem: no log_level specified. defaulting to ALL");
    else
	g.log_level = log_level;
    
    if (!cfg_get_str(&g.pid_file, "pid_file"))
	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "rb.conf problem: no pid_file specified");
#if DEBUG_ENABLED
    if (!cfg_get_str(&g.core_file, "core_file"))
	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "rb.conf problem: no core_file specified");
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "core_file [%s]", g.core_file);
#endif
    if (!cfg_get_str(&g.user_login, "user_login"))
    	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "rb.conf problem: no user_login specified");
    for (i = 0; i < strlen(g.user_login) && i < TEXTLENGTH_STD; i++)
	g.user_login[i] = tolower(g.user_login[i]);
    if (!cfg_get_str(&g.user_realname, "user_realname"))
    	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "rb.conf problem: no user_realname specified");
    if (!cfg_get_str(&g.user_nick, "user_nick"))
    	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "rb.conf problem: no user_nick specified");
    if (!cfg_get_str(&g.user_modes, "user_modes"))
    	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_WARN, "rb.conf problem: no user_modes specified");
    if (!cfg_get_bool(&g.user_retry_nick, "user_retry_nick"))
	g.user_retry_nick = false;
    if (!cfg_get_str(&g.module_dir, "module_dir"))
    	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "rb.conf problem: no module_dir specified");
    if (!cfg_get_str(&g.mysql_host, "mysql_host"))
    	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "rb.conf problem: no mysql_host specified");
    if (!cfg_get_str(&g.mysql_user, "mysql_user"))
    	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "rb.conf problem: no mysql_user specified");
    if (!cfg_get_str(&g.mysql_pass, "mysql_pass"))
    	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "rb.conf problem: no mysql_pass specified");
    if (!cfg_get_str(&g.mysql_db, "mysql_db"))
    	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "rb.conf problem: no mysql_db specified");
    if (!cfg_get_str(&g.mysql_table_irc_users, "mysql_table_irc_users"))
    	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "rb.conf problem: no mysql_table_irc_users specified");
    if (!cfg_get_str(&g.irc_host, "irc_host"))
    	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "rb.conf problem: no irc_host specified");
    if (!cfg_get_str(&g.irc_port, "irc_port"))
	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "rb.conf problem: no irc_port specified");
    if (!cfg_get_str_array(&g.irc_channel, &g.irc_channel_count, "irc_channel"))
	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_WARN, "rb.conf problem: no irc_channel specified");
    g.irc_channel_joined = (char *) mycalloc (g.irc_channel_count);
    for (i=0; i < g.irc_channel_count; i++)
	g.irc_channel_joined[i] = 'N';
    if (!cfg_get_str(&g.pserve_version, "pserve_version")) {
	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_WARN, "rb.conf problem: no pserve_version specified");
	g.pserve_version = mycalloc(9);
	strcpy(g.pserve_version, "not_cnfg");
    }
    if (!cfg_get_str(&g.pserve_trigger, "pserve_trigger")) {
	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_WARN, "rb.conf problem: no pserve_trigger specified");	
    	g.pserve_trigger = mycalloc(9);
	strcpy(g.pserve_trigger, "not_cnfg");
    }
    if (!cfg_get_str(&g.pserve_speed, "pserve_speed")) {
	ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_WARN, "rb.conf problem: no pserve_speed specified");	
    	g.pserve_speed = mycalloc(9);
	strcpy(g.pserve_speed, "not_cnfg");
    }

    if ((gettimeofday(&g.currenttime, (struct timezone *) NULL)) < 0) {
        ioutput(OUTPUT_DEST_NO | OUTPUT_TYPE_CRITICAL, "gettimeofday error: %s", strerror(errno));
	exit(-1);
    }

    timer_wait_reset();
    cfg_close();

    if ( (fd = open(g.pid_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) == T_ERROR ) {
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "pid_file open error: %s", strerror(errno));
    } else {
	char *tmpbuff;
	tmpbuff = mycalloc(10);
	sprintf(tmpbuff, "%d\n", getpid());
	if ( write(fd, tmpbuff, strlen(tmpbuff)) != ((int) strlen(tmpbuff)) ) {
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "pidfile write error: %s", strerror(errno));
	}
	mydelete(tmpbuff);
	if ( close(fd) == T_ERROR ) {
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "pidfile close error: %s", strerror(errno));
	}
    }

    g.server_input_line = mycalloc(TEXTLENGTH_MAX*4);
    ioutput(OUTPUT_DEST_LOG | OUTPUT_DEST_NO | OUTPUT_TYPE_INFO, "config closing");

    config_show();
    return;
}

void config_deinit(void) {
unsigned int i;

    context();

    ioutput(OUTPUT_DEST_LOG | OUTPUT_DEST_NO | OUTPUT_TYPE_INFO, "clearing configuration");
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "------------------END----------------------");
    
    free(g.log_file); 		g.log_file = NULL;
    free(g.pid_file); 		g.pid_file = NULL;
#if DEBUG_ENABLED
    free(g.core_file);		g.core_file = NULL;
#endif
    free(g.user_login); 	g.user_login = NULL;
    free(g.user_realname); 	g.user_realname = NULL;
    free(g.user_nick); 		g.user_nick = NULL;
    free(g.user_modes); 	g.user_modes = NULL;
    free(g.module_dir);		g.module_dir = NULL;
    free(g.mysql_host);		g.mysql_host = NULL;
    free(g.mysql_user);		g.mysql_user = NULL;
    free(g.mysql_pass);		g.mysql_pass = NULL;
    free(g.mysql_db);		g.mysql_db = NULL;
    free(g.mysql_table_irc_users); g.mysql_table_irc_users = NULL;
    free(g.irc_host);		g.irc_host = NULL;
    free(g.irc_port);		g.irc_port = NULL;
    mydelete(g.conf_file);
    mydelete(g.server_input_line);

    for (i=0; i < g.irc_channel_count; i++)
	free(g.irc_channel[i]);
    free(g.irc_channel);	g.irc_channel = NULL;
    mydelete(g.irc_channel_joined);

    if (!strcmp(g.pserve_version, "not_cnfg")) {
	mydelete(g.pserve_version);
    } else {
	free(g.pserve_version);	g.pserve_version = NULL;
    }
    if (!strcmp(g.pserve_trigger, "not_cnfg")) {
	mydelete(g.pserve_trigger);
    } else {
	free(g.pserve_trigger);	g.pserve_trigger = NULL;
    }
    if (!strcmp(g.pserve_speed, "not_cnfg")) {
	mydelete(g.pserve_speed);
    } else {
	free(g.pserve_speed);	g.pserve_speed = NULL;
    }

    pthread_mutex_destroy(&g.mutex_db);

    return;
}

void config_show(void) {
unsigned int i;

    context();

    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "-----------------START---------------------");
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "conf_file [%s]", g.conf_file);
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "log_file [%s]", g.log_file);
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "log_level [%d]", g.log_level);
#if DEBUG_ENABLED
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "core_file [%s]", g.core_file);
#endif
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "user_nick [%s]", g.user_nick);	
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "user_realname [%s]", g.user_realname);	
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "pid_file [%s]", g.pid_file);
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "user_login [%s]", g.user_login);	
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "user_modes [%s]", g.user_modes);
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "user_retry_nick [%d]", g.user_retry_nick);
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "module_dir [%s]", g.module_dir);	
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "mysql_host [%s]", g.mysql_host);	
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "mysql_user [%s]", g.mysql_user);	
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "mysql_pass [%s]", g.mysql_pass);	
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "mysql_db [%s]", g.mysql_db);	
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "mysql_table_irc_users [%s]", g.mysql_table_irc_users);
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "irc_host [%s]", g.irc_host);	
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "irc_port [%s]", g.irc_port);	
    for (i=0; i < g.irc_channel_count; i++)
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "irc_channel %d/%d:[%s] %c", i + 1, g.irc_channel_count, g.irc_channel[i], g.irc_channel_joined[i]);
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "-----------------MODULES-------------------");
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "pserve_version [%s]", g.pserve_version);
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "pserve_trigger [%s]", g.pserve_trigger);
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "pserve_speed [%s]", g.pserve_speed);
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "-------------------------------------------");

    return;
}
