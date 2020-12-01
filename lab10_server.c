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
#include <pthread.h>

#define PORTNUM 3600

//user data
struct dataset
{
    char data1[30];
    int data2;
};

struct shared_memory 
{
    struct dataset mydata_copy;
    pthread_mutex_t m_lock;
    int socket_fd;
};

//consumer
void *thread_consumer(void *share)
{
    sleep(1);
    struct shared_memory *my_share = (struct shared_memory *)share;
    while(1)
    {
         //start critical section
        pthread_mutex_lock(&(my_share->m_lock));
        write(my_share->socket_fd,(void *)&(my_share->mydata_copy), sizeof(my_share->mydata_copy));
		pthread_mutex_unlock(&(my_share->m_lock));
        sleep(1);
        //end critical section
    }
}

//producer
void *thread_producer(void *client_fd)
{
    pthread_t thread_id;
    int socket_fd = *((int *)client_fd);
    struct dataset mydata;
    memset((void *)&mydata , 0x00, sizeof(mydata));
    read(socket_fd, (void *)&mydata , sizeof(mydata));
    struct shared_memory *share;
    //create shared_memory(share)
    share = (struct shared_memory *)malloc(sizeof(struct shared_memory));
    strcpy(share->mydata_copy.data1, mydata.data1);
    share->mydata_copy.data2 = mydata.data2;
    share->socket_fd = socket_fd;
    pthread_mutex_init(&(share->m_lock), NULL);
    pthread_create(&thread_id, NULL, thread_consumer, (void *)share);
    while(1)
    {
        //start critical section
        pthread_mutex_lock(&(share->m_lock));
        int temp = 0;
        //shift operate
        for(int i =0;i<strlen(share->mydata_copy.data1);i++)
        {
            if(i==0)
            temp = share->mydata_copy.data1[0];
            if(share->mydata_copy.data1[i+1] != NULL)
                share->mydata_copy.data1[i] = share->mydata_copy.data1[i+1];
            else
                share->mydata_copy.data1[i] = temp;
        }
        //plus operation
        int a = ntohl(share->mydata_copy.data2);
        a++;
        share->mydata_copy.data2 = htonl(a);
		pthread_mutex_unlock(&(share->m_lock));
        sleep(1);
        //end critical section
    }    
}

//main
int main(int argc, char **argv)
{

    int listen_fd, client_fd;
    socklen_t addrlen;
    struct sockaddr_in clientaddr, serveraddr;
    pthread_t thread_id;

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
        printf("connect : %s\n",inet_ntoa(clientaddr.sin_addr));

        if (client_fd == -1)
        {
            printf("accept error\n");
            break;
        }
        pthread_create(&thread_id, NULL, thread_producer, (void *)&client_fd);
		pthread_detach(thread_id);
    }
    return 0;
}