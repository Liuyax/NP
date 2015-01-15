#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define PORT1 8000
#define PORT2 8001
#define IP_ADDRESS1 "127.0.0.1"
#define IP_ADDRESS2 "127.0.0.2"
#define    FINISH_FLAG    "FILE_TRANSPORT_FINISH"
#define    MAXLINE        1024*32 /*max datagram = 64k*/
unsigned int part1_rec = 0;
unsigned int part2_rec = 0;

unsigned int off_len = 1024*1024*512;
void *part1(void *buf1);
void *part2(void *buf2);

int main(int argc,char **argv)
{
    FILE                   *fp;
    char *p1_,*p2_;
    p1_ = (char*)malloc(1024*1024*600);
    p2_ = (char*)malloc(1024*1024*800);
    pthread_t thread1,thread2;
    if(pthread_create(&thread1,NULL,part1,p1_) !=0 )
    {
        perror("pthread_create");
        return 0;
    }
    if(pthread_create(&thread2,NULL,part2,p2_) !=0 )
    {
        perror("pthread_create");
        return 0;
    }
    pthread_join(thread1,NULL);
    printf("thread1: %u\n\n",part1_rec);
    pthread_join(thread2,NULL);
    printf("thread2: %u\n\n",part2_rec);
    if ((fp = fopen(argv[1], "wb")) == NULL)
    {
        perror("Creat file failed");
        exit(0);
    }

    unsigned w1 = fwrite(p1_, sizeof(char), part1_rec,fp);
    printf("w1 done\n");
    unsigned w2 = fwrite(p2_, sizeof(char),part2_rec,fp);
    printf("w2 done\n");
    return 0;
}

void *part1(void *buf1)
{
    printf("part1\n");
    struct sockaddr_in     serv_addr,client_addr;
    int                    sock_id,recv_len,clie_addr_len,write_length,ret;
    char *tmp1;
    tmp1 = (char*)buf1;
    unsigned int r_sum = 0;
    unsigned int w_sum = 0 ;
    const char echo[5] = "done";
    if ((sock_id = socket(AF_INET,SOCK_DGRAM,0)) < 0)
    {
        perror("Create socket failed\n");
        exit(0);
    }
    printf("part1:%d\n",sock_id);
    int nRecvBuf = 32 * 1024;
    setsockopt(sock_id,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));

    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port =htons(PORT1);
    serv_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS1);

    if (bind(sock_id,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
    {
        perror("part1 Bind socket faild\n");
        exit(0);
    }
    else
        printf("part1 bind socket succeed\n");
    clie_addr_len = sizeof(client_addr);

    while (recv_len = recvfrom(sock_id, tmp1, MAXLINE, 0,(struct sockaddr *)&client_addr, &clie_addr_len))
    {
        if(recv_len < 0)
        {
            printf("Recieve data from client failed!\n");
            break;
        }
        r_sum += recv_len;
        tmp1 += recv_len;

        //       printf("PART1:\nr_sum= %u \n,r_len =%u\n\n",r_sum,recv_len);
        // printf("part1 rec:%u\n",r_sum);

        //sleep(1);
        if(!sendto(sock_id, echo, 5 ,0,(struct sockaddr *)&client_addr,clie_addr_len))
            printf("echo fail\n");
        if(r_sum >= off_len )break;
        //  printf("part1 echoed\n");
        //sleep(1);
    }
    printf("P1 Finish recieve\n");
    part1_rec  = r_sum;
    close(sock_id);
    pthread_exit(NULL);

}
void *part2(void *buf2)
{
    printf("part2\n");
    struct sockaddr_in     serv_addr2,client_addr2;
    int                    sock_id2,recv_len2,clie_addr_len2,write_length2,ret2;
    char *tmp2;
    tmp2 = (char*)buf2;


    unsigned int r_sum2 = 0;
    unsigned int w_sum2 = 0 ;
    const char echo2[5] = "done";
    int flag = 0;
    if ((sock_id2 = socket(AF_INET,SOCK_DGRAM,0)) < 0)
    {
        perror("part2 Create socket failed\n");
        exit(0);
    }
    printf("part2 : %d\n",sock_id2);
    int nRecvBuf2 = 32 * 1024;
    setsockopt(sock_id2,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf2,sizeof(int));

    memset(&serv_addr2,0,sizeof(serv_addr2));
    serv_addr2.sin_family = AF_INET;
    serv_addr2.sin_port =htons(PORT2);
    serv_addr2.sin_addr.s_addr = inet_addr(IP_ADDRESS2);

    if (bind(sock_id2,(struct sockaddr *)&serv_addr2,sizeof(serv_addr2)) < 0)
    {
        perror("part2 Bind socket faild\n");
        exit(0);
    }
    else
        printf("part2 bind socket succeed\n");
    clie_addr_len2 = sizeof(client_addr2);
    while (recv_len2 = recvfrom(sock_id2, tmp2, MAXLINE, 0,(struct sockaddr *)&client_addr2, &clie_addr_len2))
    {
        if(recv_len2 < MAXLINE)
        {
            if(flag == 1)break;
            else  flag++;
            if(recv_len2 < 0)
            {
                printf("Recieve data from client failed!\n");
                break;
            }

        }
        r_sum2 += recv_len2;
        tmp2 += recv_len2;
        //      printf("r_len = %d , r_sum = %u\n",recv_len,r_sum);
        printf("part2 rec:%u\n",r_sum2);
        //sleep(1);

        if(!sendto(sock_id2, echo2, 5 ,0,(struct sockaddr *)&client_addr2,clie_addr_len2))
            printf("echo2 fail\n");
        printf("part2 echoed");
        //   sleep(1);
    }
    printf("P2 Finish recieve\n");
    part2_rec = r_sum2;
    close(sock_id2);
    pthread_exit(NULL);
}
