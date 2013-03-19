
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define T_ERROR -1
#define T_OK 0

#define T_UNDEF 0

int module_locate(const char *d_name);
int file_isdir(char *d_name);

int main(int argc, char *argv[]) {
pthread_mutex_t m;

    pthread_mutex_init(&m, NULL);
    module_locate("./");
    pthread_mutex_destroy(&m);

return 0;

}

int module_locate(const char *d_name) {

DIR *handle;
struct dirent *handle_next;
char *path;
	    
    handle = opendir(d_name);
    if (!handle) {
	printf("modules opendir: %s\n", strerror(errno));
        return (T_ERROR);
    }
				
    while ( (handle_next = readdir(handle)) != NULL ) {
        if (!(!strcmp(handle_next->d_name, "..")
	    || !strcmp(handle_next->d_name, "."))) {
	    path = calloc(sizeof(char) * (strlen(handle_next->d_name) + strlen(d_name) + 2), 1);
	    strncpy(path, d_name, strlen(d_name));
	    if (path[strlen(path) - 1] != '/')
		strcat(path, "/");
	    strcat(path, handle_next->d_name);
	    printf("-> %s\n", path);
	    if (!file_isdir(path)) {
		module_locate(path);   /* handle_next->d_name */
	    } else {
		if ((strlen(handle_next->d_name) > 3)
	            && (path[strlen(path) - 2] == 's')
		    && (path[strlen(path) - 1] == 'o')) {
		    printf(" _%s_%s_\n", handle_next->d_name, path);
		}
	    }	    
	    free(path);
	}
    }
    if (closedir(handle)) {
    	printf("closedir %s\n", strerror(errno));
        return (T_ERROR);
    }
    return (T_OK);
}

int file_isdir(char *d_name) {
struct stat estado;
int res = T_ERROR;
char tmpbuff[256];

    if (stat(d_name, &estado) >= T_UNDEF) {
        if (S_ISDIR(estado.st_mode)) {
	    if (readlink(d_name, tmpbuff, sizeof(tmpbuff)) == -1 ) {
		res = T_OK;
	    }
	}
    } else {
    	printf("stat %s (%s)\n", strerror(errno), d_name);    
    }

    return res;
}

