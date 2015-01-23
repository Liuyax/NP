#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT1 8000
#define PORT2 8001
#define IP_ADDRESS1 "127.0.0.1"
#define IP_ADDRESS2 "127.0.0.2"
#define    FINISH_FLAG    "FILE_TRANSPORT_FINISH"
#define    MAXLINE        1024*32
void *part1(void *buf1);
void *part2(void *buf2);

unsigned int off_len = 1024*1024*512;
void *part1(void *buf1);
void *part2(void *buf2);

int main(int argc,char **argv)
{
    pthread_t thread1,thread2;
    char filename[10];
    sprintf(filename,"%s",argv[1]);
    if(pthread_create(&thread1,NULL,part1,filename) !=0 )
    {
        perror("p1 pthread_create");
        return 0;
    }
    if(pthread_create(&thread2,NULL,part2,filename) !=0 )
    {
        perror("p2 pthread_create");
        return 0;
    }

    pthread_join(thread1,NULL);
    printf("thread1 done\n");
    pthread_join(thread2,NULL);
    printf("thread2 done\n");
    return 0;
}
void *part1(void *buf1)
{
    char echo[20];
    FILE                   *fp1;
    struct sockaddr_in     server_addr;
    char                   buf[MAXLINE];
    int                    sock_id,read_len,send_len,server_addr_len,ret,i,pid;
    unsigned int send_sum = 0;
    if ((fp1 = fopen((char*)buf1,"rb")) == NULL)
    {
        perror("Open file failed\n");
        exit(0);
    }
    if ((sock_id = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Create socket failed");
        exit(0);
    }


    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT1);

    inet_pton(AF_INET, IP_ADDRESS1, &server_addr.sin_addr);
    server_addr_len = sizeof(server_addr);
    ret = connect(sock_id, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    if (ret == -1)
    {
        perror("Connect socket failed!\n");
        exit(0);
    }
    bzero(buf, MAXLINE);
    server_addr_len = sizeof(server_addr);
    //unsigned lose_g = 0;
    while (read_len = fread(buf, 1, MAXLINE, fp1),read_len)
    {
        send_len = send(sock_id, buf, read_len, 0);
   //     if(lose_g != 30 &&lose_g != 60)
    //    {
        if ( send_len < 0 )
        {
            perror("Send data failed\n");
            exit(0);
        }
   //     }
        bzero(buf, MAXLINE);
        recv(sock_id, echo, 20, 0);
        while(strcmp(echo, "timeout") == 0)
        {
            send(sock_id, buf, read_len, 0);
            recv(sock_id, echo, 20, 0 );
        }
        send_sum += send_len;
        if(send_sum >= off_len)
        {
            printf("part1 :send =%d\n send_sum = %u\n", send_len,send_sum);
            break;
        }
   //     lose_g++;
    }
    printf("part1 :send_sum = %u\n\n",send_sum);
    fclose(fp1);
    close(sock_id);

}
void *part2(void *buf2)
{
    FILE                  *fp2;
    struct sockaddr_in     server_addr2;
    char                   bf2[MAXLINE];
    char echo2[20];
    int                    sock_id2,read_len2,send_len2,server_addr_len2,ret2,i2;
    unsigned int send_sum2 = 0;
    if ((fp2 = fopen((char*)buf2,"rb")) == NULL)
    {
        perror("fp2 Open file failed\n");
        exit(0);
    }
    fseek(fp2,off_len,0);
    if ((sock_id2 = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Create socket failed");
        exit(0);
    }


    memset(&server_addr2,0,sizeof(server_addr2));
    server_addr2.sin_family = AF_INET;
    server_addr2.sin_port = htons(PORT2);

    inet_pton(AF_INET, IP_ADDRESS2, &server_addr2.sin_addr);

    server_addr_len2 = sizeof(server_addr2);

    ret2 = connect(sock_id2, (struct sockaddr *)&server_addr2, sizeof(struct sockaddr));
    if (ret2 == -1)
    {
        perror("Connect socket failed!\n");
        exit(0);
    }
    bzero(bf2, MAXLINE);
    server_addr_len2 = sizeof(server_addr2);
    while (read_len2 = fread(bf2, 1, MAXLINE, fp2),read_len2)
    {
        send_len2 = send(sock_id2, bf2, read_len2, 0);
        if ( send_len2 < 0 )
        {
            perror("Send data failed\n");
            exit(0);
        }
        bzero(bf2, MAXLINE);
        recv(sock_id2, echo2, 20, 0 );
        while(strcmp(echo2, "timeout") == 0)
        {
            send(sock_id2, bf2, read_len2, 0);
            recv(sock_id2, echo2, 20, 0 );
        }
        send_sum2 += send_len2;
    }
    printf("part2 sendsum :%u\n",send_sum2);

    if(feof(fp2)==0)
    {
        sprintf(bf2,"error");
        send(sock_id2,bf2,6,0);
    }

    fclose(fp2);
    memset(bf2,0,10);
    sprintf(bf2,"finish");
    send(sock_id2,bf2,7,0);
    close(sock_id2);
    printf("part2 Send finish\n");
}

