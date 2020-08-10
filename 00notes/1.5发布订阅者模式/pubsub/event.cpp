#include "event.h"
#include "eventid.h"

#include <iostream>

#include <sstream>
#include <json/json.h>


LoginEvent::LoginEvent(char *buffer)
	:iEvent(buffer)
{}

std::ostream& LoginEvent::dump(std::ostream& out) const
{
	out << "LoginEvent event: sn = "<< getSN()<<",";
	out <<"eid = "<<getEid() <<",";
	out <<"name = "<< name_ <<",";
	out <<"pwd = "<< pwd_<<"\n";
	return out;
}

bool LoginEvent::decode()
{
	Json::Reader reader;
	Json::Value attributes;

	if(!read.parse(content_, attributes))
	{
		printf("parse Json content(%s) failed.", content_.c_str());
		return false;
	}

	if((attributes["cmd"] != Json::nullValue && attributes["cmd"].isString()))
	{
		if(attributes["cmd"].asString().compare("login")!=0)
		{
			return false;
		}
		setEid(EID_LOGIN_REQ);
	}

	if((attributes["sn"] != Json::nullValue && attributes["sn"].isInt()))
	{
		setSN(attributes["sn"].asInt());
	}

	if(attributes["name"] != Json::nullValue && attributes["name"].isString())
	{
		name_ = attributes["name"].asString();
	}

	if(attributes["password"] != Json::nullValue && attributes["password"].asString())
	{
		pwd_ = attributes["password"].asString();
	}
	return true;
}

void LoginEvent::encode()
{
	return NULL;
}
