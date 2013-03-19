#include "defines.h"

int server_connect(void) {
struct addrinfo hints, *remotehost, *remotehost0;
struct sockaddr_in localaddr;
int retval, addrlen;

    context();

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    if ( (retval = getaddrinfo(g.irc_host, g.irc_port, &hints, &remotehost0)) != T_OK ) {
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "getaddrinfo error: %s", gai_strerror(retval));
	return T_ERROR;
    }

    for ( remotehost = remotehost0; remotehost; remotehost = remotehost->ai_next) {
	if ( (g.serversocket = socket(remotehost->ai_family, remotehost->ai_socktype, remotehost->ai_protocol)) == T_ERROR ) {
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "socket error: %s", strerror(errno));
	    continue; //siguiente de la lista
	}

	if (fcntl(g.serversocket, F_SETFL, O_NONBLOCK) < 0) {
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "couldn't set nonblocking error: %s", strerror(errno));
	}

        alarm(CONFIG_CONNECT_TIMEOUT);
	retval = connect(g.serversocket, remotehost->ai_addr, remotehost->ai_addrlen);
	if ( (retval < 0) && !((errno == EINPROGRESS) || (errno == EAGAIN)) ) {
    	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "connect error: %s", strerror(errno));
	    alarm(0);
	    close(g.serversocket);
	    return T_ERROR;
	}
	alarm(0);

	bzero(g.ourip_text, sizeof(g.ourip_text));
	addrlen = sizeof(localaddr);
	bzero (&localaddr, addrlen);
	g.ourip = 0;

	if (!g.usenatip) {

	    // 1st step: try to get our *text* ip address
    	    if (getnameinfo(remotehost->ai_addr, remotehost->ai_addrlen, g.ourip_text, sizeof(g.ourip_text), NULL,
		0, NI_NUMERICHOST) == T_OK) {
		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "ourip (1) = %s", g.ourip_text);
	    } else {
    		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "couldn't get ourip: %s", strerror(errno));
		close(g.serversocket);
    		return T_ERROR;
	    }
	    
	    // 2nd step: try to get our *numeric* ip address (should be the same as 1)
    	    if (getsockname(g.serversocket, (struct sockaddr *) &localaddr, &addrlen) < 0) {
    		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "couldn't get ourip: %s", strerror(errno));
		close(g.serversocket);
    		return T_ERROR;
	    } else {
		g.ourip = ntohl(localaddr.sin_addr.s_addr);
		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "ourip (2) = %d.%d.%d.%d", 
		    (g.ourip >> 24) & 0xFF, (g.ourip >> 16) & 0xFF, 
		    (g.ourip >> 8) & 0xFF, (g.ourip) & 0xFF);
	    }

	} else {
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "behind a proxy/firewall, couldn't get our real internet address");
    	}
	break;
    }

    freeaddrinfo(remotehost0);
    timeval_update(&g.lastservercontact, g.currenttime);
    return T_OK;

}

void server_read(void) {
char tempstr[TEXTLENGTH_STD];
char tempbuff[TEXTLENGTH_MAX*4];

    context();

    bzero(tempstr, TEXTLENGTH_STD);

    if ((g.serverstatus == SERVER_STATUS_TRYING) && FD_ISSET(g.serversocket, &g.writeset)) {
        if (write(g.serversocket, "\n", 1) < 0) {
            ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "server connection failed: %s", strerror(errno));
            FD_CLR(g.serversocket, &g.writeset);
            close(g.serversocket);
	    module_serverquit();
            g.serverstatus = SERVER_STATUS_NOTCONNECTED;
        } else {
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "server connection established, logging in");
            FD_CLR(g.serversocket, &g.writeset);
	    g.serverstatus = SERVER_STATUS_CONNECTED_1;
	    snprintf(tempstr, TEXTLENGTH_STD - 1, "USER %s * * :%s", g.user_login, g.user_realname);
	    server_write(tempstr, SERVER_SEND_NOW);
	    snprintf(tempstr, TEXTLENGTH_STD - 1, "NICK %s", g.user_nick);
	    server_write(tempstr, SERVER_SEND_NOW);
	}
    }
    
    if ( ((g.serverstatus == SERVER_STATUS_CONNECTED_1) || (g.serverstatus == SERVER_STATUS_CONNECTED_2)) 
	&& FD_ISSET(g.serversocket, &g.readset)) {
	int length=0, i=0, j=0;
	
	timeval_update(&g.lastservercontact, g.currenttime);
	bzero(tempbuff, TEXTLENGTH_MAX*4);
	length = read(g.serversocket, &tempbuff, TEXTLENGTH_MAX*4);

        if (length < 1) {
            ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "closing server connection: connection lost.");
            FD_CLR(g.serversocket, &g.readset);
            close(g.serversocket);
	    module_serverquit();
            g.serverstatus = SERVER_STATUS_NOTCONNECTED;
        } else {
            i = 0;
            j = g.linecut;
            while ((i < TEXTLENGTH_MAX*4) && (tempbuff[i] != '\0')) {
                while ((i < TEXTLENGTH_MAX*4) && (tempbuff[i] != '\n') && (tempbuff[i] != '\0')) {
                    g.server_input_line[j] = tempbuff[i];
                    i++;
                    j++;
                }
        	g.server_input_line[j] = '\0';
    		if (tempbuff[i] != '\n')
            	    g.linecut = j;
    		else {
        	    if (g.server_input_line[strlen(g.server_input_line) - 1] == 0x0D)
	        	/* chop ^M off end of line if there */
        		g.server_input_line[strlen(g.server_input_line) - 1] = '\0';
		    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG_EXTRA, ">RCV>: %s", g.server_input_line);
		    server_parseline(stripnonprintable(g.server_input_line));
            	    g.linecut = 0;
		}
		j = 0;
		i++;
	    }
	}
    }

    return;																												
}

void server_send(void) {
// use server_write to send queued stuff to the server
    struct send_queue_t *p = g.send_queue_ptr;
    struct send_queue_t *ptmp = g.send_queue_ptr;
    
    context();
    if (g.send_queue_count > 0) {
	server_write(p->line, SERVER_SEND_NOW);
	mydelete(p->line);
	ptmp = p->next;
	mydelete(p);
	g.send_queue_ptr = ptmp;
	g.send_queue_count--;
    }
    return;
}

void server_write(char *msg, int sendtype) {
char tmp[TEXTLENGTH_MAX];
int length=0;

    context();
    if ( (g.serverstatus == SERVER_STATUS_CONNECTED_1) || (g.serverstatus == SERVER_STATUS_CONNECTED_2)) {
	if (sendtype == SERVER_SEND_NOW) {
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "<SND<: %s", msg);
	    snprintf(tmp, TEXTLENGTH_MAX - 1, "%s\n", msg);
	    if ( (length = write(g.serversocket, tmp, strlen(tmp))) == T_ERROR ) {
		FD_CLR(g.serversocket, &g.readset);
		close(g.serversocket);
		module_serverquit();
		g.serverstatus = SERVER_STATUS_NOTCONNECTED;
		return;
	    }
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG_EXTRA, "<CNT<: %d %d", strlen(msg), length);
	    timeval_update(&g.lastservercontact, g.currenttime);
	} else { //SERVER_SEND_NORMAL
	    struct send_queue_t *p = g.send_queue_ptr;
	    if (g.send_queue_count < SERVER_QUEUE_MAXSIZE) {
    		g.send_queue_count++;
		ioutput(OUTPUT_DEST_LOG |OUTPUT_TYPE_DEBUG, "<QUE<: (%d) %s", g.send_queue_count, msg);
		if (!p) {
		    g.send_queue_ptr = p = mycalloc(sizeof(struct send_queue_t));
		} else {
		    while (p->next) {
			p=p->next;
		    }
		    p->next = mycalloc(sizeof(struct send_queue_t));
		    p = p->next;
		}
		p->line = mycalloc(strlen(msg)+1);		    
		strncpy(p->line, msg, strlen(msg)+1);
		p->next = NULL;
	    } else {		
		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "server queue is full");
	    }
	}
    }
    return;
}

void server_parseline(char *line) {
char *tempstr = mycalloc(TEXTLENGTH_MAX);
char *tempstr2 = mycalloc(TEXTLENGTH_MAX);
char *part2, *part3, *part4, *part5;

    context();

    if (!line)
	return;

    line[TEXTLENGTH_MAX - 1] = '\0';

    part2 = getpart(line, 2);
    part3 = getpart(line, 3);
    part4 = getpart(line, 4);
    part5 = getpart(line, 5);

    if (part2 == NULL) {
	mydelete(tempstr);
	mydelete(tempstr2);
	mydelete(part2);
	mydelete(part3);
	mydelete(part4);
	mydelete(part5);
	return;
    }

    /* NOTICE nick */
/*    if (part3 && !strcasecmp(part2, "NOTICE")
        && !strcasecmp(part3, g.user_nick))*/
    /* privmsgparse("NOTICE", line); */ // send to modules

    if ( (part4) && (!strcasecmp(part2, "PRIVMSG")) )/* && !strcasecmp(part3, g.user_nick) )*/
	privmsg_parse(line);
		    
    /* :server 451  xxxx :Register first. */
    if (!strcmp(part2, "451")) {
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "register first: %s", line);
	snprintf(tempstr, TEXTLENGTH_MAX - 2, "USER %s * * :%s", g.user_login, g.user_realname);
	server_write(tempstr, SERVER_SEND_NOW);
	snprintf(tempstr, TEXTLENGTH_MAX - 2, "NICK %s", g.user_nick);
        server_write(tempstr, SERVER_SEND_NOW);
    }

    /* 433 * xxxxx :Nickname is already in use or registered (missing or wrong password) */
    if (!strcmp(part2, "433")) {
	char nickbuff[TEXTLENGTH_STD];
	int size_newnick;
	
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "nick already in use: %s", line);

        snprintf(tempstr, TEXTLENGTH_MAX - 2, "USER %s * * :%s", g.user_login, g.user_realname);
        server_write(tempstr, SERVER_SEND_NOW);
	if (!g.user_retry_nick) { // construye un nuevo nick
	    bzero(nickbuff, TEXTLENGTH_STD);
	    if (strlen(g.user_nick) > 5)
		g.user_nick[6] = '\0';
	    snprintf(nickbuff, TEXTLENGTH_STD - 1, "%s%03ld", g.user_nick, g.currenttime.tv_sec & 255);
	    size_newnick = strlen(nickbuff) + 1;
	    g.user_nick = (char *) realloc(g.user_nick, size_newnick);
	    bzero(g.user_nick, size_newnick);
	    strncpy(g.user_nick, nickbuff, size_newnick);
	}
	snprintf(tempstr, TEXTLENGTH_MAX - 2, "NICK %s", g.user_nick);
    	server_write(tempstr, SERVER_SEND_NOW);
    }

    /* ERROR :Closing Link */
    if (strncmp(line, "ERROR :Closing Link", 19) == 0) {
	if (g.serverstatus != SERVER_STATUS_EXIT) {
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "server closed connection: %s", line);
            FD_CLR(g.serversocket, &g.readset);
	    close(g.serversocket);
	    module_serverquit();
            g.serverstatus = SERVER_STATUS_NOTCONNECTED;
	    if ( strstr(line, "hrottle") && strstr(line, "connect too fast") ) {
		g.servertimereconnect *= 2;
		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "throttling connection, waiting %d secs", g.servertimereconnect);
	    }
        }
    }
								    
    /* server ping */
    if (!strncmp(line, "PING :", 6)) {
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "server ping: %s", line);
	snprintf(tempstr, TEXTLENGTH_MAX - 2, "%s", line);
	tempstr[1] = 'O'; // PONG
	server_write(tempstr, SERVER_SEND_NOW);
	/* ping for austnet verification */
	/* snprintf(tempstr,maxtextlength-2,"MODE %s %s",gdata.user_nick,gdata.user_modes); */
	/* writeserver2(tempstr,1); */
	if (g.user_modes && strlen(g.user_modes) && (g.serverstatus == SERVER_STATUS_CONNECTED_1)) {
	    // on the first ping, send mode info
	    g.serverstatus = SERVER_STATUS_CONNECTED_2;
    	    snprintf(tempstr, TEXTLENGTH_MAX - 2, "MODE %s %s", g.user_nick, g.user_modes);
    	    server_write(tempstr, SERVER_SEND_NORMAL);
    	    g.servertimereconnect = SERVER_WAIT_RECONNECT; // successful connection! restart counters
	}
    }

    module_ircinput(line, part2, part3, part4, part5);

    mydelete(tempstr);
    mydelete(tempstr2);
    mydelete(part2);
    mydelete(part3);
    mydelete(part4);
    mydelete(part5);
					       									     
    return;
}

void server_notice(const char *nick, const char *format, ...) {
char tempstr[TEXTLENGTH_MAX];
char tempstr2[TEXTLENGTH_MAX];
va_list args;

    bzero(tempstr, TEXTLENGTH_MAX);
    bzero(tempstr2, TEXTLENGTH_MAX);

    va_start(args, format);
    vsnprintf(tempstr, TEXTLENGTH_MAX - 2, format, args);
    va_end(args);

    snprintf(tempstr2, TEXTLENGTH_MAX - 2, "NOTICE %s :%s", nick, tempstr);
    server_write(tempstr2, SERVER_SEND_NORMAL);

    return;
}

void server_ctcp(const char *nick, const char *format, ...) {
char tempstr[TEXTLENGTH_MAX];
char tempstr2[TEXTLENGTH_MAX];
va_list args;

    bzero(tempstr, TEXTLENGTH_MAX);
    bzero(tempstr2, TEXTLENGTH_MAX);

    va_start(args, format);
    vsnprintf(tempstr, TEXTLENGTH_MAX - 2, format, args);
    va_end(args);

    snprintf(tempstr2, TEXTLENGTH_MAX - 2, "PRIVMSG %s :%c%s%c", nick, 001, tempstr, 001);
    server_write(tempstr2, SERVER_SEND_NORMAL);

    return;
}

void privmsg_parse(char *line) {
    char *nick, *hostname, *hostmask, *msg1, *msg2, *msg3, *msg4, *msg5, *msg6, *dest;
    unsigned int i, j;
    
    hostmask = getpart(line, 1);
    for (i = 1; i <= strlen(hostmask); i++)
	hostmask[i - 1] = hostmask[i];
	      
    dest = getpart(line, 3);
    msg1 = getpart(line, 4);
    msg2 = getpart(line, 5);
    msg3 = getpart(line, 6);
    msg4 = getpart(line, 7);
    msg5 = getpart(line, 8);
    msg6 = getpart(line, 9);
    
    if (msg1)
	msg1++;
	
    nick = mycalloc(TEXTLENGTH_STD);
    hostname = mycalloc(TEXTLENGTH_MAX);
    
    i = 1;
    j = 0;
    while ( (line[i] != '!') && (i < strlen(line)) && (i < TEXTLENGTH_STD - 1) ) {
	nick[i - 1] = line[i];
	i++;
    }
    nick[i - 1] = '\0';
    
    /* see if it came from a user or server, ignore if from server */
    if ( (i == strlen(line)) || (i == (TEXTLENGTH_STD - 1)) )
	goto abort;

    while ( (line[i] != '@') && (i < strlen(line)) )
	i++;
    i++;
		
    while ( (line[i] != ' ') && (i < strlen(line)) && (j < TEXTLENGTH_MAX - 1) ) {
        hostname[j] = line[i];
        i++;
        j++;
    }
    hostname[j] = '\0';

    module_privmsg(line, nick, hostname, hostmask, dest, msg1, msg2, msg3, msg4, msg5);
    
    abort:
    msg1--;
    mydelete(dest);
    mydelete(nick);
    mydelete(hostname);
    mydelete(hostmask);
    mydelete(msg1);
    mydelete(msg2);
    mydelete(msg3);
    mydelete(msg4);
    mydelete(msg5);
    mydelete(msg6);
		
    return;	
}
