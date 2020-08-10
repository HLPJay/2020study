#include "DispatchMsgService.h"

#include <stdio.h>
#include <algorithm>
#include <isotream>
#include <sstream>


DispatchMsgService DispatchMsgService::instance_;


DispatchMsgService::DispatchMsgService()
	:msg_queue_(PosixQueue<iEvent>(65536))
{

}

DispatchMsgService::~DispatchMsgService()
{

}

bool DispatchMsgService::init()
{
	int ret=pthread_create(&thread_hdl_, NULL, svc, this);
	if(ret != 0)
	{
		printf("create pthread error.\n");
		return false;
	}
	ser_exit_ = false;
	return true;
}

bool DispatchMsgService::uninit()
{
	ser_exit_ = true;
	pthread_join(thread_hdl_, NULL);
	return true;
}

bool DispatchMsgService::subscribe(u16 eid, iEventHandler* handler)
{
	if(NULL== handler)
	{
		return false;
	}
	T_EventHandlersMap::iterator iter = subscribes_.find(eid);
	if(iter != subscribes_.end())
	{
		T_EventHandlers::iterator handler_iter = std::find(iter->second.begin(), iter->second.end(), handler);
		if(handler_iter == iter->second.end())
		{
			iter->second.push_back(handler);
		}
	}
	else
	{
		subscribes_[eid].push_back(handler);
	}
	return true;
}

bool DispatchMsgService::unsubscribe(u16 eid, iEventHandler* handler)
{
	if(NULL == handler)
	{
		return false;
	}

	T_EventHandlersMap::iterator iter = subscribes_.find(eid);
	if(iter != subscribes_.end())
	{
		T_EventHandlers::iterator handler_iter = std::find(iter->second.begin(), iter->second.end(), handler);
		if(handler_iter != iter->second.end())
		{
			iter->second.erase(handler_iter);
		}
	}
	return true;
}

bool DispatchMsgService::publish(iEvent *ev)
{
	if(NULL == ev)
	{
		return false;
	}
	return msg_queue_.enqueue(ev, 0);
}

void *DispatchMsgService::svc(void *argv)
{
	printf("dms is running\n");

	DispatchMsgService* dmsvr = (DispatchMsgService*) argv;
	if(NULL == argv)
	{
		printf("parameter of thread is invilid \n");
		return NULL;
	}

	while(!dmsvr->ser_exit_)
	{
		iEvent* ev = NULL;

		if(-1 == dmsvr->msg_queue_.dequeue(ev, 1))
		{
			continue;
		}

		std::stringstream ss;
		ev->dump(ss);
		printf("dequeue an event(%s).\n", ss.str().c_str());

		dmsvr->process(ev);
		delete ev;
	}
}
bool DispatchMsgService::process(const iEvent* ev)
{
	if(NULL == ev)
	{
		return false;
	}

	u16 eid = ev->getEid();

	if(eid == EID_UNKOWN)
	{
		printf("event is invalid.");
		return false;
	}

	T_EventHandlersMap::iterator handlers = subscribes_.find(eid);
	if(handlers == subscribes_.end())
	{
		printf("DispatchMsgService ：no event handlers.\n");
		return false;
	}

	for(T_EventHandlers::iterator iter = handlers->second.begin();
		iter!=handlers->second.end();
		iter++)
	{
		iEventHandler* handler = *iter;
		handler->handle(ev);
	}
	return true;
}