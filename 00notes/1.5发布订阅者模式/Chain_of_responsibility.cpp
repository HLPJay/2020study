#ifndef CHAIN_OF_RESPOSIBILITY_H
#define CHAIN_OF_RESPOSIBILITY_H

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <cstdlib>
#include <time.h>

namespace bank{

enum LoanResult_t{
	LR_FAILED = 0,
    LR_OK     = 1,
    LR_OTHER  = 2
};

class Request
{
public:
	Request(const std::string &name, int age, int loan)
	:_name(name), _age(age), _loan(loan)
	{}
	~Request(){};

	int getloan()
	{
		return _loan;
	}

	int getInfo()
	{
		return _age;
	}

	void print_info()
	{
		std::cout<<"custom name:"<<_name<<std::endl;
		std::cout<<"custom age:"<<_age<<std::endl;
		std::cout<<"custom laon:"<<_loan<<std::endl;
	}
private:
	std::string _name;
	int _age;
	int _loan;
};

class Response
{
public:
	Response(LoanResult_t ret, const std::string &msg)
	:_loanResult(ret), _resultMsg(msg)
	{}
	~Response(){};

	void echo(){
        printf("lona result is %d, and msg is %s\n", _loanResult, _resultMsg.c_str());
    }
private:
	int _loanResult;
	std::string _resultMsg;
};

class handler{
public:
	handler(const std::string &name, const std::string &title, handler* next)
	{
		person_name = name;
		title_name = title;
		handler_num_ = 0;
		refuse_num_ = 0;
		forward_num_ = 0;
		setNext(next);
	}
	virtual ~handler() {}
	virtual bool has_permission(Request *req) = 0;
	virtual int getAllowloan() = 0;

	Response* handleMessage(Request *request)
	{
		//经过这里处理只是没有权限
		if(1 == has_permission(request))
		{
			handler_num_++;
			std::cout<<"now handling busing is "<<person_name <<",title name is"<<title_name<<"handler_num: "<<handler_num_<<std::endl;
			return new Response(LR_OK, "You can get money three days later.");
		}
		else
		{
			if(getNext()!=NULL)
			{
				handler_num_++;
				std::cout<<"loan amout out of my Amount of permissions(my permissions="<<getAllowloan();
                std::cout<<"),now give next handler to deal!"<<std::endl;
                return getNext()->handleMessage(request);
			}
			else
			{
				refuse_num_++;
				return new Response(LR_FAILED, "result: sorry!loan amout out of my bank permission.");
			}
		}
	}

	void print_bank_info(){
        std::cout<<"person name:"<<person_name<<std::endl;
        std::cout<<"title name:"<<title_name<<std::endl;
        std::cout<<"handle num:"<<handler_num_<<std::endl;
        std::cout<<"refuse num:"<<refuse_num_<<std::endl;
        std::cout<<"forward num:"<<forward_num_<<std::endl;
    }
protected:
	std::string person_name;
	std::string title_name;
	int  handler_num_; //执行的次数
	int  refuse_num_;  //拒绝的次数
	int  forward_num_;

	handler* nextHandler;
	void setNext(handler *hand)
	{
		nextHandler = hand;
	}

	handler* getNext(void)
	{
		return nextHandler;
	}

};

// class Employee :public handler{
// public:
// 	Employee(const std::string &name,const std::string &title, handler* next)
// 		:handler(name, title, next)
// 	{}
// 	~Employee(){}
// 	bool has_permission(Request *req)
// 	{
// 		if(req->getloan()<getAllowloan())
// 		{
// 			return 1;
// 		}
// 		return 0;
// 	}

// 	int getAllowloan()
// 	{
// 		return this->MAX_LOAN;
// 	}

// protected:
// 	const int MAX_LOAN = 100000;

// };

class BankManager: public handler{
public:
	BankManager(const std::string &name, const std::string &title, handler* next)
		:handler(name, title, next)
	{}
	~BankManager(){}

	bool has_permission(Request *req)
	{
		if(req->getloan()<getAllowloan())
		{
			return 1;
		}
		return 0;
	}

	int getAllowloan()
	{
		return this->MAX_LOAN;
	}
protected:
	const int MAX_LOAN = 1000000;
};

class CityBankManager: public handler{
public:
	CityBankManager(const std::string &name, const std::string &title, handler* next)
		:handler(name, title, next)
	{}
	~CityBankManager(){}

	bool has_permission(Request *req)
	{
		if(req->getloan()<getAllowloan())
		{
			return 1;
		}
		return 0;
	}

	int getAllowloan()
	{
		return this->MAX_LOAN;
	}
protected:
	const int MAX_LOAN = 10000000;
};

class ProvinceBankManager: public handler{
public:
	ProvinceBankManager(const std::string &name, const std::string &title, handler* next)
		:handler(name, title, next)
	{}
	~ProvinceBankManager(){}

	bool has_permission(Request *req)
	{
		if(req->getloan()<getAllowloan())
		{
			return 1;
		}
		return 0;
	}

	int getAllowloan()
	{
		return this->MAX_LOAN;
	}
protected:
	const int MAX_LOAN = 100000000;
};

class TopBankManager: public handler{
public:
	TopBankManager(const std::string &name, const std::string &title, handler* next)
		:handler(name, title, next)
	{}
	~TopBankManager(){}

	bool has_permission(Request *req)
	{
		if(req->getloan()<getAllowloan())
		{
			return 1;
		}
		return 0;
	}

	int getAllowloan()
	{
		return this->MAX_LOAN;
	}
protected:
	const int MAX_LOAN = 1000000000;
};

}

#endif


using namespace bank;
int main(int argc, char** argv)
{
	if(argc<2)
	{
		printf("error of input.\n");
		return false;
	}
	int loan = 0;
	if(argc == 2)
	{
		printf("get number loan is %s\n", argv[1]);
		loan = atoi(argv[1]);
	}
	else
	{
		srand(time(NULL));
		loan = rand()%10001000+1;
		printf("use random data is  %d",loan);
	}
//测试request
	Request *req = new Request("hlp", 28, loan);
	printf("----------------------------------\n");
	printf("get the custom request:\n");
	req->print_info();
	printf("----------------------------------\n");

	//构造责任链的初始化
	TopBankManager *top =new TopBankManager("top", "TopBankManager", nullptr);
	ProvinceBankManager *province = new ProvinceBankManager("province", "ProvinceBankManager", top);
	CityBankManager *city = new CityBankManager("city", "CityBankManager", province);
	BankManager *bank = new BankManager("bank", "BankManager", city);
	bank->print_bank_info();
	city->print_bank_info();
	province->print_bank_info();
	top->print_bank_info();


	Response *rep = bank->handleMessage(req);
	if(rep != nullptr)
	{
		rep ->echo();
	}
	bank->print_bank_info();
	city->print_bank_info();
	province->print_bank_info();
	top->print_bank_info();

	delete(rep);
	delete(bank);
	delete(city);
	delete(province);
	delete(top);
	return 0;
}
