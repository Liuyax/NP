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

#define    FINISH_FLAG    "FILE_TRANSPORT_FINISH"
#define    MAXLINE        1024*16

unsigned int send_sum = 0;
char echo[5];
//union echo{
//   char ch[4];
//   int num;
//};

void usage(char *command)
{
    printf("usage :%s ipaddr portnum filename\n", command);
    exit(0);
}

int main(int argc,char **argv)
{
//    union echo echo_;
    FILE                   *fp;
    struct sockaddr_in     server_addr;
    char                   buf[MAXLINE];
    int                    sock_id,read_len,send_len,server_addr_len,ret,i;

    if (argc != 4)
    {
        usage(argv[0]);
    }
    if ((fp = fopen(argv[3],"rb")) == NULL)
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
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    server_addr_len = sizeof(server_addr);

    ret = connect(sock_id, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    if (ret == -1)
    {
        perror("Connect socket failed!\n");
        exit(0);
    }
    bzero(buf, MAXLINE);
    server_addr_len = sizeof(server_addr);
    while (read_len = fread(buf, 1, MAXLINE, fp),read_len)
    {
        send_len = sendto(sock_id, buf, read_len, 0,(struct sockaddr *)&server_addr,server_addr_len);
//		send_sum += send_len;
//        printf("send_num = %d\n send_sum = %u\n",read_len,send_sum);
        if ( send_len < 0 )
        {
            perror("Send data failed\n");
            exit(0);
        }
        bzero(buf, MAXLINE);
        recvfrom(sock_id, echo, 5, 0,(struct sockaddr *)&server_addr, &server_addr_len);
        //  printf("%s\n",echo);
    }
    if(feof(fp)==0)
    {
        sprintf(buf,"error");
        send(sock_id,buf,6,0);
    }

    fclose(fp);
    memset(buf,0,10);
    sprintf(buf,"finish");
    send(sock_id,buf,7,0);
    close(sock_id);
    printf("Send finish\n");
    return 0;
}
