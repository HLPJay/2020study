//这里适应一定的业务逻辑：
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
	void updateCallPara(void *p);
private:
	//回调函数,超时事件,回调参数的保存
	Timerfunc m_func; //回调函数
	int m_interval;   //超时间隔
	void* m_paras;     //回调函数的参数
	bool m_isParaChange; 

	int m_timeout;  // ==>超时时间 5s
	int m_interval_sign;
	int m_flag_startCommand;
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

Timer::Timer()
 : m_func(NULL), m_interval(-1),m_paras(NULL), m_isParaChange(false),m_timeout(5),m_interval_sign(0),m_flag_startCommand(false) 
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
	while(!IsStop())
	{
		if(m_isParaChange)   //参数有变化，重新执行回调
		{
			(*m_func)(m_paras);
			m_isParaChange = false;
			m_interval_sign = 0;
			m_flag_startCommand = true;
		}

		if(m_interval_sign >= m_timeout) //超时执行回调结束的指令
		{
			(*m_func)(m_paras); //执行停止命令
			m_interval_sign = 0;
			m_flag_startCommand = false;
		}
		if(m_flag_startCommand)   //时间变更标志
		{
			printf("m_interval_sign++ %d, %d\n", m_interval_sign,m_timeout);
			m_interval_sign++;
		}
		
		sleep(1);
	}
	printf("run end \n");
}

void Timer::registerHandler(TimerHandler handler, void* p)
{
	m_func = handler;
	m_paras = p;
	m_isParaChange = true;
}

void Timer::setInterval(int millisecond)
{
	m_interval = millisecond;
}

void Timer::Stop()
{
	Thread::Stop();
}

void Timer::updateCallPara(void *p)
{
	m_paras = p;
	m_isParaChange = true;
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
	time->Start(); //启动线程开始执行回调

	for(; i<3; i++)
	{
		sleep(1);
	}

	time->updateCallPara((void*)&i);
	for(; i<20; i++)
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

/*****************************************************************
腾讯面试：      https://www.yuque.com/linuxer/linux_senior/czr88p
网络编程得练练： https://www.yuque.com/linuxer/linux_senior/wk8w2m
基础库～网络库：        https://github.com/limingfan2016/game_service_system
linux多线程服务端编程：  https://github.com/834810071/muduo_study


根据业务做具体调整：
	要实现有变更重新执行回调，重新变更超时时间。
	增加一个修改触发标识，增加一个超时时间，增加一个变量修改函数



	如果参数改变，则触发回调函数的重新调用。
	如果超时，则触发停止。

这里有两种定时其的实现方案：
	1：endtime-starttime进行超时处理。
	2：无限循环，用sleep（1）作为耗时变更，条件判断


******************************************************************/