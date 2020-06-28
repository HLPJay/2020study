/***********************************************
知识点：
1：结构体字节对齐
2：柔性数组，以及取值方案
3：字节序network和host的转换  ntohs htons
4：netmap通过nmap实现的内存映射
5：poll实现的io多路复用
6：自己实现协议栈时，ping命令以及arp命令起到的作用

**********************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/poll.h>
#define NETMAP_WITH_LIBS

#include <net/netmap_user.h>

#pragma pack(1)


//mac地址只在局域网内通信有用
//链路层  以太网头
#define ETH_LENGTH 6
#define PROTR_IP 0x0800
#define PROTR_UDP 17

struct ethhdr
{
	unsigned char dest[ETH_LENGTH];
	unsigned char src [ETH_LENGTH];
	unsigned short proto;
};

//ip地址怎么理解  有一个位域，网络协议包有4个字节，叫ip地址 ==>为了寻址
//ip头

struct iphdr
{
	//ipv4/ipv6
	unsigned char version:4,
				  hdrlen:4; //首部长度 15 表示有多少个4字节   15*4 = 60

	unsigned char tos;  //8位服务类型 视频？音频？图片？ 
	unsigned short tot_len; //总长度
	unsigned char  id;//标识
	unsigned short flag:3,
					offset :13;
	unsigned char ttl;
	unsigned char proto;
	unsigned short check;
	
	unsigned int sip;//ip地址 有一个位域，网络协议包有4个字节，叫ip地址 ==>为了寻址
	unsigned int dip;
};

struct udphdr{
	unsigned char sport;//没有叫端口，数据包中有个位
	unsigned char dport;
	unsigned char length;
	unsigned char check;
};
//14 20 8  ==44字节
struct udppkg{
	struct ethhdr eh;  //链路层
	struct iphdr ip;   //网络层
	struct udphdr udp; //传输层
	
	unsigned char body[0]; //柔性数组，零长数组 sizeof = 0
};

//网卡数据到操作系统  到应用层
//网卡   协议栈  应用程序
int main()
{
	//
	struct nm_desc *nmr = nm_open("netmap:eth0", NULL, 0 , NULL);
	if(nmr == NULL)
	{
		return -1;
	}
	//如何知道内存中有数据？   多路复用
	//这里只有一个fd，所以用poll
	struct pollfd pfd = {0};
	pfd.fd = nmr->fd;
	pfd.events = POLLIN;
  while(1)
	{
		int ret = poll(&pfd, 1, -1);
		if(ret<0) continue;
		if(pfd.events & POLLIN)
		{
			//已经在内存的数据，不叫读    读，从外设中读到内存中
			struct nm_pkthdr h;
			unsigned char * stream = nm_nextpkt(nmr, &h);
			//判断数据是否是以太网数据
			
			struct ethhdr *eh = (struct ethhdr*)stream;
			//网络字节序network --》host 
			//大小端问题  两个字节以上，考虑转 short 2 4 8字节...
			//ntohs htons
			if(ntohs(eh->proto) == PROTR_IP)//判断是ip头
			{
				struct udppkg *udp = (struct udppkg*)stream;
				if(udp->ip.proto == PROTR_UDP)
				{
					//看一下udp长度
					int udp_length = ntohs(udp->udp.length);
					udp->body[udp_length - 8] = '\0';//总长度减去头长度，body的最终位置
					printf("udp--->%s", udp->body)
				}
			}
		}
	}
	
	return 0;
}

//先测试一下 ping eth0的ip，然后测试
//出现多次send，发送不出去了？
//正在发送的时候去ping，发现ping不通了

//问题1：把eth0所有的数据接管了 ==》没有做arp    mac和ip地址的对应
//问题2：ping实现的是acmp的协议， 我们的程序里没有实现
//arp -a
//ping命令以及arp表的维护
//netstat 看不到的  我们走的不是内核的协议栈
