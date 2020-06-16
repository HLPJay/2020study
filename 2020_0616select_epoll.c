#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

//select 
#define BUFFER_LEN 1024
#define EPOLL_SIZE 1024
int main()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd<0) return -1;

	int port = 6016;
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(6016);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	//bind在端口被占用的情况会失败
	if(bind(sockfd, (struct sockaddr*)&serv_addr,sizeof(struct sockaddr))<0)
	{
		perror("bind");
		return -2;
	}
if(listen(sockfd, 5) <0)
	{
		perror("listen");
		return -3;
	}
	//server info end

#if 0
	//fd 0  1 2 是系统占用 其他都是int从3开始 
	//标记是否可读可写 检测io是否可读写
	fd_set rfds,rset;
	FD_ZERO(&rfds);//这个集合全部置0
	FD_SET(sockfd, &rfds);//置1


	int maxfd = sockfd+1;
	while(1)
	{	
		rset = rfds;
		int nready = select(maxfd, &rset, NULL, NULL, NULL) //监控这些最大fd 可读 可写 出错 时间（为NULL且前面无可读，一直阻塞）
		if(nready <0) continue;//超时的时候返回-1
		if(FD_ISSET(sockfd, &rset)) //这个fd是否设置到set中 connect
		{
			struct sockaddr_in clientaddr;
			socklen_t client_len = sizeof(clientaddr);
			int clientfd = accpet(sockfd, (struct sockaddr*)&clientaddr, client_len);
			if(clientfd<=0)
				continue;
			FD_SET(clientfd, &rfds);
			
			if(clientfd >maxfd)
			{
				maxfd = clientfd;
			}
			printf("sockfd %d, maxfd:%d, clientfd: %d \n", sockfd, maxfd, clientfd);
			if(--nready == 0)
				continue;
		}
    
    //所有sockfd都是int型且依次增加
		int i=0;
		for(i = sockfd+1; i<maxfd; i++)
		{
			//因为有clr，重新计算maxfd
			if(FD_ISSET(i, &rset))
			{
				char buffer[BUFFER_LEN] = {0};
				int ret = recv(i, buffer. BUFFER_LEN, 0);
				//每一个socket 对应一个tcb --》有sendbuf和recvbuf --》网卡
				//可写判断 sendbuf是否有空余的地方 可读判断recvbuf是否有数据
				if(ret<0)
				{
					if(errno == EAGAIN ||errno = EWOULDBLOCK)//多线程操作可能别的已经读取了
					{
						printf("read all Data");
					}else{
						FD_CLR(i, &rfds);
						close(i);
					}	
				}else if(ret ==0)//断开 收到对方fin
				{
					printf("disconnect %d", i);
					FD_CLR(i, &rfds);
					close(i);
				}else{
					printf("RECV :%s, %d bytes \n", buffer, ret);
				}
				
				if(--nready == 0)
					break;
			}
		}
	}
#else
	//select相当于 循环去每个敲门   epoll相当于小区蜂巢
	//epoll通过红黑树组织所有的io  有io把io加到就绪队列（蜂巢）
	//队列的节点在红黑树里，也在队列里
	//epoll_create()  epoll_ctl()  epoll_wait()
  int epfd = epoll_create(1); //size参数只有大于0和小于0的区别，没有意义 ==》最初有意义
	struct epoll_event ev, events[EPOLL_SIZE] = {0};
	ev.events = EPOLLIN;
	ev.data.fd =sockfd;//用于epoll_wait时传出来
	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);//根节点 sockefd当key用 
	while(1)
	{
		int nready = epoll_wait(epfd, events, EPOLL_SIZE, -1);
		if(nready == -1)
		{
			continue;
		}
		int i =0 ;
		for(i = 0; i<nready; i++)//events是就绪队列
		{
			
			if(events[i].data.fd == sockfd)
			{
				struct sockaddr_in clientaddr;
				socklen_t client_len = sizeof(clientaddr);
				int clientfd = accpet(sockfd, (struct sockaddr*)&clientaddr, client_len);
				if(clientfd<=0)
					continue;
				
				ev.events = EPOLLIN | EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
			}else{
				int clientfd = events[i].data.fd;
				char buffer[BUFFER_LEN] = {0};
				int ret = recv(i, buffer. BUFFER_LEN, 0);
				if(ret<0)
				{
					if(errno == EAGAIN ||errno = EWOULDBLOCK)//多线程操作可能别的已经读取了
					{
						printf("read all Data");
					}else{
						//调用send前要设置为out
					}	
				}else if(ret ==0)//断开 收到对方fin
				{
					printf("disconnect %d", i);
				}else{
					printf("RECV :%s, %d bytes \n", buffer, ret);
				}
			}
		}
	}
	
#endif

}
