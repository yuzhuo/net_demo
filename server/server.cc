

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

    int sfd, cfd;
    struct sockaddr_in my_addr, peer_addr;
    socklen_t peer_addr_size;
 
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1)
	    handle_error("socket");

    int on = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
        handle_error("setsockopt");
 
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(1234);

    char sip[kMaxAddrLen];
    inet_ntop(AF_INET, &my_addr.sin_addr.s_addr, sip, kMaxAddrLen);
    printf("bind info\n");
    printf("ip: %s, port: %d\n", sip, 1234);
 
    if (bind(sfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
	    handle_error("bind");
 
    if (listen(sfd, kBacklog) == -1)
	    handle_error("listen");
 
    unsigned int addr_len = sizeof(peer_addr);
    printf(" ** addr_len: %d\n", addr_len);

    while (1) {
        if ((cfd = accept(sfd, (struct sockaddr *)&peer_addr, &addr_len)) == -1)
            handle_error("accept");
        printf(" ** addr_len: %d\n", addr_len);
     
        char cip[kMaxAddrLen];
        inet_ntop(AF_INET, &peer_addr.sin_addr.s_addr, cip, kMaxAddrLen);
        int port = ntohs(peer_addr.sin_port);
        printf("client info: \n");
        printf("ip: %s, port: %d\n", cip, port);
        
        int nread = -1;
        unsigned char buf[kBufLen + 1];
        printf("reading [%d]...\n", cfd);

        while ((nread = read(cfd, buf, kBufLen)) > 0) {
            printf("*** nread: %d\n", nread);
            for (int i = 0; i < nread; ++i)
                printf("0x%x ", buf[i]);
            printf(" --- \n");
        }
        printf("nread: %d\n", nread);
        close(cfd);
    }
    
    close(sfd);
 
    return 0;
}

