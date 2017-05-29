

#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <limits.h>
#include <errno.h>
#include <sys/stropts.h>


const int kBacklog = 1;
const int kBufLen = 16;
const int kMaxAddrLen = 128; 
#define OPEN_MAX 1024

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

    struct pollfd client[OPEN_MAX];
    client[0].fd = sfd;
    client[0].events = POLLRDNORM;
    for (int i = 1; i < OPEN_MAX; ++i) {
        client[i].fd = -1;
    }

    int maxi = 0;
    for (;;) {
        printf("\nmaxi=%d, polling", maxi);
        int nready = poll(client, maxi + 1, -1);

        if (-1 == nready)
            handle_error("poll");

        if (client[0].revents & POLLRDNORM) {
            unsigned int addr_len = sizeof(peer_addr);
            cfd = accept(sfd, (struct sockaddr *)&peer_addr, &addr_len);
            printf("\naccept [%d]\n", cfd);
            if (-1 == cfd)
                handle_error("accept");

            int i = 1;
            for ( ; i < OPEN_MAX; ++i) {
                if (client[i].fd < 0) {
                    client[i].fd = cfd;
                    client[i].events = POLLRDNORM;
                    break;
                }
            }

            if (OPEN_MAX == i)
                handle_error("too many clients");

            if (i > maxi)
                maxi = i;

            if (--nready <= 0)
                continue;
        }

        for (int i = 1; i < OPEN_MAX; ++i) {
            if ((cfd = client[i].fd) < 0)
                continue;

            if (client[i].revents & (POLLRDNORM | POLLERR)) {
                int nread = 0;
                char buf[kBufLen + 1];
                printf("\nread from [%d]", cfd);
                if ((nread = read(cfd, buf, kBufLen)) < 0) {
                    if (errno == ECONNRESET) {
                        close(cfd);
                        client[i].fd = -1;
                        printf("\nreset client[%d]", cfd);
                    } else {
                        handle_error("read error");
                    }
                } else if (0 == nread) {
                    close(cfd);
                    client[i].fd = -1;
                    printf("\nclose client[%d]", cfd);
                } else {
                    buf[nread] = 0;
                    printf("\n");
                    printf("%s", buf);
                }

                if (nready <= 0)
                    break;
            }
        }
    }

    close(sfd);
 
    return 0;
}

