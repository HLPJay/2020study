/*******************************************
定义tcp协议栈，实现tcp通信
第一：定义特定的用户数据，进行解析。  socket自带的tcp接口
第二：定义协议栈，UDP，TCP从网卡接收到数据，直接解析。
第三：基于socket，对其进行封装使用。

********************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_pton

struct protoDada{
	int type;
	int length;
	char body[0];
};

int ClientMain()
{
	//创建socket   连接对端   发送数据   接收数据
	int cli_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(cli_sock_fd == -1)
	{
		printf("ERROR create cli socket error. \n");
		close(socket_fd);
		return -1;
	}
	//去连接
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	const char* ip = "0.0.0.0";
	addr.sin_family = AF_INET;
	addr.sin_port =  htons((short)7000);
	inet_pton(AF_INET, ip, &addr.sin_addr);

	while(true)
	{
		if(connect(cli_sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
		{
			printf("ERROR connect server socket error. \n");
			return -1;
		}

		char sendData[] = "client Data test.";
		send(cli_sock_fd, sendData, strlen(sendData),0);
		
		char recvData[255];
		int ret = recv(cli_sock_fd, recvData, 255, 0);
		if(ret>0)
		{
			recvData[ret] = '\0';
			printf("recvData: %s\n", recvData);
		}
		
		usleep(1000);
	}
	close(cli_sock_fd);
	return 0;
}



int ServerMain()
{	
	//定义socket 绑定端口， 开始监听， accept等待连接， 负责接收， 返回结果
	int socket_fd = socket(AF_INET, SOCK_STREAM , 0);
	if(socket_fd == -1)
	{
		printf("ERROR: create server socket error. \n");
		return -1;
	}
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET; //ipv4
	addr.sin_port = htons((short)7000);
	inet_pton(AF_INET, ip, &addr.sin_addr);//ip转换 反转用inet_ntop
	
	//如果要设置端口重用，就用setsockopt，SO_REUSEADDR==》远端地址其实不同  同一个socket绑定多个地址
	int opt = 1;
	if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		printf("ERROR: setsocketopt error\n");
	}

	if(bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		printf("ERROR: bind address error \n")
		close(socket_fd);
		return -1;
	}
  
  
	if(listen(socket_fd, 1024) == -1)
	{
		printf("ERROR: listen server socket error \n");
		close(socket_fd);
		return -1;
	}
	while(true)
	{
		int client_fd;
		int len = sizeof(addr);
		client_fd = accept(socket_fd, (struct sockaddr *)&addr, &len);
		if(client_fd == -1)
		{
			printf("ERROR:accept error -1 \n");
			break;
		}
		char recvData[255] = {0};
		int ret = recv(client_fd, recvData, 255, 0);
		if(ret>0)
		{
			recv[ret] = 0x00;
			printf("recv: %s \n", recvData);
		}

		char* sendData = "hello client \0";//char sendData[] = "hello client";
		send(client_fd, sendData, strlen(sendData),0);
	}
	close(socket_fd);
	return 0;
}
