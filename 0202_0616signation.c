#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

//信号可以协助我们实现Proactor    
//比较老，用的比较少
//Ctrl+c就是信号
//signal ---》不可控的原因系统崩溃，可以用信号去实现清除

//yes ==>async 是异步的操作 ==》但是为啥不常用？
int sockfd;
//通过信号触发，当io有数据时会触发到
void do_sigio(int sig)//参数是信号的值 kill -9 收到是9
{
	char buffer[256] = {0};
	struct sockaddr_in cliaddr;
	
	int clilen = sizeof(struct sockaddr_in);
	int len = recvfrom(sockfd,buffer,256,0,(struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
	printf("message : %s \n", buffer);
	int slen = sendto(sockfd,buffer, len, 0,(struct sockaddr*)&cliaddr, clilen);
}

int main()
{
	struct sigaction sigio_action;
	sigio_action.sa_flags = 0;
	sigio_action.sa_handler = do_sigio;
	sigaction(SIGIO, &sigio_action, NULL);//SIGIO 信号
	
	//操作系统  先检查是否进行运行 进程在运行的时候捕获到信号 然后执行
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(6016);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	
	bind(sockfd, (struct sockaddr*)&serv_addr,sizeof(struct sockaddr));
	fcntl(sockfd, F_SETOWN, getpid());//操作fd设置阻塞非阻塞 属于哪个进程
	
	int flag = fcntl(sockfd, F_GETFL, 0);
	flag | = O_ASYNC| O_NONBLOCK;
	fcntl(sockfd,F_SETFL, flag);
	
	//这里while sleep会切换出去
	while(1)
	{
		sleep(1);
	}
	close(sockfd);
	return 0;
}
