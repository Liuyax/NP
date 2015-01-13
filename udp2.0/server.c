#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define    FINISH_FLAG    "FILE_TRANSPORT_FINISH"
#define    MAXLINE        1024*32 /*max datagram = 64k*/
unsigned int r_sum = 0;
unsigned int w_sum = 0 ;
const char echo[5] = "done";

unsigned int off_len = 1024*1024*512;


void usage(char *command)
{
    printf("usage :%s portnum filename\n", command);
    exit(0);
}
int main(int argc,char **argv)
{
    char *p1,*p2,*tmp1,*tmp2;
    p1 = (char*)malloc(1024*1024*600);
    p2 = (char*)malloc(sizeof(char)*1024*1024*800);
    tmp1 = p1;
    tmp2 = p2;
    struct sockaddr_in     serv_addr,client_addr;
    int                    sock_id,recv_len,clie_addr_len,write_length,ret;
    FILE                   *fp;
    if (argc != 3)
    {
        usage(argv[0]);
    }
    int pid = fork();
    if(pid < 0)
    {
        perror("fork failed\n");
        exit(0);
    }
 else if(pid)
    {
        if ((sock_id = socket(AF_INET,SOCK_DGRAM,0)) < 0)
        {
            perror("Create socket failed\n");
            exit(0);
        }

        int nRecvBuf = 32 * 1024;
        setsockopt(sock_id,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));

        memset(&serv_addr,0,sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(atoi(argv[1])+1);
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(sock_id,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        {
            perror("Bind socket faild\n");
            exit(0);
        }
        clie_addr_len = sizeof(client_addr);
        while (recv_len = recvfrom(sock_id, tmp2, MAXLINE, 0,(struct sockaddr *)&client_addr, &clie_addr_len))
        {
            if(recv_len < 10)
            {
                if(recv_len < 0)printf("Recieve data from client failed!\n");
                break;
            }
            r_sum += recv_len;
            tmp2 += recv_len;
            //      printf("r_len = %d , r_sum = %u\n",recv_len,r_sum);


          //   printf("PART2:\nr_sum= %u \n,r_len =%u\n\n",r_sum,recv_len);
            if(!sendto(sock_id, echo, 5 ,0,(struct sockaddr *)&client_addr,clie_addr_len))
                printf("echo fail\n");
        }
        printf("P2 Finish recieve\n");
        printf("p2: waiting p1 finish\n");
        wait(NULL);
        printf("p2: start writing\n");
        fp = fopen(argv[2], "ab+");
        unsigned w1 = fwrite(p2, sizeof(char), r_sum, fp);
      //  fflush(fp);
        printf("P2 Finish write: %u \n",w1);
        fclose(fp);
        close(sock_id);
    }

    else
    {

        if ((sock_id = socket(AF_INET,SOCK_DGRAM,0)) < 0)
        {
            perror("Create socket failed\n");
            exit(0);
        }
        int nRecvBuf = 32 * 1024;
        setsockopt(sock_id,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));

        memset(&serv_addr,0,sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(atoi(argv[1]));
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(sock_id,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        {
            perror("Bind socket faild\n");
            exit(0);
        }
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

            if(r_sum == off_len )break;
//            sleep(2);
           if(!sendto(sock_id, echo, 5 ,0,(struct sockaddr *)&client_addr,clie_addr_len))
              printf("echo fail\n");
        }
        printf("P1 Finish recieve\n");
         printf("PART1:\nr_sum= %u \n\n",r_sum);
     if ((fp = fopen(argv[2], "wb")) == NULL)
        {
            perror("Creat file failed");
            exit(0);
        }
          unsigned w1 = fwrite(p1, sizeof(char), r_sum, fp);
        //fflush(fp);

        printf("P1 Finish write:%u\n",w1);
        fclose(fp);
        close(sock_id);
        exit(EXIT_SUCCESS);
    }

    return 0;
}
