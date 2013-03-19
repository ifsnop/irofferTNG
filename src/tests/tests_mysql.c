
#include <mysql/mysql.h>


int main(int argc, char *argv[]) {

 MYSQL * mysql_dbh;
 
 mysql_dbh = mysql_init(0);
 
 mysql_close(mysql_dbh);

    return 0;
}
