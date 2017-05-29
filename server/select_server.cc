
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>


const int kBacklog = 1;
const int kBufLen = 16;
const int kMaxAddrLen = 128; 

static void handle_error(const char *msg)
{
   perror(msg);
   exit(EXIT_FAILURE);
}


void int_handler(int) {
    printf("received a SIGINT signal!\n");
}

int main()
{
    signal(SIGINT, int_handler);

    int sfd, cfd;
    struct sockaddr_in my_addr, peer_addr;
    socklen_t peer_addr_size;
    int client[FD_SETSIZE];
 
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

    fd_set allset;
    fd_set rset;
    int maxfd = sfd;
    int maxi = -1;
    for (int i = 0; i < FD_SETSIZE; ++i) {
        client[i] = -1;
    }

    FD_ZERO(&allset);
    FD_SET(sfd, &allset);

    for (;;) {
        rset = allset;
        printf("\n** begin select");
        printf("\n");
        for (int i = 0; i < 32; ++i) {
            printf("%d", FD_ISSET(i, &allset));
        }
        fflush(stdout);
        int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        printf("\n** select returned: %d", nready);

        if (nready == -1)
            handle_error("select");
        
        if (FD_ISSET(sfd, &rset)) {
            if ((cfd = accept(sfd, (struct sockaddr *)&peer_addr, &addr_len)) == -1)
                handle_error("accept");
            
            printf("\naccept [%d]\n", cfd);

            int i = -1;
            for (i = 0; i < FD_SETSIZE; ++i) {
                if (client[i] < 0) {
                    client[i] = cfd;
                    break;
                }
            }

            if (i == FD_SETSIZE)
                handle_error("too many clients.");

            FD_SET(cfd, &allset);

            if (cfd > maxfd)
                maxfd = cfd;

            if (i > maxi)
                maxi = i;

            if (--nready <= 0)
                continue;
        }

        for (int i = 0; i <= maxi; ++i) {
            if ((cfd = client[i]) < 0)
                continue;

            if (FD_ISSET(cfd, &rset)) {
                int nread;
                char buf[kBufLen + 1];
                printf("\nread from [%d]", cfd);
                if ((nread = read(cfd, buf, kBufLen)) > 0) {
                    buf[nread] = 0;
                    printf("\n");
                    printf("%s", buf);
                } else {
                    char buf[32];
                    sprintf(buf, "\nclosed [%d]", cfd);
                    printf("%s\n", buf);
                    FD_CLR(cfd, &allset);
                    close(cfd);

                    client[i] = -1;
                }

                if (--nready <= 0)
                    break;
            }
        }
    }
    
    close(sfd);
 
    return 0;
}

