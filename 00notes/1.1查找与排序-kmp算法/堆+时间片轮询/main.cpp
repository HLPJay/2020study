#include "Cheap.h"
#include "timeheap.h"
#include <iostream>
#include <thread>
#include <atomic>

#include "threadpool.h"
using namespace std;

void Test_Time_heap();

int main()
{

      //基于堆实现的时间片轮询
   Test_Time_heap();


    return 0;
}

std::atomic<bool> _continue(false);
void funcCallback(void *arg)
{
    UserData *data = (UserData *)arg;
    cout<<"data is "<<data->m_eventid<<endl;
}

void timeCallback()
{
    TimeHeap time_heap;
    time_heap.Insert(TimeEntry(1, 1000000, funcCallback, UserData(),LOOP_TIMER ));
    for(int i=0;i<10;i++)
    {
        const uint64_t timeout = i * 2000;
        time_heap.Insert(TimeEntry(i, timeout, funcCallback, UserData(),ONCE_TIMER ));
    }

    while(!_continue)
    {
        time_heap.TimeLoop(); //执行一个任务就结束
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void Test_Time_heap()
{
    /************************************************
    基于时间的先后，执行相关的对象任务
     **********************************************/

    std::thread t(timeCallback);
    t.detach();
    getchar();
    _continue  = true;
}


void Test_My_heap()
{

    vector<int> vec = {4,6,1,3,2,8,5,7,3,9};

    Cheap<int, MIN_HEAP, false> min_heap;

    for(int i=0; i<(int)vec.size();i++)
    {
        min_heap.Insert(vec[i]);
        min_heap.Print();
    }
    cout<<min_heap.GetTop()<<endl;
    min_heap.DeleteTop();
    min_heap.Print();
    min_heap.Insert(20);
    min_heap.Print();
    min_heap.Insert(7);
    min_heap.Print();
   
    Cheap<int, MAX_HEAP> max_heap;
    for(int i=0; i<(int)vec.size();i++)
    {
        max_heap.Insert(vec[i]);
        max_heap.Print();
    }
    cout<<max_heap.GetTop()<<endl;
    max_heap.DeleteTop();
    max_heap.Print();
    max_heap.Insert(20);
    max_heap.Print();
    max_heap.Insert(7);
    max_heap.Print();

    Cheap<int, MIN_HEAP, false> sort_min_heap;
    sort_min_heap.TestSort(vec);
    cout<<"min:";
     for(int i=0; i<(int)vec.size();i++)
     {
        cout<<vec[i]<<"  ";
     }
    cout<<endl;

    vector<int> vec2 = {4,6,1,3,2,8,5,7,3,9};
    Cheap<int, MAX_HEAP, false> sort_max_heap;
    sort_max_heap.TestSort(vec2);
    cout<<"max:";
    for(int i=0; i<(int)vec2.size();i++)
    {
       cout<<vec2[i]<<"  ";
    }
   cout<<endl;

}
