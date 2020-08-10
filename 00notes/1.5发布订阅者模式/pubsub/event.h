#ifndef NS_EVENT_H
#define NS_EVENT_H

#include "glo_def.h"

#include <string>
#include <map>
#include <vector>

class iEvent
{
public:
	iEvent(char * content):context_(content)
	{}
	~iEvent(){};
	virtual u16 getEid() const {return eid_; };
	virtual void setEid(u16 eid) {eid_ = eid};
	virtual u32 getSN() const {return sn_;};
	virtual void setSN(u32 sn){sn_ = sn};

	virtual std::iostream& dump(std::ostream& out) const
	{
		return out;
	}
	virtual bool decode() {return true;};
	virtual void encode() {return NULL;};

private:
	u16    eid_;
	u32    sn_;
protected:
	std::string content_;
};


class LoginEvent: public iEvent
{
public:
	LoginEvent(char *buffer);
	virtual ~LoginEvent(){};

	virtual std::ostream& dump(std::ostream& out) const;

	virtual bool decode();
	virtual void encode();

	const std::string& getName() {
		return name_;
	}
	const std::string& getPwd()
	{
		return pwd_;
	}
private:
	std::string name_;
	std::string pwd_;
};

#endif