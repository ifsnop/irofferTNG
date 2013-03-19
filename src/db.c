#include "defines.h"

void db_row_clean(struct db_row_t row) {
int i;

    context();
    
    for (i=0; i<row.num_fields; i++) {
	mydelete(row.r[i]);
    }

    if (row.num_fields != -1) {
	mydelete(row.r);
    }
    row.num_fields = -1;
    return;
}

struct db_row_t db_read(const char *q, ...) {
MYSQL_RES * res;
MYSQL_ROW row;
struct db_row_t ret;
char *query;
va_list args;
int pres=0, i; 

    context();

    query = mycalloc(TEXTLENGTH_MAX);

    va_start(args, q);
    vsnprintf(query, TEXTLENGTH_MAX - 2, q, args);
    va_end(args);

    ret.num_fields = -1;
    
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_SQL, "db_read 1: %s", query);
    if (g.mysql_connected) {
	pres = pthread_mutex_lock(&g.mutex_db);
	if (pres)
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "db_read adquire: %s", strerror_r(errno, NULL, 0));
	if (!mysql_real_query(g.mysql_dbh, query, strlen(query))) {
	    if ((res = mysql_store_result(g.mysql_dbh)) != NULL) {
		if (mysql_num_rows(res) > 0) {
		    row = mysql_fetch_row(res);
		    ret.num_fields = mysql_field_count(g.mysql_dbh);
		    ret.r = mycalloc(sizeof(char *)*ret.num_fields);
		    for(i=0; i<ret.num_fields; i++) {
			ret.r[i] = mycalloc(strlen(row[i]) + 1);
			strcpy(ret.r[i], row[i]);
		    }
		}
		mysql_free_result(res);
		pres = pthread_mutex_unlock(&g.mutex_db);
		if (pres)
		    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "db_read release 1: %s", strerror_r(errno, NULL, 0));
		mydelete(query);
		return ret;
	    } 
	}
        pres = pthread_mutex_unlock(&g.mutex_db);
	if (pres)
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "db_read release 2: %s", strerror_r(errno, NULL, 0));
    } else {
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_SQL, "no connection to database");
	mydelete(query);
	return ret;
    }
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_SQL, "db_read 2: %s", mysql_error(g.mysql_dbh));
    mydelete(query);
    return ret;
}

int db_write(const char *q, ...) {
char *query;
va_list args;
int pres=0; 

    context();

    query = mycalloc(TEXTLENGTH_MAX);

    va_start(args, q);
    vsnprintf(query, TEXTLENGTH_MAX - 2, q, args);
    va_end(args);
    
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_SQL, "db_write 1: %s", query);
    if (g.mysql_connected) {
	pres = pthread_mutex_lock(&g.mutex_db);
	if (pres)
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "db_write adquire: %s", strerror_r(errno, NULL, 0));
	if (!mysql_real_query(g.mysql_dbh, query, strlen(query))) {
	    mydelete(query);
	    pres = pthread_mutex_unlock(&g.mutex_db);
	    if (pres)
	        ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "db_write release 1: %s", strerror_r(errno, NULL, 0));
	    return T_OK;
	} else {
	    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_SQL, "db_write 2 %s", mysql_error(g.mysql_dbh));
	    mydelete(query);
	    pres = pthread_mutex_unlock(&g.mutex_db);
	    if (pres)
	        ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "db_write release 2: %s", strerror_r(errno, NULL, 0));
	    return T_ERROR;
	}
    } else {
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_SQL, "no connection to database");
        mydelete(query);
	return T_ERROR;
    }
}

