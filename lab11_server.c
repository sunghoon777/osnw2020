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
    char data1[30];
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
    int client_copy[3] = {0 , 0, 0};
    int fd_num;
	int maxfd = 0;
    memset((void *)&mydata_copy , 0x00, sizeof(mydata_copy));



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
        memset((void *)&mydata , 0x00, sizeof(mydata));
        allfds = readfds;
        printf("Select Wait %d\n", maxfd);
        fd_num = select(maxfd + 1 , &allfds, (fd_set *)0, (fd_set *)0, NULL);

        if (FD_ISSET(listen_fd, &allfds))
		{
			addrlen = sizeof(clientaddr);
			client_fd = accept(listen_fd, (struct sockaddr *)&clientaddr, &addrlen);
            client_copy[count] = client_fd;
			FD_SET(client_fd,&readfds);
			if (client_fd > maxfd)
				maxfd = client_fd;
			printf("Accept OK\n");
			continue;
		}
        printf("connect : %s\n",inet_ntoa(clientaddr.sin_addr));


        for (i = 0; i <= maxfd; i++)
		{
            
            sockfd = i;
            if (FD_ISSET(sockfd, &allfds))
			{
                if (read(sockfd, (void *)&mydata , sizeof(mydata)) <= 0)
				{
					close(sockfd);
					FD_CLR(sockfd, &readfds);
				}
                else
                {
                    if(count == 0)
                    {
                        strcpy(mydata_copy.data1,mydata.data1);
                        mydata_copy.data1[strlen(mydata_copy.data1)-1] = '\0';
                    }
                    else
                    {
                        strcat(mydata_copy.data1,mydata.data1);
                        mydata_copy.data1[strlen(mydata_copy.data1)-1] = '\0';
                        
                    }
                    int a = ntohl(mydata.data2);
                    mydata_copy.data2 = mydata_copy.data2 + a;    
                    if(count >= 2)
                    {
                        mydata_copy.data2 = htonl(mydata_copy.data2);
                        write(client_copy[0],(void *)&mydata_copy , sizeof(mydata_copy));
                        write(client_copy[1],(void *)&mydata_copy , sizeof(mydata_copy));
                        write(client_copy[2],(void *)&mydata_copy , sizeof(mydata_copy));
                    }
                }
            }
        }
        count++;
    }
    return 0;
}