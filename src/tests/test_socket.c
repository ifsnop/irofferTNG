
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>	     
#include <netinet/in.h>		     

int main(int argc, char *argv[]) {

struct sockaddr_in serverip;
struct hostent *remotehost;
int retval;
int addrlen;
int serversocket;

    serversocket = socket(AF_INET, SOCK_STREAM, 0);
    printf("1\n");
    serverip.sin_family = AF_INET;
    serverip.sin_port = htons(atoi("6667"));
    printf("1.5\n");
    remotehost = gethostbyname("localhost");
    printf("2\n");
    
    memcpy(&serverip.sin_addr, *((struct in_addr **) remotehost->h_addr_list), sizeof(struct in_addr));
    printf("3\n");

/*
    inet_aton("127.0.0.1", &serverip.sin_addr);
*/
    retval = connect(serversocket, (struct sockaddr *) &serverip, sizeof(serverip));
    printf("4\n");
/*
    addrlen = sizeof(serverip);
    getsockname(serversocket, (struct sockaddr *) &serverip, &addrlen);
    printf("5\n");
*/
    close(serversocket);
    printf("6\n");
    
    return 0;
}

