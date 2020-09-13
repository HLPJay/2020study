/************************************
try-catch的实现方案：
	setjmp和longjmp来实现

goto实现的是函数内部的跳转
try-catch实现的是跨线程等的跳转
***********************************/

#include <stdio.h>
#include <setjmp.h>

#if 0
//第一版  实现基础的demo
jmp_buf env;
int count = 0;

void jmp_fnc (int idx)
{
	longjmp(env, idx);
}

int main()
{
	int idx = 0;
	count = setjmp(env);
	if(count == 0)
	{
		printf("count :%d \n", count);
		jmp_fnc(++idx);
	}else if(count == 1)
	{
		printf("count :%d \n", count);
		jmp_fnc(++idx);
	}else if(count == 2)
	{
		printf("count :%d \n", count);
		jmp_fnc(++idx);
	}

	return 0;
}
#endif
//第二版   使用setjmp/longjmp实现try-catch初步代码
/***************************
实现方案：
	setjmp实现保存当前上下文信息，返回的是当前信息
	longjmp负责跳转到当前的环境信息位置

	使用#define对try进行宏定义实现
*******************************/
#if 0
int count =0;
jmp_buf env;
void sub_func(int idx)
{
	longjmp(env, idx);
}
#define Try  if((count = setjmp(env)) == 0)
#define Catch(e) else if(count ==(e))
#define Throw(e) sub_func(e)
#define Finally

int main()
{
	int idx = 0;
	Try {
		printf("idx: = %d .\n", idx);
		Throw(++idx);
	}Catch(1)
	{
		printf("idx: = %d .\n", idx);
		Throw(++idx);
	}Catch(1)
	{
		printf("idx: = %d .\n", idx);
		Throw(++idx);
	}Finally
	{
		printf("Finally. \n");
	}

	return 0;
}



#endif


#if 1
//实现多线程之间，线程安全的，可嵌套的try-catch
/*******************************************
每个线程都有自己的私有空间，用线程的私有空间相关函数来实现。
	1:进程私有空间的理解
*********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include <pthread.h>

#include <string.h>
#include <stdarg.h>

//定义存储环境信息的结构体
#define EXCEPTION_MSG_LENGTH 512

typedef struct __ntyException{
	const char* name;
} ntyException;

typedef struct __ntyExceptionFrame{
	jmp_buf env;

//抛异常需要输出的一些信息
	int line;
	const char * func;
	const char * file;

//异常的触发条件，及多个用链表来标识
	ntyException * exception;
	struct __ntyExceptionFrame *prev;

	char message[EXCEPTION_MSG_LENGTH + 1];
}ntyExceptionFrame;

enum {
	ExceptionInit = 0;
	ExceptionThrown,
	ExceptionHandled, 
	ExceptionFinalized
};
//使用线程私有函数相关的接口 
#define ntyThreadData                pthread_key_t
#define ntyThreadDataCreate(key)     pthread_key_create(&(key), NULL)
#define ntyThreadDataSet(key, value) pthread_setspecific((key), (value))
#define ntyThreadDataGet(key)        pthread_getspecific((key))


ntyException SQLException = {"SQLException"};
ntyException TimeoutException = {"TimeoutException"};

//线程私有空间pthread_key_t的重定义
ntyThreadData ExceptionStack;   //pthread_key_t
#define ntyExceptionPopStack \
	ntyThreadDataSet(ExceptionStack, ((ntyExceptionFrame*)ntyThreadDataGet(ExceptionStack))->prev)
#define ReThrow \
	ntyExceptionThrow(frame.exception, frame.func, frame.file, frame.line, NULL)
#define Throw(e, cause, ...) \
	ntyExceptionThrow(&(e), __func__, __FILE__, __LINE__,  cause, ##__VA_ARGS__, NULL)

//相关的函数封装：
#define Try do{     \
	volatile int Exception_flag;  \
	ntyExceptionFrame frame;      \
	frame.message[0] = 0;         \
	frame.prev = (ntyExceptionFrame *)ntyThreadDataGet(ExceptionStack);   \
	ntyThreadDataSet(ExceptionStack, &frame); \
	Exception_flag = setjmp(frame.env);
	if(Exception_flag == ExceptionInit) {

#define Catch(e) \
		if(Exception_flag == ExceptionInit)   \
			ntyExceptionPopStack;             \
	}else if(frame.Exception_flag == &(e)) {  \
		Exception_flag = ExceptionHandled;

#define Finally \
		if(Exception_flag == ExceptionInit)   \
			ntyExceptionPopStack;             \
	}{                                        \
		if(Exception_flag == ExceptionInit)   \
			Exception_flag = ExceptionFinalized;

#define EndTry  \
		if(Exception_flag == ExceptionInit)   \
			ntyExceptionPopStack;             \
	}if(Exception_flag == ExceptionThrown)    \
		ReThrow;                              \
	}while(0)//嵌套是捕获到最后
void ntyExceptionThrow(ntyException *excep, const char* func, const char* file, int line, const char* cause, ...)
{
	va_list ap;
	ntyExceptionFrame *frame = (ntyExceptionFrame*)ntyThreadDataGet(ExceptionStack);
	if(frame)
	{
		frame->exception = excep;
		frame->line = line;
		frame->func = func;
		frame->file = file;

		if(cause)
		{
			va_start(ap, cause);
			vsnprintf(frame->message, EXCEPTION_MSG_LENGTH, cause, ap);
			va_end(ap);
		}
		ntyExceptionPopStack;
		longjmp(frame->env, ExceptionThrown);
	}else if(cause)
	{
		char message[EXCEPTION_MSG_LENGTH +1];
		
		va_start(ap, cause);
		vsnprintf(message, EXCEPTION_MSG_LENGTH, cause, ap);
		va_end(ap);

		printf("%s:%s\n raised in %s  at %s:%d \n", excep->name, message, func ?func :"?", file ? file:"?", line);
	}else
	{
		printf("%s:%p\n raised in %s  at %s:%d \n", excep->name, excep, func ?func :"?", file ? file:"?", line);
	}
}	
//多线程之间的测试

ntyException A = {"AException"};
ntyException B = {"BException"};
ntyException C = {"CException"};
ntyException D = {"DException"};

void *thread_func(void *args)
{
	pthread_t selfid = pthread_self();

	Try{
		Throw(A, "A");
	}Catch(A)
	{
		printf("catch A: %ld \n", selfid);
	} EndTry;

	Try{
		Throw(B, "B");
	}Catch(B)
	{
		printf("catch B: %ld \n", selfid);
	} EndTry;

	Try{
		Throw(C, "C");
	}Catch(C)
	{
		printf("catch C: %ld \n", selfid);
	} EndTry;

	Try{
		Throw(D, "D");
	}Catch(D)
	{
		printf("catch D: %ld \n", selfid);
	} EndTry;


	Try{
		Throw(A, "A again");
		Throw(B, "B again");
		Throw(C, "C again");
		Throw(D, "D again");
	}Catch(A)
	{
		printf("catch A again: %ld \n", selfid);
	}Catch(B)
	{
		printf("catch B again: %ld \n", selfid);
	} Catch(C)
	{
		printf("catch C again: %ld \n", selfid);
	} Catch(D)
	{
		printf("catch D again: %ld \n", selfid);
	}  EndTry;
}

static pthread_once_t once_control = PTHREAD_ONCE_INIT;

static void init_once(void)
{
	ntyThreadDataCreate(ExceptionStack); //创建线程私有空间
}
void ntyExceptionInit(void)
{
	pthread_once(&once_control, init_once); 
}
/*******************************
创建一次一个私有空间

******************************/
int main(void)
{
	ntyExceptionInit();

	Throw(D, NULL);
	Throw(C, "NULL C");
	printf("\n\n ==> test1: Try-catch\n");

	Try{
		Try {
			Throw(B, "recall B");
		} Catch (B){
			printf("catch recall B \n");
		}EndTry;

		Throw(A, NULL);
	}Catch(A) {
		printf(" \t Result OK \n");
	}EndTry;

	printf("==> Test1:OK \n\n");
	prinrf("==> Test2: Test Thread-safeness\n");

#if 1
	int i=0;
	pthread_t threads[50];

	for(i = 0; i<50; i++)
	{
		pthread_create(&threads[i], NULL, thread_func, NULL);
	}

	for(i =0; i<50; i++)
	{
		pthread_join(threads[i], NULL);
	}
#endif
	prinrf(" ==>test2 ok \n\n");
	return 0;
}
