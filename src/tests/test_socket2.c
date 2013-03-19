#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdio.h>

#define T_OK 0
#define T_ERROR -1

int main(int argc, char *argv[]) {

struct addrinfo hints, *remotehost0;
int retval;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    if ( getaddrinfo("www.google.com", "80", &hints, &remotehost0) < 0 ) {
	printf("getaddrinfo error: %s", gai_strerror(retval));
	return -1;
    }

    freeaddrinfo(remotehost0);
    
    return 0;
}
