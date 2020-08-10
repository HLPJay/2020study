/***********************************
 *实现CAS无锁队列
 *
 *
 * 解释：CAS是原子操作！
 *         ABA问题（值变化后又变为原值）---》方案1-》加标志解决？
 *         循环时间长开销大
 *         只能保证一个共享变量的原子操作
 * 要用volatile修饰：不被编译器优化
 *         CAS不从缓存中取数据，直接从内存中取？
 *
 * 乐观锁  悲观锁  共享锁
 *
 *
Queue m_head :00F61118
Queue m_head :00F61128
-----------THREAD_NUM to push-------------
start time = 2, end time = 256, use time = 4294967042
-----------get number of pop---------------------
start time = 256, end time = 401, use time = 4294967151
pop queue time= 3000000 Expect =3000000
-----------THREAD_NUM to push LOOP_TIME-------------
start time = 402, end time = 652, use time = 4294967046
has insert num 3000000
-----------THREAD_NUM to get LOOP_TIME-------------
start time = 652, end time = 865, use time = 4294967083
more thread to pop is TRUE.

***********************************/

#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <atomic>

template <typename ElemType>
struct qnode {
    ElemType data;
    struct qnode *next;
};

//定义类 相当于管理节点
template <typename ElemType>
class Queue{
public:
    Queue()
    {//哨兵节点 方便操作
        struct qnode<ElemType> *head = new qnode<ElemType>;
        m_head = m_tail = head;
        m_head->next = m_tail->next = NULL;

        printf("Queue m_head :%p \n", m_head);
    }
    ~Queue(){
        struct qnode<ElemType>* volatile temp = NULL;
        while(m_head)
        {
//            printf("delete m_head: %p \n",m_head);
            temp = m_head;
            m_head = m_head->next;
            delete temp;
        }
    }
    
    //在队尾插入
    bool push(const ElemType &data)
    {
        struct qnode<ElemType> * node = new qnode<ElemType>;
        node->data = data;
        node->next =NULL;

        //临时变量放入缓存中
        struct qnode<ElemType> * t = m_tail;
        struct qnode<ElemType> * old_tail = m_tail;
        do{
            while(t->next != NULL)
                t = t->next;

        }while(!__sync_bool_compare_and_swap(&t->next, NULL, node)); //直到指向最后节点，吧node节点赋给最后节点
       //注意这里第一个参数的&
        __sync_bool_compare_and_swap(&m_tail, old_tail, node);//如果缓存中的tail和内存中的一样，则将最后节点更新为node
        return true;
    }

    //在队首提取数据
    bool pop(ElemType &data)
    {
        struct qnode<ElemType> *head = NULL;
        //判断内存中的头和缓存中的head相同，则把内存中的头后移
        do
        {
            head = m_head;
            if(head->next == NULL)
            {
                return false;
            }

        }while(!__sync_bool_compare_and_swap(&m_head, head, head->next));
        data = head->next->data;
        delete head;
        return true;
    }
private:
    struct qnode<ElemType>* volatile  m_head;
    struct qnode<ElemType>*  volatile m_tail;
};
//无锁队列，使用的时候不用关注加锁，线程安全直接使用
/******************************
 * 测试方案：
 *              多个线程操作队列
 *         观察
 *              线程是安全的，所以插入的次数一定合格 多线程插入，然后pop成功的次数
 *                                                                               插入固定的数据后，pop多次去取，查看是否取完
 *
 * ***************************/

#define THREAD_NUM 3
#define LOOP_TIME 1000000
typedef void *(*thread_func)(void*);
static Queue<int> g_queue;
static Queue<int> g_queue1;
void* queue_pop_func(void *arg);
void* queue_push_func(void *arg);
void* queue_pop_func1(void *arg);
void* queue_push_func1(void */*arg*/);
void queue_test_set(thread_func func, void*argv);
void queue_test_get(thread_func func, void*argv);
int main()
{

    //采用回调函数的方式去测试 可扩展 传入线程回调函数和回调参数
    //多线程push，然后一个线程取（验证push次数正确）
    printf("-----------THREAD_NUM to push-------------\n");
    queue_test_set(queue_push_func, NULL);  //多个线程执行插入动作

    printf("-----------get number of pop---------------------\n");
    unsigned long count =0;
    queue_test_get(queue_pop_func, &count);
    printf("pop queue time= %lu Expect =%lu \n",count, ((long)THREAD_NUM*LOOP_TIME) );

    //多线程已经存好的数据，然后多个线程取数据，查看是否取完了
    printf("-----------THREAD_NUM to push LOOP_TIME-------------\n");
    queue_test_set(queue_push_func1, NULL);  //多个线程执行插入动作
    printf("has insert num %lu \n", ((long)THREAD_NUM*LOOP_TIME) );
    
    //多个线程取数据，看看是否有取失败的场景，看看是否有剩余
    printf("-----------THREAD_NUM to get LOOP_TIME-------------\n");
    queue_test_set(queue_pop_func1, NULL);  //多个线程执行取数据

    int value;
    if(g_queue1.pop(value))
    {
           printf("more thread to pop is ERROR.\n");
    }else
    {
        printf("more thread to pop is TRUE.\n");
    }
    return 0;
}


void* queue_push_func(void */*arg*/)
{
    //线程回调  执行队列的插入
    for(long  i=1; i<=LOOP_TIME; i++)
    {
//        printf("i=%lu \n", i);
        g_queue.push(i);
    }
    return NULL;
}

void* queue_push_func1(void */*arg*/)
{
    //线程回调  执行队列的插入
    for(long  i=1; i<=LOOP_TIME; i++)
    {
//        printf("i=%lu \n", i);
        g_queue1.push(i);
    }
    return NULL;
}
static int lxx_atomic_add(long *ptr, int increment)
{
  long old_value = *ptr;
  __asm__ volatile("lock; xadd %0, %1 \n\t"
                   : "=r"(old_value), "=m"(*ptr)
                   : "0"(increment), "m"(*ptr)
                   : "cc", "memory");
  return *ptr;
}


void* queue_pop_func(void *arg)
{
//    long count = *(long *)arg;
    int value;
    while(g_queue.pop((value)))
    {
//        printf("pop count =%lu \n", count);
        lxx_atomic_add((long *)arg, 1);
    }
    return NULL;
}

void* queue_pop_func1(void */*arg*/)
{
//    long count = *(long *)arg;

    for(long  i=1; i<=LOOP_TIME; i++)
    {
        int value;
        if(!g_queue1.pop(value))
        {
            printf("pop error %lu \n", i);
        }
    }
    return NULL;
}

void queue_test_set(thread_func func, void*argv)
{
    //可以统计运行时间==》多个线程执行push
    clock_t start = clock();
    pthread_t t_id[THREAD_NUM] ={0};
    for(int i=0;i<THREAD_NUM; i++)
    {
        int ret  =pthread_create(&t_id[i], NULL, func, argv);
        if(ret != 0)
        {
            printf("create pthread error \n");
        }
    }

    for(int i=0; i<THREAD_NUM; i++)
    {
        pthread_join(t_id[i], NULL);
    }
    clock_t end_time = clock();
    printf("start time = %lu, end time = %lu, use time = %lu \n", start, end_time, (start-end_time));
    //如果push不是原子的
    return ;
}
void queue_test_get(thread_func func, void*argv)
{
    //直接统计可以pop的个数确定push的个数是对的
    clock_t start = clock();
    pthread_t t_id;
    int ret= pthread_create(&t_id, NULL, func, argv);
    if(ret != 0)
    {
        printf("create pthread error \n");
    }
    pthread_join(t_id, NULL);
    clock_t end_time = clock();
    printf("start time = %lu, end time = %lu, use time = %lu \n", start, end_time, (start-end_time));
    //如果push不是原子的
    return ;
}
