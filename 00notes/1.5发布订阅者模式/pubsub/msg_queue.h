#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <list>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

template <class T>
class GenericQueue
{
public:
	static GenericQueue* CreateInstance(int max_items = 0);
	
	virtual ~CreateInstance(){}
	virtual int enqueue(T* item, int timeout = -1) =0 ;
	virtual int dequeue(T* &item, int timeout = -1) =0;
	virtual int peek(T* &item, int timeout=-1) =0;
	virtual bool isFull() =0;
	virtual bool isEmpty() = 0;
	virtual int queueStatus() = 0;
protected:
	GenericQueue() {}

};

template <class T>
class MsgQueue
{
public:
	MsgQueue(unsigned int max_items = 0):
		m_delegate(GenericQueue<T>::CreateInstance(max_items))
	{}
	virtual ~MsgQueue()
	{
		delete m_delegate;
	}
//TODO reinterpret_cast<T*> 强制类型转换
	virtual int enqueue(T* item, int timeout = 0)
	{
		return m_delegate->enqueue(reinterpret_cast<T*>(item), timeout);
	}
	virtual int dequeue(T* &item, int timeout = 0)
	{
		return m_delegate->dequeue(reinterpret_cast<T* &>(item), timeout);
	}
	virtual int peek(T* &item, int timeout=0)
	{
		return m_delegate->peek(reinterpret_cast<T* &>(item), timeout);
	}

	virtual bool isFull()
	{
		return m_delegate->isFull();
	}

	virtual bool isEmpty()
	{
		return m_delegate->isEmpty();
	}

	virtual int queueStatus()
	{
		return m_delegate->queueStatus();
	}

protected:
	GenericQueue<T> *m_delegate;
}
/*************************
TODO :
	pthread_mutex_init, pthread_cond_init
**************************/
template <class T>
class PosixQueue: public GenericQueue<T>
{
public:
	PosixQueue(unsigned int max_items);
	~PosixQueue();
	int enqueue(T* item, int timeout);
	int dequeue(T* &item, int timeout);
	int peek(T*& item, int timeout);
	bool isFull();
	bool isEmpty();
	int queueStatus();
private:
	void abort();
	int getTimeout(int timeout, struct timespec& timed);
private:
	unsigned int    m_maxItems;
	pthread_mutex_t m_mutex;
	pthread_cond_t  m_canPushCondition;
	pthread_cond_t  m_canPopConditions;
	
	int 		    m_pusherWaitingCount;
	int 		    m_poperWaitingCount;
	std::list<T*>   m_items;
	bool            m_aborting;
}
template <calss T>
PosixQueue<T>::PosixQueue(unsigned int max_items)
	:m_maxItems(max_items),
	m_pusherWaitingCount(0),
	m_poperWaitingCount(0),
	m_aborting(false)
{
	pthread_mutex_init(&m_mutex, NULL);
	pthread_cond_init(&m_canPushCondition,NULL);
	pthread_cond_init(&m_canPopConditions, NULL);
}

template <calss T>
PosixQueue<T>::~PosixQueue()
{
	abort();

	pthread_cond_destory(&m_canPushCondition);
	pthread_cond_destory(&m_canPopConditions);
	pthread_mutex_destory(&m_mutex);
}

template <calss T>
int PosixQueue<T>::enqueue(T* item, int timeout)
{
	struct timespec timed;
	if(timeout != -1)
	{
		getTimeout(timeout, timed);
	}

	if(pthread_mutex_lock(&m_mutex))
	{
		return -1;
	}

	int result = 0;
	if(m_maxItems)
	{
		while(m_items.size() >= m_maxItems)
		{
			++m_pusherWaitingCount;
			if(timeout == -1)
			{
				pthread_cond_wait(&m_canPushCondition, &m_mutex);
				--m_pusherWaitingCount;
			}
			else
			{
				int wait_res = pthread_cond_timedwait(&m_canPushCondition, &m_mutex, &timed);
				--m_pusherWaitingCount;
				if(wait_res == ETIMEDOUT)
				{
					result = -1;
					break;
				}
			}
			if(m_aborting)
			{
				result = -1;
				break;
			}
		}
	}

	if(result ==0 )
	{
		m_items.push_back(item);
		if(m_poperWaitingCount)
		{
			pthread_cond_broadcast(&m_canPopConditions);
		}

	}
	pthread_mutex_unlock(&m_mutex);
	return result;
}

template <calss T>
int PosixQueue<T>::dequeue(T* &item, int timeout)
{
	struct timespec timed;
	if(timeout!=-1)
	{
		getTimeout(timeout, timed);
	}

	if(pthread_mutex_lock(&m_mutex))
	{
		return -1;
	}
	int result =0;
	if(timeout)
	{
		while(m_items.empty())
		{
			++m_poperWaitingCount;
			if(timeout == -1)
			{
				pthread_cond_wait(&m_canPopConditions, &m_mutex);
				--m_poperWaitingCount;
			}
			else
			{
				int wait_res = pthread_cond_timedwait(&m_canPopConditions,  &m_mutex,  &timed);
                --m_poperWaitingCount;
                if (wait_res == ETIMEDOUT)
                {
                    result = -1;
                    break;
                }
			}
			if(m_aborting)
			{
				result = -1;
				break;
			}
		}
	}
	if(m_items.empty() &&(result == 0))
	{
		result = -1;
	}

	if(result == 0)
	{
		item = m_items.front();
		m_items.pop_front();
	}

	if(m_maxItems && (result == 0) && m_pusherWaitingCount)
	{
		pthread_cond_broadcast(&m_canPushCondition);
	}
	pthread_mutex_unlock(&m_mutex);
	return result;

}

template <calss T>
int PosixQueue<T>::peek(T*& item, int timeout)
{
	struct timespec timed;
	if(timeout != -1)
	{
		getTimeout(timeout, timed);
	}

	if(pthread_mutex_lock(&m_mutex))
	{
		return -1;
	}

	int result = 0;
	typedef std::list<T*>::iterator head = m_items.begin();

	if(timeout)
	{
		while(head == m_items.end())
		{
			++m_poperWaitingCount;
			if(timeout == -1)
			{
				pthread_cond_wait(&m_canPopConditions,&m_mutex);
				--m_poperWaitingCount;
			}
			else
			{
				int wait_res = pthread_cond_timedwait(&m_canPopConditions, &m_mutex, &timed);
				--m_poperWaitingCount;
				if(wait_res == ETIMEDOUT)
				{
					result = -1;
					break;
				}
			}

			if(m_aborting)
			{
				result = -1;
				break;
			}
			head = m_items.begin();
		}
	}
	else
	{
		if(head == m_items.end())
		{
			result = -1;
		}
	}
	item = (head != m_items.end())? head:NULL;
	pthread_mutex_unlock(&m_mutex);
	return result;
}

template <calss T>
bool PosixQueue<T>::isFull()
{
	bool full;
	pthread_mutex_lock(&m_mutex);
	full = (m_items.size() == m_maxItems);
	pthread_mutex_unlock(&m_mutex);
	return full;
}

template <calss T>
bool PosixQueue<T>::isEmpty()
{
	bool empty;
	pthread_mutex_lock(&m_mutex);
	empty = (m_items.size() == 0);
	pthread_mutex_unlock(&m_mutex);
	return empty;
}

template <calss T>
int PosixQueue<T>::queueStatus()
{
	unsigned int ret = m_items.size();
	if(ret >= m_maxItems -1)
	{
		ret = -1;
	}
	return ret;
}

template <calss T>
void PosixQueue<T>::abort()
{
	pthread_cond_t abort_condition;
	pthread_cond_init(&abort_condition, NULL);

	T* item = NULL;

	struct timespec timed;
	getTimeout(20, timed);

	if(pthread_mutex_lock(&m_mutex))
	{
		return;
	}

	m_aborting = true;

	pthread_cond_broadcast(&m_canPopConditions);
	pthread_cond_broadcast(&m_canPushCondition);

	while(m_poperWaitingCount>0 || m_pusherWaitingCount>0)
	{
		pthread_cond_timedwait(&abort_condition,
			&m_mutex,
			&timed);
	}

	while(!m_items.empty())
	{
		item = m_items.front();
		m_items.pop_front();
		delete item;
	}
	m_items.clear();

	pthread_mutex_unlock(&m_mutex);
}

template <calss T>
int PosixQueue<T>::getTimeout(int timeout, struct timespec& timed)
{
	if(timeout != -1)
	{
		struct timeval now;
		if(gettimeofday(&now, NULL))
		{
			return -1;
		}

		now.tv_usec += timeout* 1000;
		if(now.tv_usec >= 1000000)
		{
			now.tv_sec += now.tv_usec / 1000000;
            now.tv_usec = now.tv_usec % 1000000;
		}

		timed.tv_sec  = now.tv_sec;
        timed.tv_nsec = now.tv_usec * 1000;
	}
	return 0;
}

template <class T>
GenericQueue<T> *GenericQueue<T>::CreateInstance(int max_items)
{
	return new PosixQueue<T>(max_items);
}

#endif