#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<signal.h>
#include <time.h>

#define PORT1 8000
#define PORT2 8001
#define IP_ADDRESS1 "127.0.0.1"
#define IP_ADDRESS2 "127.0.0.2"
#define    FINISH_FLAG    "FILE_TRANSPORT_FINISH"
#define    MAXLINE        1024*32 /*max datagram = 64k*/
unsigned int part2_rec = 0;
unsigned int off_len = 1024*1024*512;

const char time_out[10] = "timeout";
int timer_[2];
int sock_id_[2];
struct sockaddr *timer_sock_addr[2];
void *part1(void *filename);
void *part2(void *buf2);
void get_signal();

int main(int argc,char **argv)
{
	time_t t_start,t_end;
	FILE  *fp;
	char *p2_;
	char file_name[10];
	sprintf(file_name,"%s",argv[1]);
	timer_[0] = timer_[1] = 2;
	p2_ = (char*)malloc(1024*1024*520);
	pthread_t thread1,thread2;
	t_start = time(NULL);
	printf("timer_started!\n");   
	if(pthread_create(&thread1,NULL,part1,file_name) !=0 )
	{
		perror("pthread_create");
		return 0;
	}
	if(pthread_create(&thread2,NULL,part2,p2_) !=0 )
	{
		perror("pthread_create");
		return 0;
	}
	sleep(5);
	signal(SIGALRM,get_signal);
	alarm(1);
	pthread_join(thread1,NULL);
	pthread_join(thread2,NULL);
	if ((fp = fopen(argv[1], "ab+")) == NULL)
	{
		perror("Creat file failed");
		exit(0);
	}
	fwrite(p2_, sizeof(char),part2_rec,fp);
	fclose(fp);
	t_end = time(NULL);
	printf("time:%.0f s\n",difftime(t_end,t_start));
	return 0;
}

void *part1(void *filename)
{
	FILE *fp1;
	struct sockaddr_in     serv_addr,client_addr;
	int                    sock_id,recv_len,clie_addr_len,write_length,ret;
	unsigned int r_sum = 0;
	unsigned int w_sum = 0 ;
	const char echo[5] = "done";
	char *file_name;
	char tmp_buf[MAXLINE];
	file_name = (char*)filename;
	fp1 =  fopen(file_name, "wb");
	if ((sock_id = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		perror("Create socket failed\n");
		exit(0);
	}
	printf("part1:%d\n",sock_id);
	sock_id_[0] = sock_id;
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
	timer_sock_addr[0] = (struct sockaddr*)&client_addr;
	while (recv_len = recvfrom(sock_id, tmp_buf, MAXLINE, 0,(struct sockaddr *)&client_addr, &clie_addr_len))
	{
		timer_[0] = 2;
		if(recv_len < 0)
		{
			printf("Recieve data from client failed!\n");
			break;
		}
		r_sum += recv_len;
		fwrite(tmp_buf, sizeof(char), MAXLINE,fp1);
		if(!sendto(sock_id, echo, 5 ,0,(struct sockaddr *)&client_addr,clie_addr_len))
			printf("echo fail\n");
		if(r_sum >= off_len )break;

	}
	printf("P1 done,%u\n",r_sum);
	fclose(fp1);
	close(sock_id);
	pthread_exit(NULL);

}
void *part2(void *buf2)
{
	struct sockaddr_in     serv_addr2,client_addr2;
	int                    sock_id2,recv_len2,clie_addr_len2,write_length2,ret2;
	char *tmp2;
	tmp2 = (char*)buf2;


	unsigned int r_sum2 = 0;
	unsigned int w_sum2 = 0 ;
	const char echo2[5] = "done";

	if ((sock_id2 = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		perror("part2 Create socket failed\n");
		exit(0);
	}
	printf("part2 : %d\n",sock_id2);
	sock_id_[1] = sock_id2;
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
	timer_sock_addr[1] = (struct sockaddr*)&client_addr2;
	while (recv_len2 = recvfrom(sock_id2, tmp2, MAXLINE, 0,(struct sockaddr *)&client_addr2, &clie_addr_len2))
	{
		timer_[1] = 2;
		if(strcmp("finish",tmp2) == 0 )break;
		if(recv_len2 < 0)
		{
			printf("Recieve data from client failed!\n");
			break;
		}
		r_sum2 += recv_len2;
		tmp2 += recv_len2;
		if(!sendto(sock_id2, echo2, 5 ,0,(struct sockaddr *)&client_addr2,clie_addr_len2))
			printf("echo2 fail\n");

	}
	printf("P2 Finish recieve\n");
	part2_rec = r_sum2;
	close(sock_id2);
	pthread_exit(NULL);
}

void get_signal()
{
	int x = 0;
	for(; x < 2; x++)
	{
		if(timer_[x] <= 0)
		{
			if(!sendto(sock_id_[x],time_out, 10 ,0,timer_sock_addr[x],sizeof(struct sockaddr_in)))
				printf("timeout echo fail\n");
		}
		else
			timer_[x]--;
	}
	alarm(1);
}


