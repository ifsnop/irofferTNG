#define _MOD_
#include "../modules.h"

Function *f = NULL;

int mod_load(char *name, Function * f2, struct global_t * g2) {

   struct db_row_t row;

    f = f2;

    context();
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "module %s loading", name);

    g2->mysql_dbh = mysql_init(NULL);

    mysql_options(g2->mysql_dbh, MYSQL_READ_DEFAULT_GROUP, g2->mysql_db);
    if (!mysql_real_connect(g2->mysql_dbh, g2->mysql_host, g2->mysql_user, g2->mysql_pass, g2->mysql_db, 0, NULL, 0)) {
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "%s: failed to connect to database: %s", name, mysql_error(g2->mysql_dbh));
	    g2->mysql_connected = 0;
    } else {
	g2->mysql_connected = 1;

	row = db_read("SELECT COUNT(nick) FROM rb.users");
	if (row.num_fields >= 1)
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "%s: total users: %d", name, (int) atoi(row.r[0]));
	db_row_clean(row);

	row = db_read("SELECT COUNT(crc) FROM rb.csvs");
	if (row.num_fields >= 1)
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "%s: total files: %d", name, (int) atoi(row.r[0]));
	db_row_clean(row);
    }
    
    return (MODULE_HOOK_EVERY20SEC);
}

int mod_unload(char *name, struct global_t * g2) {
    context();

    mysql_close(g2->mysql_dbh);
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_DEBUG, "module %s unloading", name);

    return (T_OK);
}

int mod_every20sec(struct global_t * g2) {

    context();

    if ( mysql_ping(g2->mysql_dbh) ) {
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN, "Failed to connect to database (%s)", mysql_error(g2->mysql_dbh));
	g2->mysql_connected = 0;		 
    } else
	g2->mysql_connected = 1;

    return (T_IGNORE);
}

