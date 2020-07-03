#include <iostream>
#include <map>
#include <fstream>
using namespace std;
map<std::string, std::map<std::string, std::string>> m_map;
/******************************
配置文件解析类的测试：
	1：读取配置文件
	2：按照不同的数据类型对不同的接口进行解析
原理：
	实际上还是按行读取，解析字符串。
涉及知识点：
	fstream
	ifstream
	ofstream
	getline函数
  
  string::npos
*******************************/

void Trim(std::string &str)
{
    if(str.empty())
    {
        return ;
    }
    str.erase(0,str.find_first_not_of(" "));
    str.erase(0,str.find_first_not_of("\t"));
    str.erase(str.find_last_not_of(" ") + 1);
    str.erase(str.find_last_not_of("\t") + 1);
	
}
/*
void Config::Trim( string& inout_s )
{
    // Remove leading and trailing whitespace
    static const char whitespace[] = " \n\t\v\r\f";
    inout_s.erase( 0, inout_s.find_first_not_of(whitespace) );
    inout_s.erase( inout_s.find_last_not_of(whitespace) + 1U );
}
*/

/****************************************
按行对文件内部数据进行解析：
	1：判断是否有注释符号#并调整字符
	2：根据[]解析获取头信息
	3：如果没有[]，解析=前后获取参数和值。
主要知识点：
	使用substr对每行数据进行切割提取子串
****************************************/
int ParseEveryLine(std::string &line, std::string &section, std::string &key, std::string &value)
{
    if(line.empty())
    {
        return -1;
    }
    //解析#，
    int start_ops = 0;
    int end_pos = line.size();
    int pos;
    if((pos = line.find('#')) != -1)
    {
        if(pos == 0)
        {
            return -1;
        }
        //行尾#
        end_pos = pos ;
    }

    //解析最外层 []
    int s_ops, e_ops;
    if(((s_ops = line.find("[")) != -1 )&&
        ((e_ops = line.find("]")) != -1))
    {
        section = line.substr(s_ops+1, e_ops - s_ops-1);
        return 0;
    }
    //解析正常的=逻辑
    //切掉# 获取到要解析的正常数据 做=切割
    std::string new_str = line.substr(start_ops, end_pos-start_ops);
    if((pos = new_str.find("=")) == -1)
    {
        return -1;
    }
    //获取key 获取value
    key = new_str.substr(start_ops, pos);
    value = new_str.substr(pos+1, end_pos - pos);

    Trim(key);
    // replace(key.begin(), key.end(), ' ', ' '); //只能替换一个字符
    if(key.empty())
    {
        return -1;
    }
    Trim(value);
    if ((pos = value.find("\r")) > 0)
    {
        value.replace(pos, 1, "");
    }
    if ((pos = value.find("\n")) > 0)
    {
        value.replace(pos, 1, "");
    }
    if ((pos = value.find("\t")) > 0)
    {
        value.replace(pos, 1, "");
    }
    return 1;

}
bool ReadConfigFile(const std::string &filename)
{
    m_map.clear();
    if(filename.empty())
    {
        return false;
    }
    //其实内部与open关联
    /*************************************
    其实有第二个参数，默认是ios_base::in
        app 	每次写入前寻位到流结尾
        binary 	以二进制模式打开
        in 	为读打开
        out 	为写打开
        trunc 	在打开时舍弃流的内容
        ate 	打开后立即寻位到流结尾
    **************************************/
    std::ifstream infile(filename.c_str());
    if(!infile.is_open())
    {
        return false;
    }
    //按行读取
    std::string line, section, key, value;
    while(getline(infile, line))
    {
        //根据行数据进行解析 1:包含[] ==》取section  2 包含=且key不为空返回成功 ==》取section对应的其他值
        if(ParseEveryLine(line, section, key, value) == 0)
        {

        }

        if(ParseEveryLine(line, section, key, value) == 1)
        {
            //有则加入，无则新增
            if(m_map.find(section) != m_map.end())
            {
                // m_map[section][key] = value;
                m_map[section].insert(std::make_pair(key, value));
            }else
            {
                std::map<std::string,std::string> temp;
                temp.insert(std::make_pair(key, value));

                m_map.insert(std::make_pair(section, temp));
            }
        }

    }
    infile.close();
    return true;
}void Mytestofconfig()
{
    ReadConfigFile("config.ini");
    for(auto iter= m_map.begin();iter!=m_map.end();iter++)
    {
        cout<<iter->first<<":"<<endl;
        for(auto it = iter->second.begin();it!=iter->second.end();it++)
        {
            cout<<"key="<<it->first<<"  value="<<it->second<<endl;
        }
    }
}

	
