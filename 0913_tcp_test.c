void my_error(const char* err_string, int line)
{
	std::cout<<"line:"<<line<<std::endl;
	perror(err_string);
	exit(1);
}


/************************
实现tcpsocket的封装：
思考：
	创建socket，绑定端口，开始监听，等待连接，接收数据，处理数据

//是通过perror把标准错误输出到客户端
************************/

#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include<iostream>
#include<cstring>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/signal.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<errno.h>

class TCPServerSocket{
public:
	TCPServerSocket();
	~TCPServerSocket();

public:
	//TCP相关的接口 接收连接，关闭连接，接收数据，处理数据
	void AcceptClient();
	bool ServerRecv(int conn_fd);
	void CloseClient(int i);

	//服务端的触发
	void Run();
	//数据的处理
	bool DealWithRecvPacket(int conn_fd, unsigned char* recv_data, uint16_t opt_type, int data_size);
private:
	int server_sock_fd;
	int cli_conn_fd;
	int epollfd;

	int cli_len; //accept的第三个参数

	struct epoll_event  event;
	struct epoll_event*  events;
	struct sockaddr_in server_addr;
	struct sockaddr_in cli_addr;
};
class TCPClientSocket{
public:
	TCPClientSocket();
	~TCPClientSocket();

	//客户端负责连接，发送，和接收
	bool ConnectServer();
	bool SendToServer();

	void run();
private:
	int cli_fd;
	struct sockaddr_in server_addr;
};

#endif //TCP_SOCKET_H










#include "TCPSocket.h"

#define PORT 7000
#define LISTEN_SIZE 1023
#define EPOLL_SIZE 1023
//创建服务器socket
TCPServerSocket::TCPServerSocket()
{
	server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_sock_fd < 0)
	{
		my_error("create server socket error.",__LINE__);
	}
	//设置套接字可以重新绑定端口
	int opt =1;//so_reuseaddr
	if(setsockopt(server_sock_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(int))<0)
	{
		my_error("setsockopt error.", __LINE__);
	}

	memset(server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//绑定端口
	if(bind(server_sock_fd, (struct sockaddr*)server_addr, sizeof(struct sockaddr))<0)
	{
		my_error("bind error.", __LINE__);
	}

	//开始监听端口
	if(listen(server_sock_fd, 1023)<0)
	{
		my_error("listen error.", __LINE__);
	}

//加入epoll中，使用epoll来监听所有的fd，服务端的fd进行监听accept，客户端的fd进行recv
	cli_len = sizeof(struct sockaddr_in);
	events = (struct epoll_event*)malloc(sizeof(struct epoll_event)*EPOLL_SIZE);

	epollfd = epoll_create(EPOLL_SIZE);
	if(epollfd == -1)
	{
		my_error("create epoll error.",__LINE__);
	}

	//epoll_ctl有四个参数  epollfd，增删， sockfd， event
	event.events = POLLIN;
	event.data.id = server_sock_fd;
	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, server_sock_fd, &ev))
	{
		my_error("epoll ctl error",__LINE__);
	}
}


TCPServerSocket::~TCPServerSocket()
{
	close(server_sock_fd);
	free(events);
	std::cout<<"服务器TCP退出成功"<<std::endl;
}

//TCP相关的接口 接收连接，关闭连接，接收数据，处理数据
void TCPServerSocket::AcceptClient()
{
	//fd接收到客户端的地址和fd数据
	cli_conn_fd = accept(server_sock_fd, (struct sockaddr*)&cli_addr,sizeof(struct sockaddr_in));
	if(cli_conn_fd<0)
	{
		my_error("accept error", __LINE__);
	}
	//把接收到的fd放入epoll中
	event.events = EPOLLIN|EPOLLRDHUP;
	event.data.id = cli_conn_fd;
	if(epoll_ctl(epollfd, EPOLL_CTL_ADD, cli_conn_fd, & event)<0)
	{
		my_error("client to epoll error", __LINE__);
	}
	std::cout<<"a connect is connected, ip is"<<inet_ntoa(cli_addr.sin_addr)<<std::endl;
}
/*
struct NetPacketHeader
{
    unsigned short      wDataSize;  ///< 数据包大小，包含封包头和封包数据大小
    unsigned short      wOpcode;    ///< 操作码
};
*/
bool TCPServerSocket::ServerRecv(int conn_fd)
{
	unsigned char recv_buff[10000];
	memset(recv_buff, 0 , 10000);
	int recv_size = 0;

	int header_sum_size = 0;
	int header_data_sum_size = 0;
	int data_sum_size = 0;
	//安全，多次得接收到结构体头
	while(header_sum_size != sizeof(NetPacketHeader))
	{
		recv_size = recv(cli_conn_fd, recv_buff+header_sum_size, sizeof(NetPacketHeader) - header_sum_size, 0);
		if(recv_size == 0)
		{
			cout<<"从客户端接收头数据失败 。"<<std::endl;
			return false;
		}
		header_sum_size+=recv_size;
	}
	NetPacketHeader *phead = (NetPacketHeader*)recv_buff;
	header_data_sum_size = phead->wDataSize;
	data_sum_size = header_data_sum_size-sizeof(NetPacketHeader);

	//获取到数据区数据的大小，对数据进行接收
	while(header_sum_size != header_data_sum_size)
	{
		recv_size = recv(cli_conn_fd, recv_buff+header_sum_size, sizeof(NetPacketHeader) - header_sum_size, 0);
		if(recv_size == 0)
		{
			cout<<"从客户端接收头数据失败 。"<<std::endl;
			return false;
		}
		header_sum_size+=recv_size;
	}
//把recvbuff偏移头文件的长度
	DealWithRecvPacket(cli_conn_fd, (unsigned char*)(phead+1), phead->wOpcode, data_sum_size);
	std::cout<<"recv_buff:"<<recv_buff<<endl;
}

void TCPServerSocket::CloseClient(int i)
{
	//从epoll中删除，关闭该链接
	cout<<"a connet is quit,ip is "<<inet_ntoa(cli_addr.sin_addr)<<endl;
	epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, &event);
	close(events[i].data.fd);
}

//服务端的触发
void TCPServerSocket::Run()
{
	while(1)
	{
		int Ready_num = 0;
		//epoll的fd，事件集合，大小，超时时间
		Ready_num = epoll_wait(epollfd, events, EPOLL_SIZE, -1);
		for(int i=0; i<Ready_num; i++)
		{
			//负责接收
			if(events[i].data.fd = server_sock_fd)
			{
				AcceptClient();
			}else if(events[i].events & EPOLLIN){
				ServerRecv(events[i].data.fd);
			}

			if(events[i].events & EPOLLRDHUP)
			{
				closeClient(i);
			}
		}
	}
}

//数据的处理 先解析文件名  再解析文件内容
//应该设计为文件名，文件总大小，文件大小，文件内容，编号
bool TCPServerSocket::DealWithRecvPacket(int conn_fd, unsigned char* recv_data, uint16_t opt_type, int data_size)
{
	if(opt_type = 1)
	{
		std::cout<<recv_data<<std::endl;
	}
}



// int main()
// {

// 	TCPServerSocket tcpServer;
// 	tcpServer.run();
// 	return 0;
// }


#ifndef PORT
#define PORT 7000
#endif

//创建socket 
TCPClientSocket::TCPClientSocket(int argc, char** argv)
{
    //根据传入的参数进行解析ip
    if(argc!=3)
    {
    	cout<<"Usgae [-a] [serv_addr]"<<endl;
    	exit(1);
    	// return ;
    }

    //解析定义的服务端的结构
    memset(&server_addr, 0 , sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    for(int i=0; i<argc; i++)
    {
    	if(strcmp("-a", argv[i]) == 0)
    	{
    		if(inet_aton(argv[i+1],&server_addr.sin_addr) == 0)
    		{
    			cout<<"invaild server ip address success"<<endl;
    			exit(1);
    		}
    		break;
    	}
    }
    //创建socket
    cli_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(cli_fd<0)
    {
    	my_error("create socket error", __LINE__);
    }
}

TCPClientSocket::~TCPClientSocket()
{
	close(cli_fd);
}
//客户端负责连接，发送，和接收
bool TCPClientSocket::ConnectServer()
{
	if(connet(cli_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr))<0)
	{
		my_error("connect server error", __LINE__);
		return false;
	}
	return true;
}
bool TCPClientSocket::SendToServer()
{
	//构造相关的报文，使用send进行发送
	//如果要发送文件呢？
}

void TCPClientSocket::run()
{
	if(ConnectServer())
	{
		SendToServer();
	}
}


int main(int argc, char ** argv)
{
	TCPClientSocket client(argc, argv);
	client.run();
	sleep(10);
}
