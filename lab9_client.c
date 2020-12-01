#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


struct dataset{
    char data1[15];
    int data2;
};

int main(int argc, char **argv)
{

    struct sockaddr_in serveraddr;
    int server_sockfd;
    int client_len;
    char data1[15] = "\0"; 
    int data2 = 0;
    struct dataset *d = malloc(sizeof(struct dataset));

    printf("문자열을 입력하세요 : ");
    gets(data1);
    printf("숫자를 입력하세요 : ");
    scanf("%d", &data2);
    
    strcpy(d->data1, data1);
    d->data2 = data2;
    print("%s 와 %d 를 전송하였습니다.",d->data1, d->data2);
 
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

    if (write(server_sockfd, d, sizeof(d)) <= 0)
    {
        perror("write error : ");
        return 1;
    }

    while (1)
    {
        sleep(1);
        read(server_sockfd, d, sizeof(d));
        printf("read : %s     %d\n", d->data1, d->data2);
        memset(d, 0x00, sizeof(d));
    }
    return 0;
}