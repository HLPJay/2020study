#include <stdio.h>
#include <openssl/lhash.h>
#define NAME_LENGTH 32
#define BUff_LENGTH 128
typedef struct SPerson
{
	char name[NAME_LENGTH];
	int high;
	char otherInfo[BUff_LENGTH];
}Person;

//需要对比和内容一样
static int person_cmp(const void *a, const void *b)
{
	char* namea = ((Person*)a)->name;
	char* nameb= ((Person*)a)->name;
	return strcmp(namea,nameb);
}

unsigned long person_key(const void *a)
{
	Person *p =(Person*)a;
	//进行计算可以自己设计key
	long key = 
	return key;
}
static void  print_vlaue(void * value)
{
	Person *p = (Person*)value;
	printf("name :%s \n", p->name);
	printf("high: %d \n", p->high);
	printf("otherinfo : %s \n", p->otherInfo);
}
int main()
{
	//openssl接口     对比函数和哈希函数
	//创建哈希 _LHASH结构体 
	_LHASH *h = lh_new(NULL, person_cmp);
	if(h ==NULL)
	{
		printf("error.");
		return -1;
	}
	Person p1= ={"hhh",175, "hh1"};
	Person p1= ={"yyy",170, "yy1"};
	Person p1= ={"nnn",171, "nn1"};

	//一个一个往哈犀里传
	lh_insert(h, &p1);
	lh_insert(h, &p2);
	lh_insert(h, &p3);

	//遍历
	lh_doall(h,print_vlaue);
	//查找哈希 对应的查找的key person_cmp对比函数要实现 yyy和遍历的对比
	void * data = lh_retrieve(h, (const char*)"yyy");
	if(data ==NULL)
	{
		return -1;
	}
	print_vlaue(data);
	lh_free(h);
	return 0;
}
/********************************
编译的时候要带：
-lssl -lcrypto
哈希在实际的使用中：
	服务器的fd与fd对应的应用数据  user_data 
	通过user_data 找到相关的key fd
存储内容：content
	索引：idx
		形成映射关系，通过hash函数来实现查找
		查找性能O(1)
**********************************/
