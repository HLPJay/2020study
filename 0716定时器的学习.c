
	不推荐AMD的cpu,推荐酷睿。
	8G内存 
	128G SSD
	天猫旗舰店或者京东自营，线下一定要去品牌专卖店
	机械硬盘  固态硬盘(SSD  快)==>高端推荐Intel和三星pro系列
	屏幕1080P以上


	华硕，联想，戴尔（低端不建议买），惠普。

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main( void )
 {
    long i = 10000000L;
    clock_t start, finish;
    double duration;
    printf( "Time to do %ld empty loops is ", i );
    start = clock(); while( i-- ) ;
    finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    printf( "%f seconds \n", duration );
    return 0;
}



#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
/****************************************
struct timeval{ 
　　long tv_sec; //秒 
　　long tv_usec; //微秒 
}; 
struct timezone 
{ 
　　int tz_minuteswest; //和Greenwich 时间差了多少分钟 
　　int tz_dsttime; //日光节约时间的状态 
}; 
****************************************/



typedef unsigned long int uint64_t;
static uint64_t GetCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int main()
{
    uint64_t mytest = 1;
    printf("%lu \n", GetCurrentTime());
    sleep(2);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    printf("second: %ld\n", tv.tv_sec); // 秒
    printf("millisecond: %ld\n", tv.tv_sec * 1000 + tv.tv_usec / 1000); // 毫秒
    printf("microsecond: %ld\n", tv.tv_sec * 1000000 + tv.tv_usec); // 徽秒
    
    sleep(3); // 让程序休眠3秒
    printf("---------------------sleep 3 second-------------------\n");
    
    gettimeofday(&tv, NULL);
        
    printf("second: %ld\n", tv.tv_sec); // 秒
    printf("millisecond: %ld\n", tv.tv_sec * 1000 + tv.tv_usec / 1000); // 毫秒
    printf("microsecond: %ld\n", tv.tv_sec * 1000000 + tv.tv_usec); // 徽秒

    return 0;
}


#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

class Thread {
public:
	Thread();
	virtual ~Thread();
	//线程的实际运行体
	virtual void run() = 0;
	//启动线程
	void Start();
	//停止线程
	void Stop();
	bool IsStop();
private:
	bool stopFlag;
	pthread_t threadId;
};

class Timer : public Thread{
	typedef void(*Timerfunc)(void* p);
    typedef Timerfunc TimerHandler;
public:
	Timer();
	virtual ~Timer();
public:
	//注册回调函数,线程的运行函数,设置超时时间,停止运行函数
	void run();
	void registerHandler(TimerHandler handler, void* p);
	void setInterval(int millisecond); //毫秒级别
	void Stop();  //停止线程的运行

private:
	//回调函数,超时事件,回调参数的保存
	Timerfunc m_func; //回调函数
	int m_interval;   //超时间隔
	void* m_paras;     //回调函数的参数
};


#include "./mythread.h"
typedef unsigned long int uint64_t;

Thread::Thread():stopFlag(false), threadId(-1)
{}

//回调函数,传入自己的指针
static void* ThreadCall(void* p)
{
	printf("ThreadCall start\n");
	Thread * t = (Thread*)p;
	//开始运行线程
	t->run();
	//运行完销毁线程
}

Thread::~Thread()
{
	Stop();
}

void Thread::Start()
{
	printf("Start start\n");
	int err = pthread_create(&threadId, NULL, ThreadCall, this);
	if(err != 0)
	{
		printf("can not create thread \n");
	}
}

void Thread::Stop()
{
	stopFlag = true; //关闭线程正在运行
	//这里不用调用join  不用等线程结束
	// void *res;
	// int ret = pthread_join(threadId, &res);
	// if (0 != ret)
 //        printf("join thread failed\n");
}

bool Thread::IsStop()
{
	return stopFlag;
}
/***************************************************


****************************************************/

Timer::Timer(): m_func(NULL), m_interval(-1),m_paras(NULL)
{}

Timer::~Timer()
{}

//死循环,根据超时时间,执行相对应的回调函数
//获取当前时间,获取变化时间,与时间间隔做对比
//使用gettimeofday函数获取当前时间,精确到毫秒级别
static uint64_t GetCurrentTime()
{
	struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void Timer::run()
{
	printf("run start \n");
	uint64_t nowTime = GetCurrentTime();
	uint64_t startTime = nowTime;
	while(!IsStop())
	{
		nowTime = GetCurrentTime();
		if(nowTime - startTime >= m_interval*1000)
		{
			//执行回调
			(*m_func)(m_paras);
		}

		sleep(1);
	}
	printf("run end \n");
}

void Timer::registerHandler(TimerHandler handler, void* p)
{
	m_func = handler;
	m_paras = p;
}

void Timer::setInterval(int millisecond)
{
	m_interval = millisecond;
}

void Timer::Stop()
{
	Thread::Stop();
}

//需要定义回调函数,需要设置超时时间,需要设置回调函数参数
void callback(void * p)
{
	int * i = (int *)p;
	printf("callback %d\n", *i);
}
int main()
{
	Timer* time = new Timer();
	int i = 0;

	time->registerHandler(&callback, (void*)&i); //注册回调
	time->setInterval(1);   //每隔一秒执行一次
	time->Start();

	for(; i<10; i++)
	{
		sleep(1);
	}
	time->Stop();

	if(time != NULL)
	{
		delete time;
		time = NULL;
	}
	return 0;
}
