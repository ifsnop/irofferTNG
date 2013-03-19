#include <stdio.h>
#include <crc32.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*int stat(const char *file_name, struct stat *buf);*/
			    
int main (int argc, char *argv[]) {

/*struct stat *buf;*/

    FILE *f;
    unsigned long tmp;

    gen_table();
    
    if (argc != 2)
	return -1;

    if ((f = fopen(argv[1], "r")) == NULL)
	return -1;
	
    tmp = get_crc(f);
/*
    if (stat(argv[1], buf) == -1 ) 
	return -1;
*/
//    printf(/*"%ld*"*/ "%08X"/*, buf->st_size*/, tmp);
    printf("%08X", (int) tmp);
    
    return 0;
}

