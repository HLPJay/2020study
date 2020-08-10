#ifndef NS_IEVENT_HANDER_H
#define NS_IEVENT_HANDER_H

#include "event.h"

class iEventHandler
{
public:
	virtual bool handler(const iEvent* ev) = 0;

	iEventHandler(){};
	virtual ~iEventHandler() {};
};



class LoginEventHandler : public iEventHandler
{
public:
	LoginEventHandler();
	virtual ~LoginEventHandler();
	void init();
	void uninit();
	virtual bool handler(const iEvent* ev);
};
#endif