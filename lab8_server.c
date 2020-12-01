#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define MAXBUF 1024
#define PORTNUM 3600
int main(int argc, char **argv)
{

    int listen_fd, client_fd;
    pid_t pid;
    socklen_t addrlen;
    int readn;
    int client_copy[3] = {0, 0, 0};
    char buf[MAXBUF];
    struct sockaddr_in clientaddr, serveraddr;
    char copy[MAXBUF] = "";
    char blank[MAXBUF] = " ";
    int count = 0;
    char enter[MAXBUF] = "\n";

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
    signal(SIGCHLD, SIG_IGN);
    mkfifo("/tmp/myfifo_r", S_IRUSR | S_IWUSR);
    mkfifo("/tmp/myfifo_w", S_IRUSR | S_IWUSR);

    while (1)
    {
        
        memset(buf, 0x00, MAXBUF);
        addrlen = sizeof(clientaddr);
        client_fd = accept(listen_fd, (struct sockaddr *)&clientaddr, &addrlen);
        client_copy[count] = client_fd;
        if (client_fd == -1)
        {
            printf("accept error\n");
            break;
        }
        
        pid = fork();

        if (pid == 0)
        {
            close(listen_fd);
            memset(buf, 0x00, MAXBUF);
            read(client_fd, buf, MAXBUF);
            printf("Read DATA %s(%d) : %s\n", inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, buf);

            int wfd;
            char buf_pipe[MAXBUF];
            memset(buf_pipe, 0x00, MAXBUF);
           
            wfd = open("/tmp/myfifo_r", O_RDWR);
            if (count == 2)
            {
                int a = client_copy[0];
                int b = client_copy[1];
                int c = client_copy[2];
                write(a, copy, strlen(copy));
                write(b, copy, strlen(copy));
                write(c, copy, strlen(copy));
                close(a);
                close(b);
                close(c);
            }
            else
            {
                write(wfd,buf, MAXBUF);
            }
            return 0;
        }
        else if (pid > 0)
        {
            int rfd;
            char buf_pipe[MAXBUF];
            rfd = open("/tmp/myfifo_r", O_RDWR);
            memset(buf_pipe, 0x00, MAXBUF);
            if(count==2)
                break;
            read(rfd, buf_pipe, MAXBUF);
            
            else
            {
                if (count == 0)
                {
                strcpy(copy, buf_pipe);
                copy[strlen(copy) - 1] = '\0';
                }
                else
                {
                      strcat(strcat(copy, blank), buf_pipe);
                      copy[strlen(copy) - 1] = '\0';
                }
            }
            close(client_fd);
        }
        count++;
        if (count > 2)
            break;
    }
    return 0;
}