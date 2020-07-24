#ifndef __TIMER_MNG_
#define __TIMER_MNG_

#define ull unsigned long long 
#define __INF__ 1e18

#include <iostream>
#include <vector>
using namespace std;

enum t_type {ONCE,CIRCLE};

class Timer_mng;
class Timer;

struct Heap_entry{		//堆条目
	unsigned long long time;
	Timer *timer;
};
struct Timer{
	t_type ttype;		//任务类型，一次还是循环
	void *(*run)(void *args);	//回调函数
	void *arg;					//会调函数参数
	ull itvl;					//时间片长度(ms)
	ull expires;				//到期时间
	int heapIndex;				//在堆中的下标
	Timer();					//初始化
	void Start(void *(*run)(void *args),void *arg,ull itvl, t_type ttype);
	void OnTimer();				//无剩余时间		
};
struct Timer_mng{
	vector<Heap_entry*> heap;		//堆
	~Timer_mng();					//析构函数
	void DetectTimers();			//检测是否超时
	void AddTimer(Timer* timer);	//向堆中添加一个定时器
	void RemoveTimer(Timer* timer);	//移除定时器
	void UpHeap(int idx);			//堆操作，向上更新
	void DownHeap(int idx);			//堆操作，向下更新
	void SwapHeap(int idx1, int idx2);	//更换堆中的两个元素
};

ull Get_now();			//得到当前时间(ms)

#endif



#include<cstdio>
#include<cstring>
#include<sys/time.h>
#include<vector>
// #include"Timer_mng.h"
using namespace std;
//Timer

Timer::Timer(){
    heapIndex=-1;
}
void Timer::Start(void *(*run)(void *args),void *arg,ull itvl, t_type ttype){
    itvl=itvl;
    this->run=run,this->arg=arg;
    ttype=ttype;
    printf("Now :%llu\n",Get_now());
    expires=itvl+Get_now();
}
 
void Timer::OnTimer(){
    heapIndex=-1;
   // printf("%d\n",*(int*)arg);
    this->run(this->arg);
}
 
//Timer_mng

Timer_mng::~Timer_mng(){
    heap.clear();
}

void Timer_mng::AddTimer(Timer* timer){  
    timer->heapIndex=heap.size();
    Heap_entry *entry=(Heap_entry*)malloc(sizeof(Heap_entry));
    entry->time=timer->expires;
    entry->timer=timer;
//    timer->run(timer->arg);
    heap.push_back(entry);
    UpHeap(heap.size()-1);
 //   free(entry);
}
 
void Timer_mng::RemoveTimer(Timer* timer){
    int idx=timer->heapIndex;
    if (!heap.empty()&&idx<heap.size()){
        if(idx==heap.size()-1){
            heap.pop_back();
        }
        else{
            SwapHeap(idx,heap.size()-1);
            heap.pop_back();
            int parent=(idx-1)/2;  //堆中父节点编号
            if(idx>0&&heap[idx]->time<heap[parent]->time)
                UpHeap(idx);
            else
                DownHeap(idx);
        }
    }
}
 
void Timer_mng::DetectTimers(){
    printf("----%lu\n",heap.size());
    while(!heap.empty()){
        ull now=Get_now();
        if(heap[0]->time<=now){
            Timer *timer=(Timer *)malloc(sizeof(Timer));
            timer=heap[0]->timer;
      //      printf("%d\n",*(int *)heap[0]->timer->arg);
            if(heap[0]->timer->run==NULL||heap[0]->timer->arg==NULL){
                perror("ERROR!");
                return;
            }

           // heap[0]->timer->run(heap[0]->timer->arg);
            RemoveTimer(timer);
            if(timer->ttype==CIRCLE){
                timer->expires=timer->itvl+Get_now();
                this->AddTimer(timer);
            }
            timer->OnTimer();
           // printf("xxx\n");
        }
    }
}
 
void Timer_mng::UpHeap(int idx){
    int parent=(idx-1)/2;
    while(idx>0&&heap[idx]->time<heap[parent]->time){
        SwapHeap(idx,parent);
        idx=parent;
        parent=(idx-1)/2;
    }
}
 
void Timer_mng::DownHeap(int idx){
    int child=idx*2+1;
    while(child<heap.size()){
        int minChild=(child+1==heap.size()||heap[child]->time<heap[child+1]->time)
            ?child:child+1;
        if(heap[idx]->time<heap[minChild]->time)
            break;
        SwapHeap(idx,minChild);
        idx=minChild;
        child=idx*2+1;
    }
}
 
void Timer_mng::SwapHeap(int idx1, int idx2){
    Heap_entry *tmp=heap[idx1];
    heap[idx1]=heap[idx2];
    heap[idx2]=tmp;
    heap[idx1]->timer->heapIndex=idx1;
    heap[idx2]->timer->heapIndex=idx2;
}

ull Get_now(){
    #ifdef _MSC_VER
        _timeb timebuffer;
        _ftime(&timebuffer);
        ull ret=timebuffer.time;
        ret=ret*1000+timebuffer.millitm;
        return ret;
    #else
        timeval tv;         
        ::gettimeofday(&tv,0);
        ull ret=tv.tv_sec;
        return ret*1000+tv.tv_usec/1000;
    #endif
}
// init()


#include<stdio.h>
#include<iostream>
#include<vector>
// #include"Timer_mng.h"
using namespace std;

#define LONGTIME 8000
// 定时事件回调函数
void *onTime(void *arg){
	int *id=(int *)arg;
    cout<<*id<<" Game Over!"<< endl;
    return nullptr;
}
 
int main(){
	// 初始化
    Timer_mng Mymng;
    for(int i=0;i<10;i++){
 		Timer *t=(Timer *)malloc(sizeof(Timer));   	
    	int *tmp=(int *)malloc(sizeof(int));
    	*tmp=i;
    	t->Start(onTime,tmp,LONGTIME,ONCE);
    	//t->run(t->arg);
    	Mymng.AddTimer(t);
    }
   // while(Mymng.heap.size())
 	Mymng.DetectTimers();
    return 0;
}

/**********************
定义执行体信息
定义堆信息
通过堆结构的事件片轮寻
***********************/
