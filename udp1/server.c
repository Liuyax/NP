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
#define    MAXLINE        1024*16 /*max datagram = 64k*/
unsigned int r_sum = 0;
unsigned int w_sum = 0 ;
const char echo[5] = "done";
//int echo_ = 0;

void usage(char *command)
{
    printf("usage :%s portnum filename\n", command);
    exit(0);
}
int main(int argc,char **argv)
{
    struct sockaddr_in     serv_addr,client_addr;
    char                   buf[MAXLINE];
    int                    sock_id,recv_len,clie_addr_len,write_length,ret;
    FILE                   *fp;

    if (argc != 3)
    {
        usage(argv[0]);
    }
    if ((fp = fopen(argv[2], "wb")) == NULL)
    {
        perror("Creat file failed");
        exit(0);
    }
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
    bzero(buf, MAXLINE);
    while (recv_len = recvfrom(sock_id, buf, MAXLINE, 0,(struct sockaddr *)&client_addr, &clie_addr_len))
    {
//        r_sum += recv_len;
  //      printf("r_len = %d , r_sum = %u\n",recv_len,r_sum);
        if(recv_len < 0)
        {
            printf("Recieve data from client failed!\n");
            break;
        }
        if(strcmp(buf,"error")==0)
        {
            printf("error\n");
            break;
        }
        if(strcmp(buf,"finish")==0)
        {
            printf("finish\n");
            break;
        }
        write_length = fwrite(buf, sizeof(char), recv_len, fp);
        if (write_length < recv_len)
        {
            printf("File write failed\n");
            break;
        }

        if(!sendto(sock_id, echo, 5 ,0,(struct sockaddr *)&client_addr,clie_addr_len))
            printf("echo fail\n");

//        w_sum += write_length ;
//        printf("w_sum = %u\n",w_sum);

        bzero(buf, MAXLINE);
    }

    printf("Finish recieve\n");
    fclose(fp);
    close(sock_id);
    return 0;
}
