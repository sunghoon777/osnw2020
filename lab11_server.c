#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PORTNUM 3600

//user data
struct dataset
{
    char data1[1024];
    int data2;
};

//main
int main(int argc, char **argv)
{
    int count = 0;
    int listen_fd, client_fd;
    socklen_t addrlen;
    struct sockaddr_in clientaddr, serveraddr;
    fd_set readfds, allfds;
    struct dataset mydata;
    struct dataset mydata_copy;
    int sockfd;
    int i;
    int client_copy[1024];
    int fd_num;
    int maxfd = 0;
    memset((void *)&mydata_copy, 0x00, sizeof(mydata_copy));

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error : ");
        return 1;
    }

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(PORTNUM);

    bind(listen_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    listen(listen_fd, 5);
    FD_ZERO(&readfds);
    FD_SET(listen_fd, &readfds);
    maxfd = listen_fd;

    while (1)
    {
        memset((void *)&mydata, 0x00, sizeof(mydata));
        allfds = readfds;
        printf("Select Wait %d\n", maxfd);
        fd_num = select(maxfd + 1, &allfds, (fd_set *)0, (fd_set *)0, NULL);

        if (FD_ISSET(listen_fd, &allfds))
        {
            addrlen = sizeof(clientaddr);
            client_fd = accept(listen_fd, (struct sockaddr *)&clientaddr, &addrlen);
            client_copy[count] = client_fd;
            FD_SET(client_fd, &readfds);
            if (client_fd > maxfd)
                maxfd = client_fd;
            printf("Accept OK\n");
            continue;
        }

        for (i = 0; i <= maxfd; i++)
        {
            memset((void *)&mydata, 0x00, sizeof(mydata));
            sockfd = i;
            if (FD_ISSET(sockfd, &allfds))
            {
                if (read(sockfd, (void *)&mydata, sizeof(mydata)) <= 0)
                {
                    close(sockfd);
                    FD_CLR(sockfd, &readfds);
                }
                else
                {
                    // read를 통해 온 mydata안에 데이터가 존재하지 않더라도 문자열 연결이나 숫자 덧셈은 동작하게된다.
                    //문자열 연결
                    if (count == 0)
                    {
                        strcpy(mydata_copy.data1, mydata.data1);
                        mydata_copy.data1[strlen(mydata_copy.data1) - 1] = '\0';
                    }
                    else
                    {
                        strcat(mydata_copy.data1, mydata.data1);
                        mydata_copy.data1[strlen(mydata_copy.data1) - 1] = '\0';
                    }
                    //숫자 덧셈
                    int a = ntohl(mydata.data2);
                    mydata_copy.data2 = mydata_copy.data2 + a;
                    a=0;
                    //주기적으로 write함 , client으로부터 3번 메시지를 받을때마다 write 동작을 하게된다.
                    if ((count + 1) % 3 == 0)
                    {
                        mydata_copy.data2 = htonl(mydata_copy.data2);
                        for (int j = 0; j <= count; j++)
                        {
                            //지금까지 연결된 클라이언트 소캣들에게 전부 write를 하게 된다.
                            write(client_copy[j], (void *)&mydata_copy, sizeof(mydata_copy));
                        }
                        mydata_copy.data2 = ntohl(mydata_copy.data2);
                    }
                }
            }
        }
        count++;
    }
    return 0;
}