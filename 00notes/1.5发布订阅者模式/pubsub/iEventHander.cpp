#include "event.h"
#include "iEventHander.h"
#include "DispatchMsgService.h"

#include <stdio.h>
LoginEventHandler::LoginEventHandler()
{

}
LoginEventHandler::~LoginEventHandler()
{

}

void LoginEventHandler::init()
{
	 dms->subscribe(EID_LOGIN_REQ, this);
}

void LoginEventHandler::uninit()
{
	dms->unsubscribe(EID_LOGIN_REQ, this);
}

bool LoginEventHandler::handler(const iEvent* ev)
{
	printf("enter handle method. \n");

	if(EID_LOGIN_REQ == ev->getEid())
	{
		LoginEvent *req = (LoginEvent*)ev;
		printf("usr(%s)'s login success.\n", req->getName().c_str());
	}
	else
	{
		printf("unsubscribe events\n");
	}
	return true;
}