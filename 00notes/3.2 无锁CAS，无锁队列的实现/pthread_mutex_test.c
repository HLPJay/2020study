/***********************************
 *c语实现：多线程对共享变量操作加锁方案
 *涉及到的知识点：
 *          传递函数指针及参数
 *          线程的使用
 *          锁(mutex)，自旋锁(spinlock(要注意初始化)), 封装原子
***********************************/
#include <ctime>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define THREAD_NUM 2
#define THREAD_LOOP_CONUNT 200000000

pthread_mutex_t  mutex; //要在线程外定义，锁线程取数据
pthread_spinlock_t spinlock;

typedef void *(*pthread_func) (void *);

void* no_mutex_func(void *arg);
void* nutex_func(void * arg);
void* spin_mutex_func(void *arg);
void* atomic_mutex_func(void *arg);
void pthread_mutex_test(pthread_func func, void *count); //传递回调函数以及回调参数

int main()
{
    printf("thread num ===>%d \n", THREAD_NUM);
    printf("count num====>%d 000 000 \n", THREAD_LOOP_CONUNT/1000/1000);
    unsigned long count =0;
    printf("---------------------no mutex test start:--------------------------\n");
    pthread_mutex_test(no_mutex_func, &count);
    printf("no mutex end of count = %lu \n", count);
    printf("---------------------no mutex test end:--------------------------\n");

    count = 0;
    printf("---------------------mutex test start:--------------------------\n");
    pthread_mutex_test(nutex_func, &count);
    printf("mutex test ====> count = %lu \n", count);
    printf("---------------------mutex test end:--------------------------\n");

    count = 0;
    printf("---------------------spinlock test start:--------------------------\n");
    pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);
    pthread_mutex_test(spin_mutex_func, &count);
    printf("spinlock test ====> count = %lu \n", count);
    printf("---------------------spinlock test end:--------------------------\n");

    count = 0;
    printf("---------------------atomic test start:--------------------------\n");
    pthread_mutex_test(spin_mutex_func, &count);
    printf("atomic test ====> count = %lu \n", count);
    printf("---------------------atomic test end:--------------------------\n");
    return 0;
}

void pthread_mutex_test(pthread_func func, void *count)
{
    //创建THREAD_NUM多个线程，同时对共享资源count进行++
   //计算耗费时间 clock()
    clock_t start_time = clock();
    pthread_t thread[THREAD_NUM] = {0};
    for(int i=0; i<THREAD_NUM; i++)
    {
        int ret = pthread_create(&thread[i], NULL, func, count);
        if(ret != 0)
        {
            printf("create thread failed. \n");
        }
    }

    for(int i=0; i<THREAD_NUM; i++)
    {
        pthread_join(thread[i], NULL);
    }

    clock_t end_time = clock();

    printf("start time == %ld    end time = %ld  usetime = %ld ms \n",end_time, start_time, end_time-start_time );
    printf("test use time:  %ld \n",  (end_time-start_time)/CLOCKS_PER_SEC);

}
void* no_mutex_func(void *arg)
{
    for(int i=0; i <THREAD_LOOP_CONUNT; i++)
    {
        (* (int*) arg)++;
    }
    return NULL;
}



void * nutex_func(void * arg)
{
//    int count =* (int*) arg;
    //通过mutex加锁实现对count进行++
    for(int i=0; i <THREAD_LOOP_CONUNT; i++)
    {
        pthread_mutex_lock(&mutex);
        (* (int*) arg)++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}
void* spin_mutex_func(void *arg)
{
    for(int i=0; i <THREAD_LOOP_CONUNT; i++)
    {
        pthread_spin_lock(&spinlock);
        (* (int*) arg)++;
        pthread_spin_unlock(&spinlock);
    }
    return NULL;
}

static int LXX_atomic_add(int * count, int para)
{
    int old = *count;
    __asm__ volatile("lock ; xadd %0, %1 \n\t"
                                :"=r"(old), "=m"(*count)
                                :"0"(para),"m"(*count)
                                :"cc", "memory");
    return *count;
}

//写一个函数，把count++用汇编写成原子的
void* atomic_mutex_func(void *arg)
{
    for(int i=0; i <THREAD_LOOP_CONUNT; i++)
    {
        LXX_atomic_add( (int*) arg, 1);
    }
    return NULL;
}
