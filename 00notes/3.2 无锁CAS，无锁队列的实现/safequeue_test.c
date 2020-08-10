/***********************************
 *STL中的容器都是不安全的
 * 自己定义一个队列，实现无锁队列
 *
 * CAS比较考验代码的设计能力，所以一般不要轻易使用
 *
 * 1：linux提供的原子操作接口12个
 *                  https://gcc.gnu.org/onlinedocs/gcc-4.1.1/gcc/Atomic-Builtins.html#Atomic-Builtins
 * 2：C++11中有对应的关键字 C++11 atomic
 *                 https://en.cppreference.com/w/cpp/atomic
 * 3： mutex相关，lie哥的技术博客：
 *                   http://47.106.79.26:4001/2016/11/04/reactor_design/
 *
 * 这里是实现一个无锁队列：
 *         对比无锁队列和有锁队列的性能
***********************************/

#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <atomic>

#include <iostream>
#include <mutex>

template <typename ElemType>
struct qnode {
    struct qnode *next;
    ElemType data;
};


template <typename ElemType>
class queue{
public:
    queue(){
        m_head = m_tail = new qnode<ElemType>;
        m_head->next = NULL;
        m_tail->next = NULL;
        printf("head of queue: %p \n", m_head);
    }

    ~queue(){
        struct qnode<ElemType> * temp;
        while(m_head)
        {
            printf("delete head of queue: %p \n", m_head);
            temp = m_head;
            m_head = m_head->next;
            delete temp;
        }
    }
public:
    bool pop(ElemType &p)//取头结点的数据 注意释放
    {
        struct qnode<ElemType>* n =m_head->next;
        if(n == NULL)
        {
         return false;
        }
        p = n->data;
        m_head->next = n->next;
        delete n;
        return true;
    }
    void push(const ElemType &p)
    {
        struct qnode<ElemType> * temp = new qnode<ElemType>;
        temp->data = p;
        temp->next = NULL;

        m_tail ->next = temp;
        m_tail = temp;
    }
    bool pop2(ElemType &e)
    {
        struct qnode<ElemType> *p = NULL;
        struct qnode<ElemType> *np = NULL;
        int count = 0;
        do
        {
            p = m_head; // 头节点，不真正存储数据
            np = p->next;
            if (p->next == NULL) // 首元节点为空，则返回
            {
            return false;
            }
            if (count++ >= 1)
            {
            printf("pop count:%d, p->next:%p\n", count, p->next);
            }
            // 更新头结点位置
        } while (!__sync_bool_compare_and_swap(&m_head, p, p->next));
        e = p->next->data;
        // printf("pop p:%p\n", p);
        delete p; // 因为我们已经将头部节点换成了p->next, 所以可以释放掉
        return true;
    }
    //采用无锁CAS的方式进行存
    void push2(const ElemType &e)
    {
        struct qnode<ElemType> *p = new qnode<ElemType>;
        // printf("push head:%p, p:%p\n", m_head, p);
        p->next = NULL;
        p->data = e;

        struct qnode<ElemType> *t = m_tail;
        struct qnode<ElemType> *old_t = m_tail;
        int count = 0;
        do
        {
            while (t->next != NULL) // 非空的时候要去更新 t->_next
                t = t->next;          // 找到最后的节点
            if (count++ >= 1)
            {
                printf("push count:%d, t->next:%p\n", count, t->next);
            }
            // 将null换为p即是插入的节点
        } while (!__sync_bool_compare_and_swap(&t->next, NULL, p));

        // 将最后的节点_tail更换为p节点
        __sync_bool_compare_and_swap(&m_tail, old_t, p);
    }
private:
    //一种是对象里控制
    struct qnode<ElemType>* m_head; //队列的头 ==》出队
    struct qnode<ElemType>* m_tail;    //队列的尾部 ==》入队
};

static int lxx_atomic_add(int *ptr, int increment)
{
    int old_value = *ptr;
    __asm__ volatile("lock; xadd %0, %1 \n\t"
                   : "=r"(old_value), "=m"(*ptr)
                   : "0"(increment), "m"(*ptr)
                   : "cc", "memory");
    return *ptr;
}



#define THREAD_NUM 3
#define THREAD_PUSH_COUNT 1000000
static int g_count_push =0;
static int g_count_pop = 0;


typedef void* (*pthread_func)(void*);
//线程回调函数
static queue<int> g_no_mutex_queue;
void * no_mutex_push_func(void *arg);
void * no_mutex_pop_func(void *arg);
static queue<int> g_mutex_queue;
static std::mutex g_mutex;
void * mutex_push_func(void *arg);
void * mutex_pop_func(void *arg);
static queue<int> g_safe_queue;
//CAS实现无锁队列保证线程安全
void * safequeue_push_func(void *arg);
void * safequeue_pop_func(void *arg);
void test_thread_func( pthread_func push_func, pthread_func pop_func, char **argv );
int main()
{
    //多线程给队列进行插入和删除操作，观察结果
    printf("thread num is %d. \n", THREAD_NUM);

    printf("------------------------------use no mutex--------------------------------\n");
    //通过多线程调用回调函数进行测试  分别查看push和pop的次数
    g_count_push =0;
    g_count_pop = 0;
    test_thread_func(no_mutex_push_func, no_mutex_pop_func, NULL);

    printf("------------------------------use  mutex queue--------------------------------\n");
    g_count_push =0;
    g_count_pop = 0;
    test_thread_func(mutex_push_func, mutex_pop_func, NULL);

    printf("------------------------------  queue is safely--------------------------------\n");
    g_count_push =0;
    g_count_pop = 0;
    test_thread_func(safequeue_push_func, safequeue_pop_func, NULL);


    return 0;
}

/********************************
 *
head of queue: 008175C8
thread num is 2.
start time is 28, end is:336, use time is 0
push count is 2000000 , pop count is 634(因为有很多次取得时候没有数据)
delete head of queue: 008175C8

 * ******************************/
void * no_mutex_push_func(void */*arg*/)
{
    for(int i=0; i<THREAD_PUSH_COUNT; i++)
    {
        g_no_mutex_queue.push(i);
        //计算push的次数 这里用的原子加  必然不会出现问题
        lxx_atomic_add(&g_count_push, 1);
    }
    return NULL;
}

void * no_mutex_pop_func(void */*arg*/)
{
    for(int i=0; i<THREAD_PUSH_COUNT; i++)
    {
        int value = 0;
        if(g_no_mutex_queue.pop(value))
        {
            lxx_atomic_add(&g_count_pop, 1);
        }
        else{
            lxx_atomic_add(&g_count_pop, 1);
        }
    }

    return NULL;
}


void * mutex_push_func(void */*arg*/)
{
    for(int i=0; i<THREAD_PUSH_COUNT; i++)
    {
        g_mutex.lock();
        g_count_push++;
        g_mutex_queue.push(i);
        g_mutex.unlock();
    }
    return NULL;
}

void * mutex_pop_func(void */*arg*/)
{
    for(int i=0; i<THREAD_PUSH_COUNT; i++)
    {
        g_mutex.lock();
        int value = 0;
        if(g_mutex_queue.pop(value))
        {
            g_count_pop++;
        }else
        {
            g_count_pop++;
            //            printf("pop ERROR.\n");
        }
        g_mutex.unlock();

    }
    return NULL;
}

void * safequeue_push_func(void */*arg*/)
{
    for(int i=0; i<THREAD_PUSH_COUNT; i++)
    {
        g_safe_queue.push2(i);
        lxx_atomic_add(&g_count_push, 1);
    }
    return  NULL;
}
void * safequeue_pop_func(void */*arg*/)
{
    int last_value = 0;
    static int s_pid_count = 0;
    s_pid_count++;
    int pid = s_pid_count;
    while (true)
    {
    int value = 0;
    if (g_safe_queue.pop2(value))
    {
      last_value = value;
      if (g_count_pop != value)
      {
        printf("pid:%d, -> value:%d, expected:%d\n", pid, value, g_count_pop);
      }
      g_count_pop++;
      // printf("pid:%d, -> value:%d\n", pid, value);
    }
    else
    {
      // printf("pid:%d, null\n", pid);
    }

    if (g_count_pop >= THREAD_NUM * THREAD_PUSH_COUNT)
    {
          printf("%s dequeue:%d\n", __FUNCTION__, last_value);
          break;
    }
    }
    printf("%s exit\n", __FUNCTION__);
    return NULL;
}

void test_thread_func( pthread_func push_func, pthread_func pop_func, char **argv )
{
    clock_t start = clock();

    //创建多个线程取执行push动作
    pthread_t push_threadid[THREAD_NUM] = {0};
    for(int i=0; i<THREAD_NUM; i++)
    {
        int ret = pthread_create(&push_threadid[i], NULL, push_func,argv);
        if(ret!=0)
        {
            printf("pthread create error :%d \n", i);
        }
    }
     //创建多个线程取执行pop动作
    pthread_t pop_threadid[THREAD_NUM] = {0};
    for(int i=0; i<THREAD_NUM; i++)
    {
        int ret = pthread_create(&pop_threadid[i], NULL, pop_func,argv);
        if(ret!=0)
        {
            printf("pthread create error :%d \n", i);
        }
    }

    for(int i=0; i<THREAD_NUM; i++)
    {
        pthread_join(push_threadid[i], NULL);
    }
    for(int i=0; i<THREAD_NUM; i++)
    {
        pthread_join(pop_threadid[i], NULL);
    }
    clock_t end = clock();
    //    观察执行结果
    printf("start time is %lu, end is:%lu, use time is %lu\n", start, end,(end-start)/CLOCKS_PER_SEC);
    printf("push count is %d , pop count is %d \n" ,g_count_push, g_count_pop);
}
