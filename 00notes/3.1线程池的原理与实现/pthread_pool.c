/************************************
该代码基于windows版本下的qt环境实现

new的时候执行创建线程池对应的函数
delete的时候执行销毁线程池
add的时候调用添加


其实也就是对定义好的接口进行封装

如何搞成循环队列--》没有必要
如何检测线程的运行个数？
*************************************/
#include <string.h>
#include <pthread_signal.h>
#include <pthread.h>

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

typedef struct NWORKER
{
    pthread_t threadid;
    bool isStopWorking = false;

    struct MANAGER *manager;
    struct NWORKER *prev;
    struct NWORKER *next;
}NWORKERS;

typedef struct NJOB
{
    void (*jobFunc)(struct NJOB* job);
    void *funcpara;

    struct NJOB *prev;
    struct NJOB *next;
}NJOBS;

typedef struct MANAGER
{
    // int threadnum = 0;
    // int workingnum = 0;
    struct NWORKER * workers;
    struct NJOB * jobs;

    //互斥锁和信号锁
    pthread_mutex_t thread_mtx;
    pthread_cond_t thread_cond;
}THREADPOOL;

//把一个对象插入到链表中 在头部插入
#define LL_ADD(item, list) do{ \
    item->prev = NULL;         \
    item->next = list;          \
    list = item;               \
}while(0)

#define LL_DELETE(item, list) do{                           \
    if(item->prev!=NULL) item->prev->next = item->next;     \
    if(item->next != NULL) item->next->prev = item->prev;   \
    if(item == list) list = item->next;                     \
    item->prev = item->next = NULL;                         \
}while(0)

//线程的执行体  负责取任务并执行相关内容
static void* workerFunc(void *arg)
{
    printf("thread start...\n");
    NWORKERS* worker = (NWORKERS *)arg;
    while(1)
    {
        //从任务队列中取对象   需要加锁
        pthread_mutex_lock(&worker->manager->thread_mtx);
        while(worker->manager->jobs == NULL)
        {
            if(worker->isStopWorking) break;
            printf("thread wait...\n");
            pthread_cond_wait(&worker->manager->thread_cond, &worker->manager->thread_mtx);
        }
        if(worker->isStopWorking)
        {
            pthread_mutex_unlock(&worker->manager->thread_mtx);
        }

        NJOBS * job = worker->manager->jobs;
        if(job!=NULL)
        {
            LL_DELETE(job, worker->manager->jobs);
        }
        pthread_mutex_unlock(&worker->manager->thread_mtx);

        if(job == NULL) continue;
        job->jobFunc(job);
    }

    free(worker);
    pthread_exit(NULL);
}
//创建线程池，线程执行取任务
int ThreadPoolCreate(THREADPOOL *pool, int number)
{
    if(pool == NULL)
    {
        perror("Pool is NULL.\n");
        return -1;
    }
    if(number<1)
    {
        number =1;
    }
    memset(pool, 0, sizeof(THREADPOOL));

    pthread_mutex_t blank_mtx = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER; //init ializer

    memcpy(&pool->thread_mtx, &blank_mtx, sizeof(pool->thread_mtx));
    memcpy(&pool->thread_cond, &blank_cond, sizeof(pool->thread_cond));
    for(int i=0; i<number; i++)
    {
        //创建线程的个数
        printf("start create thread %d \n", i);
        NWORKERS* worker = (NWORKERS*)malloc(sizeof(NWORKERS));
        if(worker == NULL)
        {
            perror("malloc worker error. \n");
            return 1;
        }
        memset(worker, 0, sizeof(NWORKERS));
        //初始化
        worker->manager = pool;
        int ret = pthread_create(&worker->threadid, NULL, workerFunc, (void *)worker);
        if(ret != 0)
        {
            perror("create thread error. \n");
            free(worker);
            return 1;
        }
        LL_ADD(worker, pool->workers);
    }
    return 0;
}
//往线程池中存任务
int ThreadPoolPushTask(THREADPOOL *pool, NJOBS* job)
{
    if(pool == NULL || job == NULL)
    {
        perror("pool or job is NULL. \n");
        return -1;
    }
    pthread_mutex_lock(&pool->thread_mtx);

    LL_ADD(job, pool->jobs);
    pthread_cond_signal(&pool->thread_cond);
    pthread_mutex_unlock(&pool->thread_mtx);
    return 0;
}

//如果任务没有执行完，会有内存泄漏
void ThreadPoolShutDown(THREADPOOL * pool)
{
    if(pool == NULL) return;
    NWORKERS *worker = pool->workers;
    while(worker != NULL)
    {
        worker->isStopWorking =true;
        worker = worker->next;
    }
    pthread_mutex_lock(&pool->thread_mtx);

    pool->workers = NULL;
    pool->jobs = NULL;
    pthread_cond_broadcast(&pool->thread_cond);
    pthread_mutex_unlock(&pool->thread_mtx);
}

// //测试  构建任务队列，然后查看任务运行
// #define THREAD_NUM 5
void myjobfunc(NJOBS *job)
{
    int index =* (int *)job->funcpara;
    printf("index : %d, selfid : %d\n", index, pthread_self());

    free(job->funcpara);
    free(job);
}

// int main()
// {
//     printf("Pthread Pool  start.\n");
//     THREADPOOL t_pool;
//     ThreadPoolCreate(&t_pool, THREAD_NUM);//创建线程池

//     for(int i =0; i<100; i++)
//     {
//         NJOBS* job = (NJOBS*) malloc(sizeof(NJOBS));
//         if(job == NULL)
//         {
//             perror("malloc job error.\n");
//             exit(1);
//         }

//         //初始化并塞值
//         memset(job, 0, sizeof(NJOBS));
//         job->jobFunc = myjobfunc;
//         job->funcpara = malloc(sizeof(int));
//         *(int*)job->funcpara = i;
//         ThreadPoolPushTask(&t_pool, job);
//     }
//     getchar(); //这里要让主线程运行着 ！！！！！！！
//     return 0;
// }

//对接口进行封装  实现模块化
//使用可变传参的方式，定义函数指针取调用
typedef struct {
    size_t size;
    void* (*ctor)(void *_self, va_list *params);
    void* (*dtor)(void *_self);
} AbstractClass;

void *New(const void *_class1, ...) {
    const AbstractClass *class1 = (const AbstractClass *)_class1;
    void *p = calloc(1, class1->size);
    memset(p, 0, class1->size);

    assert(p);
    *(const AbstractClass**)p = class1;

    if (class1->ctor) {
        va_list params;
        va_start(params, _class1);
        p = class1->ctor(p, &params);
        va_end(params);
    }
    return p;
}


void Delete(void *_class1) {
    const AbstractClass **class1 = (const AbstractClass **)_class1;

    if (_class1 && (*class1) && (*class1)->dtor) {
        _class1 = (*class1)->dtor(_class1);
    }

    free(_class1);
}

typedef struct _ThreadPool {
    const void *_;
    THREADPOOL *wq;
} ThreadPool;
typedef struct _ThreadPoolOpera {

    size_t size;
    void* (*ctor)(void *_self, va_list *params);
    void* (*dtor)(void *_self);
    void (*addJob)(void *_self, void *task);

} ThreadPoolOpera;

//创建线程的接口
void* ntyThreadPoolCtor(void *_self, va_list *params)
{
    ThreadPool *pool = (ThreadPool*)_self;
    pool->wq = (THREADPOOL*)malloc(sizeof(THREADPOOL));
    memset(pool->wq, 0, sizeof(THREADPOOL));
    int num = va_arg(*params, int);
    printf("ntyThreadPoolCtor --> %d\n", num);
    ThreadPoolCreate(pool->wq, num);
    return pool;
}

void* ntyThreadPoolDtor(void *_self)
{
    ThreadPool *pool = (ThreadPool*)_self;
    ThreadPoolShutDown(pool->wq);
    free(pool->wq);
    return pool;
}
void ntyThreadPoolAddJob(void *_self, void * task)
{
    //塞任务的类型
    ThreadPool *pool = (ThreadPool*)_self;
    NJOBS *job = (NJOBS*)task;

    ThreadPoolPushTask(pool->wq, job);
}
//结构体（ThreadPoolOpera）类型初始化
const ThreadPoolOpera threadOpera = {
    sizeof(ThreadPool), //这个参数没啥用吧？
    ntyThreadPoolCtor,ntyThreadPoolDtor,ntyThreadPoolAddJob,
};

const void* pthreadOpera = &threadOpera;
//new delete用来调度回调函数
int main()
{
    //为啥要用单例？
    void * pool = New(pthreadOpera, 5);

    for(int i=0; i<100; i++)
    {
        NJOBS* job = (NJOBS*) malloc(sizeof(NJOBS));
        if(job == NULL)
        {
            perror("malloc job error.\n");
            exit(1);
        }

        //初始化并塞值
        memset(job, 0, sizeof(NJOBS));
        job->jobFunc = myjobfunc;
        job->funcpara = malloc(sizeof(int));
        *(int*)job->funcpara = i;

        ThreadPoolOpera**  p_opera = (ThreadPoolOpera**)pool;
        if((*p_opera)!=NULL && (*p_opera)->addJob)
        {
            (*p_opera)->addJob(pool, job);
        }

    }
    getchar();
    return 0;
}
