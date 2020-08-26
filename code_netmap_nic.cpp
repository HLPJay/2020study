/***********************************
实现对netmap网卡获取数据的封装

***********************************/
#ifndef NTY_NIC_H_
#define NTY_NIC_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/netmap_user.h>

#define NETMAP_WITH_LIBS


#pragma pack(1)

typedef struct _nic_context{
	struct nm_desc * nmr;
	unsigned char send_pkt_body_buff[1514];
	
	unsigned char* recv_pkt_buff[64];
	uint16_t recv_pkt_len[64];

	uint16_t send_pkt_size;
	uint8_t dev_poll_flag;
	uint8_t idle_poll_count; 
}nic_context;
#pragma pack()
#endif

#include "nty_nic.h"

/**************************
初始化   写  读
申请内存 初始化netmap对应网卡

*****************************/

int nty_nic_init(nic_context *ctx, const char* ifname)
{
	if(ctx!= NULL) return ;
	ctx = (nic_context *)calloc(1, sizeof(nic_context));
	if(ctx ==NULL)
	{
		return -1;
	}
	struct nmreq req;
	memset(&req, 0, sizeof(struct nmreq));
	req.nr_arg3 = EXTRA_BUFS;

	ctx->nmr = nm_open(ifname, &req, 0, NULL);
	if(ctx->nmr == NULL)
		return -2;
	return 0;
}


int nty_nic_read(nic_context *ctx, unsigned char **stream)
{
	if(ctx == NULL)
	{
		return -1;
	}
	struct nm_pkthdr h;
	*stream = nm_nextpkt(ctx->nmr, &h);
	return 0;
}


int nty_nic_write(nic_context *ctx, const void* stream, int len)
{
	if(ctx == NULL) return -1;
	if(stream == NULL) return -2;
	if(len == 0) return 0;

	nm_inject(ctx->nmr, stream, len);
	return 0;
}


unsigned char *nty_nic_get_wbuffer(nic_context *ctx, int nif, uint16_t pktsize)
{
	ctx->send_pkt_size = pktsize;
	return (uint8_t*) ctx->send_pkt_body_buff ;
}

unsigned char* nty_nic_get_rbuffer(nic_context *ctx, int nif, uint16_t *len)
{
	*len = ctx->recv_pkt_len[nif];
	return ctx->recv_pkt_buff[nif];
}

int nty_nic_send_pkts(nic_context *ctx, int nif)
{
	if(ctx->send_pkt_size == 0) return -1;
tx_again:
	if(nm_inject(ctx->nmr, ctx->send_pkt_body_buff, ctx->send_pkt_size) == 0)
	{
		printf("Failed to send pkt of size %d on interface: %d\n",
			  ctx->send_pkt_size, nif);
		ioctl(ctx->nmr->fd, NIOCTXSYNC, NULL);
		goto tx_again;
	}
	ctx->send_pkt_size = 0;
	return 0;
}

//把数据从netmap中取到对应的结构体数组中   buff的长度再对应的数组中
int nty_nic_recv_pkts(nic_context *ctx, int nif)
{
	assert(ctx != NULL);
	int n = ctx->nmr->last_rx_ring - ctx->nmr->first_rx_ring + 1;
	int i = 0, r = ctx->nmr->cur_rx_ring, got = 0, count = 0;

	for (i = 0;i < n && ctx->dev_poll_flag;i ++) {
		struct netmap_ring *ring;

		r = ctx->nmr->cur_rx_ring + i;
		if (r > ctx->nmr->last_rx_ring) r = ctx->nmr->first_rx_ring;

		ring = NETMAP_RXRING(ctx->nmr->nifp, r);
		
		for ( ;!nm_ring_empty(ring) && i != got; got ++) {
			
			int idx = ring->slot[ring->cur].buf_idx;
			ctx->rcv_pktbuf[count] = (unsigned char*)NETMAP_BUF(ring, idx);
			
			ctx->rcv_pkt_len[count] = ring->slot[ring->cur].len;
			ring->head = ring->cur = nm_ring_next(ring, ring->cur);

			count ++;
		}
	}

	ctx->nmr->cur_rx_ring = r;
	ctx->dev_poll_flag = 0;

	return count;
}


//通过变量的方式来实现poll的识别
int nty_nic_select(nic_context *ctx)
{
	int rc = 0;
	struct pollfd pfd = {0};
	pfd.fd = ctx->nmr->fd;
	pfd.events = POLLIN;

	if(ctx->idle_poll_count >= 10)
	{
		rc = poll(&pfd, 1, 1);//第二个参数是数量 第三个参数是阻塞的时间
	}else
	{
		rc = poll(&pfd, 1, 0);
	}

	ctx->idle_poll_count = (rc ==0) ? ctx->idle_poll_count+1:0;
	if(!(pfd.revents & POLLERR))
			ctx->dev_poll_flag = 1;
	return rc;
}
