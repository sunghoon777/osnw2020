#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


struct dataset{
    char data1[1024];
    int data2;
};

int main(int argc, char **argv)
{
    struct sockaddr_in serveraddr;
    int server_sockfd;
    int client_len;
    struct dataset mydata;
    memset((void *)&mydata , 0x00, sizeof(mydata));

    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("error :");
        return 1;
    }
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.01");
    serveraddr.sin_port = htons(3600);
    client_len = sizeof(serveraddr);

    if (connect(server_sockfd, (struct sockaddr *)&serveraddr, client_len) == -1)
    {
        perror("connect error :");
        return 1;
    }

    while(1)
    {
        // I/0 occur
        read(STDIN_FILENO, mydata.data1, sizeof(mydata.data1));
        //quit 입력시 끝남
        if(strncmp(mydata.data1, "quit\n",5) == 0)
      	break;
        scanf("%d", &mydata.data2);
        mydata.data2 = htonl(mydata.data2);
        write(server_sockfd, (void *)&mydata , sizeof(mydata));
        read(server_sockfd, (void *)&mydata , sizeof(mydata));
        printf("read : %s       %d\n", mydata.data1, ntohl(mydata.data2));
        memset((void *)&mydata , 0x00, sizeof(mydata));
    }
    return 0;
}