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
struct db_row_t row;
int activecols = 0;
int deactivecols = 0;
int completecols = 0;

    context();
    
    if (!strncasecmp(msg1, "!PSVERSION", 10)) {
	row = db_read("SELECT mode FROM %s.%s WHERE user = '%s'", g2->mysql_db, g2->mysql_table_irc_users, nick);
	if (row.num_fields >=1) {
	    if ( atoi(row.r[0]) != USER_MODE_HAS_0 ) {
		server_notice(nick, "PhotoServe Version: %s Mirc Version: na", g2->pserve_version);
		server_ctcp(nick, "PhotoServe Version: %s Mirc Version: na", g2->pserve_version);
    	    }
	    db_row_clean(row);
	    return (T_OK);
	}
    } else if (!strncasecmp(msg1, "!SERVINFO", 9)) {
	row = db_read("SELECT COUNT(id) FROM collections WHERE active = '1'");
	if ( row.num_fields >= 1 ) activecols = atoi(row.r[0]); else activecols = 0;
	db_row_clean(row);

	row = db_read("SELECT COUNT(id) FROM collections WHERE active = '0'");
	if ( row.num_fields >= 1 ) deactivecols = atoi(row.r[0]); else deactivecols = 0;
	db_row_clean(row);

	row = db_read("SELECT COUNT(id) FROM collections WHERE complete = '1'");
	if ( row.num_fields >= 1) completecols = atoi(row.r[0]); else completecols = 0;
	db_row_clean(row);
	
	server_notice(nick, "@Photoserve_!ServInfo,"
			"ON,"    /* pserve status */
			"%s,"    /* version info */
			" ,"     /* */
			"%s,"    /* trigger */
			"%s,"    /* up/down speed */ 
			"0.0KB," /* average upload total */
			".000 KB/sec," /* max send kbps */
			"%d,%d,%d,%d," /* active / de-active / full # cols */
			"0,0,0," /* Sends current / per nick / max */
			"0,0,0," /* Queue current / per nick / max */
			"0,0,"   /* # of gets for a network / # of leech windows for a network */
			"lnx version again", 
		    g2->pserve_version, g2->pserve_trigger, g2->pserve_speed, 
		    activecols, deactivecols, activecols + deactivecols);
	return (T_OK);
    } else if (!strncasecmp(msg1, "!WhereIs", 8) && msg2) {
	struct db_row_t row2;
	row = db_read("SELECT COUNT(owned) FROM csvs, collections WHERE csvs.id=collections.id AND collections.name='%s' and csvs.owned='1'", msg2);
	row2 = db_read("SELECT COUNT(owned) FROM csvs, collections WHERE csvs.id=collections.id AND collections.name='%s'", msg2);
	
	if ((row.num_fields >= 1) && (row2.num_fields >= 1)) {
	    server_notice(nick,"total: %s owned: %s", row2.r[0], row.r[0]);
	}
	db_row_clean(row2);
	db_row_clean(row);
    }
    return (T_IGNORE);
}
