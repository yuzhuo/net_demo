

#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


const int kBacklog = 1;
const int kBufLen = 16;
const int kMaxAddrLen = 128; 

static void handle_error(const char *msg)
{
   perror(msg);
   exit(EXIT_FAILURE);
}


void int_handler(int)
{
    printf("received a SIGINT signal!\n");
}

int main()
{
    signal(SIGINT, int_handler);

    int cfd;
    struct sockaddr_in my_addr, peer_addr;
    socklen_t peer_addr_size;
 
    if ((cfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	    handle_error("socket");
 
    memset(&my_addr, 0, sizeof(my_addr));
    /*
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(1235);
    */

    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &peer_addr.sin_addr.s_addr);
    peer_addr.sin_port = htons(1234);

    if (bind(cfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
	    handle_error("bind");

    if (connect(cfd, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) == -1)
        handle_error("connect");
    
    while(1)
    {
        sleep(1);
    }

    close(cfd);
 
    return 0;
}

