/************************************
生产者消费者，多线程实现问题

有一个存储的队列
	生产者生产
	消费者取消费

1：注意：结构体作为参数，&是引用传递 *是地址传递
2：互斥锁的实现和条件变量
************************************/

#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
using namespace std;

//使用循环队列来进行存储消息
typedef struct buffer
{
	int buffer[10];
	size_t read_positiosn;
	size_t write_position;

	//采用互斥锁的方式实现生产和消费的异步取
	mutex mtx;

	condition_variable not_full; //可写信号
	condition_variable not_empty; //可读信号
	buffer()
	{
		read_position = 0;
		write_position = 0;
	};
}ring_buffer;

ring_buffer save_buffer;

void produce_item(ring_buffer &buffer, int i)
{
	//把i放入buffer中 要加锁
	unique_lock<mutex> lock(buffer->mtx);

	//根据标志，来等待或者存
	while((buffer->write_position+1)%10 == buffer->read_position)
	{
		cout<<"buffer is full now, producer is waiting \n"<<endl;
		(buffer->not_full).wait(lock);//等待释放锁的信号
	}

	//生产者加入数据
	(buffer->buffer)[buffer->write_position] = i;
	buffer->write_position++;
	if(buffer->write_position == 10)
		buffer->write_position = 0;

	//给消费者发信号
	(buffer->not_empty).notify_all();
	lock.unlock();
}


int consumer_item(ring_buffer &buffer)
{
	//从buffer中取值返回 要加锁
	int data;
	unique_lock<mutex> lock(buffer->mtx);
	while(buffer->read_position == buffer->write_position)
	{
		cout<<"buffer is empty, consumer is waiting";
		(buffer->not_empty).wait(lock);
	}

	data = (buffer->buffer)[buffer->read_position];
	buffer->read_position ++;

	if(buffer->read_position >= 10)
	{
		buffer->read_position = 0;
	}

	(buffer->not_full).notify_all();
	lock.unlock();
	return data;
}



//负责生产消息，放入队列中//根据写标志位写在对应位上
void producer()
{
	for(int i=0; i<100; i++)
	{
		produce_item(&save_buffer, i);
	}
}

void consumer()
{
	static cnt = 0;
	while(1)
	{
		sleep(1);
		int item = consumer_item(&save_buffer);
		cout<<"consumer item is:"<<item<<endl;
		if(++cnt == 100)
		{
			break;
		}
	}
}


int main()
{

	thread produce(producer);
	thread consumes(consumer);
	produce.join();
	consumes.join();
	getchar();
	return 0;
}
