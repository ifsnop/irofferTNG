#define _MOD_
#include "../modules.h"

Function *f = NULL;

int mod_load(char *name, Function * f2, struct global_t * g2) {

    f = f2;

    context();
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "module %s loading", name);

    return (MODULE_HOOK_PRIVMSG);
}

int mod_unload(char *name, struct global_t * g2) {
    context();
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "module %s unloading", name);
    return (T_OK);
}

int mod_privmsg(char *fullline, char *nick, char *hostname, 
		char *hostmask, char *dest, char *msg1,
		char *msg2, char *msg3, char *msg4,
		char *msg5, struct global_t * g2) {
char *p;

    context();
    
//    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "commands.so: >PRIVMSG full(%s) nick(%s) hostname(%s) hostmask(%s) dest(%s) 1(%s) 2(%s) 3(%s) 4(%s) 5(%s)",
//	    fullline, nick, hostname, hostmask, dest, msg1, msg2, msg3, msg4, msg5);

    if (!msg2)
	return (T_IGNORE);

    p = strchr((fullline + 1), ':');
    if (!p)
	return (T_IGNORE);

    p = strchr((p + 1), ':');
    if (!p)
	return (T_IGNORE);
    p++;

    if (!strncasecmp(msg1, "RAWNOTICE", 9)) {
	server_notice(msg2, p);
	return (T_OK);
    } else if (!strncasecmp(msg1, "RAWCTCP", 7)) {
	server_ctcp(msg2, p);
	return (T_OK);
    }
    
    return (T_IGNORE); //pues no era para mi, no xD
}

