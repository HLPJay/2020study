#ifndef TIMEHEAP_H
#define TIMEHEAP_H

#include "Cheap.h"
#include <iostream>
#include <chrono>
#include <functional>
class UserData
{
public:
    UserData():m_eventid(0){}
    ~UserData(){}
public:
    int m_eventid;
};

/***************************************
 *调度时间的类，使用堆来进行调度
 *执行体回调和数据的类，传给调度执行
 *
 * **************************************/
typedef std::function<void (void*)> TimeCallback;
typedef enum{ ONCE_TIMER =0, LOOP_TIMER=1}timer_loop;
inline uint64_t GetTimeStamp();

class TimeEntry
{
//拷贝构造和赋值构造
public:
    TimeEntry():m_loopTimer(ONCE_TIMER),m_TimeOutNum(100),m_callback(nullptr)
    {
        m_userData.m_eventid = 0;
        m_TimeOutEnd =  GetTimeStamp()+m_TimeOutNum;
    }

    TimeEntry(int id, uint64_t timeout, TimeCallback callback, UserData data, timer_loop loop = ONCE_TIMER )
        :m_loopTimer(loop), m_TimeOutNum(timeout),m_userData(data),m_callback(callback)
    {
        m_userData.m_eventid = id;
        m_TimeOutEnd =  GetTimeStamp()+m_TimeOutNum;
    }

    ~TimeEntry(){}

//在队列里要进行比较取先后
     bool operator >(const TimeEntry &d) const {return this->m_TimeOutEnd >d.m_TimeOutEnd;}
     bool operator <(const TimeEntry &d) const {return this->m_TimeOutEnd <d.m_TimeOutEnd;}
//在队列中涉及拷贝构造和赋值构造

TimeEntry &operator = (const TimeEntry &d)
    {
        if(this == &d) {return *this;}
        m_loopTimer = d.m_loopTimer;
        m_TimeOutNum = d.m_TimeOutNum;
        m_TimeOutEnd = d.m_TimeOutEnd;
        m_userData.m_eventid = d.m_userData.m_eventid;
        m_callback = d.m_callback;
        return *this;
    }
    TimeEntry(const TimeEntry &d)
    {
        if(this == &d) {return;}
        m_loopTimer = d.m_loopTimer;
        m_TimeOutNum = d.m_TimeOutNum;
        m_TimeOutEnd = d.m_TimeOutEnd;
        m_userData.m_eventid = d.m_userData.m_eventid;
        m_callback = d.m_callback;
    }
    void Reset()
    {
        m_TimeOutEnd = GetTimeStamp()+m_TimeOutNum;
    }
    bool IsLoop()
    {
        return m_loopTimer == ONCE_TIMER?false:true;
    }
    uint64_t GetExcuteTime()
    {
        return m_TimeOutEnd;
    }
    void OnTimer()
    {
        if(m_callback) m_callback(&m_userData);
    }
private:
      //存创建时间，超时时间，回调函数，回调数据，以及是否循环执行
    timer_loop m_loopTimer;
    uint64_t m_TimeOutNum;
    uint64_t m_TimeOutEnd;

    UserData m_userData;
    TimeCallback m_callback;

};

class TimeHeap
{
public:
    TimeHeap(){}
    ~TimeHeap(){}

    void Insert(const TimeEntry &time)
    {
        m_heap.Insert(time);
    }
    void TimeLoop()
    {
        //循环从最小堆中取数据，去执行对应的堆中业务
        uint64_t now = GetTimeStamp();
        while(!m_heap.Empty())
        {
            TimeEntry timer = m_heap.GetTop();
            if(timer.GetExcuteTime()>now)
            {
                break;
            }
            //否则，删除堆顶，执行对应的回调函数
            m_heap.DeleteTop();
            timer.OnTimer();

            if(timer.IsLoop())
            {
                timer.Reset();
                m_heap.Insert(timer);
            }
        }
    }
private:
    Cheap<TimeEntry> m_heap;
};

uint64_t GetTimeStamp()
{
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
}

#endif // TIMEHEAP_H
