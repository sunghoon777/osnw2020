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
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>

#define PORTNUM 3600


struct dataset{
    char data1[15];
    int data2;
};

union semun
{
    int val;
};


int main(int argc, char **argv)
{

    int listen_fd, client_fd;
    pid_t pid;
    socklen_t addrlen;
    struct sockaddr_in clientaddr, serveraddr;
    int client_copy[3] = {0, 0, 0};
    char data1[15] = "\0"; 
    int data2 = 0;
    struct dataset *d = malloc(sizeof(struct dataset));
  


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

    while (1)
    {
        
        addrlen = sizeof(clientaddr);
        client_fd = accept(listen_fd, (struct sockaddr *)&clientaddr, &addrlen);
        read(client_fd, d, sizeof(d));
        printf("%s , %d",d->data1, d->data2);

        if (client_fd == -1)
        {
            printf("accept error\n");
            break;
        }
        
        pid = fork();

        if (pid == 0)
        {
            memset(d, 0x00, sizeof(d));
            close(listen_fd);
            fork();
            //consumer read from memory write
            if(pid == 0)
            {
                sleep(1);
                int shmid;
                int semid;
                void *shared_memory = NULL;
                struct sembuf semopen = {0, -1, SEM_UNDO};
                struct sembuf semclose = {0, -1, SEM_UNDO};
                shmid = shmget((key_t)1234, sizeof(d), 0666);
                semid = semget((key_t)3477, 0, 0666);
                shared_memory = shmat(shmid, NULL, 0);
                struct dataset *d_copy = malloc(sizeof(struct dataset));
                d_copy = shared_memory;
                while(1)
                {
                    //enter critical section
                    semop(semid, &semopen, 1);
                    strcpy(d->data1 ,((struct dataset *)d_copy)->data1);
                    d->data2 = ((struct dataset *)d_copy)->data2;
                    write(client_fd, d, sizeof(d));
                    semop(semid, &semclose, 1);
                    //end critical section
                    sleep(2);
                }
            }
            //producer read from client, operate, store data memory
            else if(pid >0)
            {
                //read from client
                read(client_fd, d, sizeof(d));
                printf("%s , %d",d->data1, d->data2);
                int shmid;
                int semid;
                void *shared_memory = NULL;
                union semun sem_union;
                struct sembuf semopen = {0, -1, SEM_UNDO};
                struct sembuf semclose = {0, -1, SEM_UNDO};
                shmid = shmget((key_t)1234, sizeof(d), 0666|IPC_CREAT);
                semid = semget((key_t)3477, 1, IPC_CREAT|0666);
                shared_memory = shmat(shmid, NULL, 0);
                struct dataset *d_copy = malloc(sizeof(struct dataset));
                sem_union.val = 1;
                semctl( semid, 0, SETVAL, sem_union);
                char data1_copy[15] = "\0";
                int data2_copy = 0;
                d_copy = shared_memory;
                strcpy( ((struct dataset *)d_copy)->data1, d->data1); //에러고쳐봄
                ((struct dataset *)d_copy)->data2 = d->data2;
                
                while(1)
                {
                    //enter critical section
                    semop(semid, &semopen, 1);
                    //operation
                    strcpy( data1_copy , ((struct dataset *)d_copy)->data1 ); //에러고쳐봄
                    data2_copy = ((struct dataset *)d_copy)->data2;
                    int temp = 0;
                    //shift operate
                    for(int i =0;i<strlen(data1_copy);i++)
                    {
                        if(i==0)
                        temp = data1_copy[0];
                        if(data1_copy[i+1] != NULL)
                        data1_copy[i] = data1_copy[i+1];
                        else
                        data1_copy[i] = temp;
                    }
                    //plus operation
                    data2_copy++;
                    strcpy(((struct dataset *)d_copy)->data1, data1_copy);
                    ((struct dataset *)d_copy)->data2 = data2_copy;
                    semop(semid, &semclose, 1);
                    //end critical section
                    sleep(1);
                }
            }
        
        }
        else if (pid > 0)
        {

        }
    }
    return 0;
}