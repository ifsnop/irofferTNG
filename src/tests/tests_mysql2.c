
#include <mysql/mysql.h>
#include <stdio.h>


int main(int argc, char *argv[]) {

 MYSQL mysql_dbh;

// mysql_dbh = mysql_init(mysql_dbh);
// mysql_close(mysql_dbh);
mysql_init(&mysql_dbh);
    return 0;
}
