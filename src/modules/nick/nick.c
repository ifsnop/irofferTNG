#define _MOD_
#include "../modules.h"

Function * f = NULL;

int mod_load(char *name, Function *f2, struct global_t * g2) {

    f = f2;
    context();
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "module %s loading", name);
    return (MODULE_HOOK_IRCINPUT);
}

int mod_unload(char *name, struct global_t * g2) {

    context();
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "module %s unloading", name);
    return (T_OK);
}

int mod_ircinput(char *fullline, char *part2, char *part3, char *part4, char *part5, struct global_t * g2) {
unsigned int j = 1;

    context();

    if (!strcasecmp(part2, "NICK") && part3) {
	char *newnick, *oldnick;;
	
	oldnick = (char *) mycalloc(TEXTLENGTH_STD);
	
	while (fullline[j] != '!' && j < strlen(fullline) && (j < (TEXTLENGTH_STD - 1))) {
	    oldnick[j - 1] = fullline[j];
	    j++;
	}
	oldnick[j - 1] = '\0';
	
	newnick = part3;
	if (newnick[0] == ':')
	    newnick++;
	
	if (!strcasecmp(oldnick, g2->user_nick)) {
	    free(g2->user_nick);
	    g2->user_nick = (char *)calloc(1,TEXTLENGTH_STD); //allocated by libconfig
	    strncpy(g2->user_nick, newnick, TEXTLENGTH_STD);
	} else { /* someone else changed nicks */
	    if (db_write("UPDATE %s.%s SET user='%s' WHERE user='%s'", g2->mysql_db, g2->mysql_table_irc_users, newnick, oldnick))
		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "nick.so: user missed %s -> %s", oldnick, newnick);
	}
	mydelete(oldnick);
	return (T_OK);
    } else if (!strcasecmp(part2, "MODE") && part3 && part4 && part5) { //mode change
	// WARNING: mode change (:ifsnop!futuro@217.127.201.198 MODE #ANTHALIA.COM +oo Heimy ifsnopALS ) (MODE) (#ANTHALIA.COM) (+oo)
	int channelid = -1;
	
	context();
	
        for (j = 0; j < g2->irc_channel_count; j++) {
	    if (!strcasecmp(g2->irc_channel[j], part3)) {
	        channelid = j;
	        break;
	    }
	}
	if (channelid > -1) {
	    if (strlen(part4) >= 2) {
		char *ptr;
		int k = 1, umode = USER_MODE_HAS_0;
		struct db_row_t row;
		while (part4[k] != '\0') {
		    ptr = getpart(fullline, k + 4);
		    if (part4[k] == 'l' || part4[k] == 'b')
			goto next;
		    if (part4[k] == 'o')
		        umode = USER_MODE_HAS_OP;
		    if (part4[k] == 'v')
		        umode = USER_MODE_HAS_V;
		    row = db_read("SELECT mode FROM %s.%s WHERE channelid = '%d' AND user = '%s'", g2->mysql_db, g2->mysql_table_irc_users, channelid, ptr);
		    if ( row.num_fields >= 1 ) {
			if (part4[0] == '+') {
			    umode |= atoi(row.r[0]);
			} else { 
			    if (part4[0] == '-') 
			        umode ^= atoi(row.r[0]);
			}
			if (db_write("UPDATE %s.%s SET mode = '%d' WHERE channelid = '%d' AND user = '%s'", g2->mysql_db, g2->mysql_table_irc_users, umode, channelid, ptr))
			    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "nick.so: problem changing user mode %s %c", ptr, umode);
		    } else {
			ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "nick.so: problem finding user %s", ptr);
		    }
		    db_row_clean(row);
next:
		    mydelete(ptr);
		    k++;
		}
	    }
	} else
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "nick.so: problem changing user mode, channel not found %s", part5);
	return (T_OK);
    } else if (!strcmp(part2, "PART") && part3) {
	char *nick;
	int channelid = -1;
	
	context();
	nick = (char *) mycalloc(TEXTLENGTH_STD);
	
	while ( (fullline[j] != '!') && (j < strlen(fullline)) && (j < (TEXTLENGTH_STD -1)) ) {
	    nick[j - 1] = fullline[j];
	    j++;
	}
	nick[j - 1] = '\0';
	if ( !strcasecmp(nick, g2->user_nick) ) {
	    // we left, funny, why?
	} else {
	    char *t3;
	    t3 = part3;
	    if (t3[0] == ':')
		t3++;
	    for (j = 0; j < g2->irc_channel_count; j++) {
		if (!strcasecmp(g2->irc_channel[j], t3)) {
		    channelid = j;
	    	    break;
		}
	    }
	    if (channelid > -1) {
		if ( db_write("DELETE FROM %s.%s WHERE channelid = '%d' AND user = '%s'", g2->mysql_db, g2->mysql_table_irc_users, channelid, nick) )
		    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "nick.so: error deleting (%s) from (%d)(%s)", nick, channelid, t3);
	    } else 
		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "nick.so: someone (%s) leave a not known channel (%s)", nick, t3);
	}
	mydelete(nick);
	return (T_OK);
    } else if (!strcasecmp(part2, "JOIN") && part3) {
	int channelid = -1;
	char *nick;
	
	context();
	nick = (char *) mycalloc(TEXTLENGTH_STD);
		
	while ( (fullline[j] != '!') && (j < strlen(fullline)) && (j < (TEXTLENGTH_STD - 1)) ) {
	    nick[j - 1] = fullline[j];
	    j++;
	}
	nick[j - 1] = '\0';

	if (part3[0] == ':') {
    	    for (j = 1; j <= strlen(part3); j++) 
		part3[j - 1] = part3[j];
	    part3[j] = '\0';	// lets ensure that part3 aka channel is #channelname, and not :#channel or whatever.
	}
	
	if ( !strcasecmp(nick, g2->user_nick) ) {
	    // we joined
	    for (j = 0; j < g2->irc_channel_count; j++) {
		if (!strcasecmp(g2->irc_channel[j], part3)) {
	    	    g2->irc_channel_joined[j] = 'Y';
		    channelid = j;
	    	    break;
		}
	    }
	    if (channelid == -1)
	    	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "nick.so: joined (%s) which is not a known channel", part3);
/*	    else {
	        if ( db_write("DELETE FROM irc_users WHERE channelid = '%d'", channelid) )
	    	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "nick.so: can't delete userlist from (%d)(%s)", channelid, part3);
	    }*/ //redundante, al hacer un join ya se ha hecho un delete previo
	    
	} else { //someone else joined
	    for (j = 0; j < g2->irc_channel_count; j++) {
		if (!strcasecmp(g2->irc_channel[j], part3)) {
		    channelid = j;
		    break;
		}	    
	    }
	    if (channelid > -1) {
		if (db_write("INSERT INTO %s.%s (channelid, user, mode) VALUES ('%d', '%s', '%d')", g2->mysql_db, g2->mysql_table_irc_users, channelid, nick, 0)) {
		    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "nick.so: error adding (%s) to (%s) ", nick, part3);
		}
	    } else {
		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "nick.so: someone (%s) joined (%s) which is not a known channel", nick, part3);
	    }
	}
	mydelete(nick);
	return (T_OK);
    } else if (!strcmp(part2, "QUIT")) {
	char *nick;
	
	context();
	
	nick = (char *) mycalloc(TEXTLENGTH_STD);
	while ( (fullline[j] != '!') && ( j < strlen(fullline)) && (j < (TEXTLENGTH_STD - 1)) ) {
	    nick[j - 1] = fullline[j];
	    j++;
	}
	nick[j - 1] = '\0';
	if (!strcasecmp(nick, g2->user_nick)) {
	    // we leave? funny
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "nick.so: we leave");
	} else {
	    if ( db_write("DELETE FROM %s.%s WHERE user = '%s'", g2->mysql_db, g2->mysql_table_irc_users, nick) )
		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "nick.so: error deleting (%s)", nick);
	}
	mydelete(nick);
    } else if (!strcmp(part2, "KICK") && part3 && part4) {
	int channelid = -1;
    
	context();

	for (j = 0; j < g2->irc_channel_count; j++) {
	    if (!strcasecmp(g2->irc_channel[j], part3)) {
	        channelid = j;
	        break;
	    }	    
	}
	if (channelid > -1) {
	    if (!strcasecmp(part4, g2->user_nick)) {
		ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "nick.so: kicked from %s, rejoining", part3);
		g2->irc_channel_joined[channelid] = 'N';
	    } else {
		if ( db_write("DELETE FROM %s.%s WHERE user = '%s' AND channelid = '%d'", g2->mysql_db, g2->mysql_table_irc_users, part4, channelid) )
		    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "nick.so: error deleting kicked (%s) from (%d)(%s)", part4, channelid, part3);
	    }
	} else 
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "nick.so: kick to (%s) on a not known channel (%s)", part4, part3);    
	return (T_OK);	    
    } else if (!strcmp(part2, "353") && part3 && part4 && part5) {
	/* names list for a channel */
        /* :server 353 our_nick = #channel :nick @nick +nick nick */
        char *t, *t2;
        int i;
        int channelid = -1;

	context();

        for (j = 0; j < g2->irc_channel_count; j++) {
	    if (!strcasecmp(g2->irc_channel[j], part5)) {
	        channelid = j;
	        break;
	    }
	}
	if (channelid > -1) {
    	    for (i = 0; (t2 = t = getpart(fullline, 6 + i)) ; i++) {
		int umode = USER_MODE_HAS_0;
        	if (t[0] == ':') { umode |= USER_MODE_HAS_0;  t++; }
        	if (t[0] == '@') { umode |= USER_MODE_HAS_OP; t++; }
        	if (t[0] == '+') { umode |= USER_MODE_HAS_V;  t++; }
		if (t[0] == '@') { umode |= USER_MODE_HAS_OP; t++; }
		if (db_write("INSERT INTO %s.%s (channelid, user, mode) VALUES ('%d', '%s', '%d')", g2->mysql_db, g2->mysql_table_irc_users, channelid, t, umode))
		    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "nick.so: problem inserting user (%s)", t);
		mydelete(t2);
	    }	
	} else {
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "nick.so: userlist from unknown channel (%s)", part5);
	}
	return (T_OK);
    } else {
	return (T_IGNORE);
    }
    return (T_IGNORE);
}
