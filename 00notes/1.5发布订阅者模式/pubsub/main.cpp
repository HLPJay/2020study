#include <stdio.h>
#include <unistd.h>

#include "event.h"
#include "DispatchMsgService.h"
#include "iEventHander.h"

class user
{
public:
	bool login()
	{
		printf("try to again. \n")
		char msg[] = "{\"cmd\":\"login\",\"sn\":1,\"name\":\"pan jinlian\",\"password\":\"xiao neiku\"}";
		LoginEvent* ev = new LoginEvent(msg);
		ev->decode();//塞入订阅信息

		dms->publish(ev);
		return true;
	}
	user(){}
	virtual ~user() {}
};

int main(int argc, char** argv)
{
	dms->init();

	LoginEventHandler handler;
	hanler.init();

	user pjl;
	pjl.login();

	for(;;)
    {
        sleep(1);
    }

    return 0;
}