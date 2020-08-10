/*************************************************************
内存池：
主要应用在客户端服务端，频繁的malloc/free的场景
      ---》会导致内存碎片比较多
如何实现内存池：
      --》内存池是一个组件，做出来后，提供api给别人使用。
      0：通过宏定义处理
      1：定义结构体：基础数据结构
      2：对外提供的api
      3：测试函数
内存池的结构体定义：内存中1页是4k
      1：小块内存： 小于4K
      2：大块内存： 大于4K
可参考：
  tcmalloc    jemalloc   nginx(ngx_palloc.c / ngx_palloc.h)
*************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>

//定义节点
struct mp_node_s  //小块内存 4k
{
    unsigned char* start;
    unsigned char* end;

    struct mp_node_s *next;
    int flag; //标志是否使用 每一次小块的malloc就+1
};
struct mp_large_s { //大块内存
    struct mp_large_s *next;
    void * alloc;
};
//end -start >size 确定柔性数组 不越界
typedef struct mp_pool{
    size_t max;
    struct mp_node_s *current;
    struct mp_large_s *large;
    //柔性数组 标签 不占内存 内存提前分配好，后面长度不确定的时候使用
    struct mp_node_s head[0]; //小块内存已经分配，但是长度不确定
}POOL;

//create pool,init
#define MP_ALIGNMENT 32
#define MAX_ALLOC_BLOCK 4096

// #define posix_memalign(p, a, s) (((*(p)) = _aligned_malloc((s), (a))), *(p) ?0 :errno)
/******************
 第一个节点多存一个pool
 * ***************/
POOL* mp_create_pool(size_t size)
{
    POOL*p;

//    malloc(10*1024) //会分配失败
    int ret = posix_memalign((void **)&p, MP_ALIGNMENT,size+sizeof(POOL) +sizeof(struct mp_node_s) );//分配大块内存
    if(ret)
    {
        return NULL;
    }
     //小块允许最大的值MAX_ALLOC_BLOCK
    p->max = size<MAX_ALLOC_BLOCK ? size:MAX_ALLOC_BLOCK ;
    //这里要使用柔性数组
    p->current = p->head;

    p->large = NULL;
    p->head->start = (unsigned char *)p +sizeof(POOL)+sizeof(struct mp_node_s);
    p->head->end =p->head->start +size;
    p->head->flag =0;

    return  p;
}

//大块的分配    要提升效率  我们分配时候 以页4k为单位申请或者分配
//内存池不限制
//mmap --->把外设/文件映射在内存
void mp_destory_pool(POOL *pool)
{
    //释放大块
    struct mp_large_s *l;
    for(l = pool->large; l!=NULL; l=l->next)
    {
        if(l->alloc)
        {
            free(l->alloc);
        }
    }
    //释放小块
    struct mp_node_s *n, *h;
    h = pool->head->next;
    while(h)
    {
        n=h->next;
        free(h);
        h=n;
    }
    free(pool);
    return ;
}

/*********************
void *p = plloc(pool,100)
1: size < pool->max
    1:判断当前mp_node_s
    2:遍历mp_node_s链表
    3:到mp_node_s结尾，malloc重新分配4K
2：alloc_large()
 * ********************/
/*********************************
 1:需要分配mp_node_s 4k
 2:新分配的内存加入链表中p->next
 3：从分配好的内存中取内存给用户
 * ******************************/
 void* mp_alloc(POOL *pool, size_t size);
static void *mp_alloc_block(struct mp_pool *pool,  size_t size)
{
    //
    struct mp_node_s *h = pool->head;
    size_t psize = (size_t)((h->end) -(unsigned char *) h);

    unsigned char *m= NULL;//这里分配的是内存块
    int ret = posix_memalign((void **)&m,MP_ALIGNMENT ,psize);
    if(ret)
    {
        return NULL;
    }
    //直接加到链表头
    struct mp_node_s *n = pool->current;
    struct mp_node_s *new_node = (struct mp_node_s*)m;
    new_node->next = pool->current;

    pool->current=new_node;
    //TODO ? 这里的head有什么作用？
    // pool->head = m +(int)sizeof(struct mp_node_s);

    new_node->start = m+(int)sizeof(struct mp_node_s);
    new_node->end =new_node->start +psize;
    return  new_node->start;//m +sizeof(struct mp_node_s);
}
static void *mp_alloc_large(struct mp_pool *pool,  size_t size)
{
    void *p = NULL;//大块内存都是申请完释放的
    int ret = posix_memalign((void **)&p, MP_ALIGNMENT, size);
    if(ret)
    {
        return NULL;
    }

    struct mp_large_s *large;
    for(large = pool->large; large; large=large->next)
    {
        if(large->alloc == NULL)//遍历，有则使用，没有则分配
        {
        	large->alloc = p;
        	return p;
        }
    }
    //存的是大块内存的地址
    large = (struct mp_large_s *)mp_alloc(pool, sizeof(struct mp_large_s));
    if (large == NULL) {
		free(p);
		return NULL;
	}

	large->alloc = p;
	//插入头节点
	large->next = pool->large;
	pool->large = large;

	return p;
}

#define mp_align_ptr(p, alignment) (void *)((((size_t)p)+(alignment-1)) & ~(alignment-1))


void* mp_alloc(POOL *pool, size_t size)
{
    if(size<= pool->max)//分配小块
    {
        struct mp_node_s *p = pool->current;
        do{
            unsigned char* m = p->start ;
            if((size_t)(p->end -(p->start)) >= (int)size)//判断可用空间
            {
                p->start += size;
                return m;
            }
          p=p->next;
        }while(p);

        return mp_alloc_block(pool, size);
    }

    return mp_alloc_large(pool, size);
}

void mp_free(POOL *pool, void *p) {

	struct mp_large_s *l;
	for (l = pool->large; l; l = l->next)
	{
		if (p == l->alloc) 
		{
			free(l->alloc);
			l->alloc = NULL;

			return ;
		}
	}
	
}
void *mp_calloc(POOL *pool, size_t size) {

	void *p = mp_alloc(pool, size);
	if (p) {
		memset(p, 0, size);
	}

	return p;
	
}

int main()
{
	int size = 1 << 12;
	printf("create pool size = %d.\n",size);
    POOL* pool =  mp_create_pool(size);
    for(int i=0; i<10; i++)
    {
    	void * p = mp_alloc(pool, 100);
    	//小块内存不释放，大块内存释放==》最后释放的
    	//其实小块内存应该也释放的
    }

    int j = 0;
	for (int i = 0;i < 5;i ++) {

		char *pp = mp_calloc(pool, 32);
		for (j = 0;j < 32;j ++) {
			if (pp[j]) {
				printf("calloc wrong\n");
			}
			printf("calloc success\n");
		}
	}

	for (int i = 0;i < 5;i ++) {
		void *l = mp_alloc(pool, 8192);
		mp_free(pool, l);
	}
	for (int i = 0;i < 58;i ++) {
		mp_alloc(pool, 256);
	}
	mp_destory_pool(pool);
    return 0;
}
