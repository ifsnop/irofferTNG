#define _MOD_
#include "../modules.h"
char *stmt;
Function *f = NULL;

int mod_load(char *name, Function * f2, struct global_t * g2) {

    f = f2;

    context();

    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "module %s loading", name);

    if (!db_write("DROP TABLE IF EXISTS %s.%s", g2->mysql_db, g2->mysql_table_irc_users))
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "%s: %s.%s deleted", name, g2->mysql_db, g2->mysql_table_irc_users);
    if (!db_write("CREATE TABLE %s ( channelid int NOT NULL, user varchar(12) NOT NULL,  mode int DEFAULT 0 )", g2->mysql_table_irc_users) ) {
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "%s: %s.%s created", name, g2->mysql_db, g2->mysql_table_irc_users);
	return (MODULE_HOOK_EVERY20SEC | MODULE_HOOK_SERVERQUIT);
    } else {
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "%s: couldn't reset irc_users", name);
	return (MODULE_HOOK_RETRY);
    } 
}

int mod_unload(char *name, struct global_t * g2) {

    context();

    if (!db_write("DROP TABLE IF EXISTS %s.%s", g2->mysql_db, g2->mysql_table_irc_users))
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "%s: %s.%s deleted", name, g2->mysql_db, g2->mysql_table_irc_users);

    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "module %s unloading", name);
    return (T_OK);
}


int mod_every20sec(struct global_t * g2) {
unsigned int i;
char tmp[TEXTLENGTH_STD];

    context();

    bzero(tmp,TEXTLENGTH_STD);
    if ( g2->serverstatus == SERVER_STATUS_CONNECTED_2 ) {
	for (i = 0; i < g2->irc_channel_count; i++) {
	    if ( g2->irc_channel_joined[i] == 'N') {
		if ( db_write("DELETE FROM %s.%s WHERE channelid = '%d'", g2->mysql_db, g2->mysql_table_irc_users, i) )
		    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "channel.so: can't delete userlist of (%d)(%s)", i,g2->irc_channel[i]);
		sprintf(tmp, "JOIN %s", g2->irc_channel[i]);
		server_write(tmp, SERVER_SEND_NORMAL);
	    }
	}
    }
    return (T_IGNORE);
}

int mod_serverquit(struct global_t * g2) {
unsigned int i;

    if ( db_write("DELETE FROM %s.%s", g2->mysql_db, g2->mysql_table_irc_users) )
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "channel.so: can't delete full userlist from %s", g2->mysql_table_irc_users);
    for (i = 0; i < g2->irc_channel_count; i++)
	g2->irc_channel_joined[i] = 'N';
							
    return (T_IGNORE);
}
