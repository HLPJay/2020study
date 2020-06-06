/******************************8
通过代码实现:	
	1:LZ77+构建哈夫曼树
	2:压缩数据
	3:字符以及字符出现的次数写入文件
	4:利用配置文件构造哈夫曼树进行解压缩

相关知识点:
	读文件和写文件
	priority_queue 以及其第三个参数
*******************************/
#include <iostream>
using namespace std;

#include "HuffManTree.h"
#include <iostream>
#include <string>

#include <vector>
#include <assert.h>
//未理解?
unsigned long getFileSize(const char *path)
{
	unsigned long filesize = -1;
	FILE *fp;
	fp = fopen(path, "r");
	if(fp == NULL)
		return filesize;
	fseek(fp, 0L, SEEK_END);
	filesize = ftell(fp);
	fclose(fp);
	return filesize;
};



//构建节点 ==>存储字符  出现的次数  以及编码
typedef unsigned char charType;
typedef unsigned int intType;
struct CharInfo
{
	charType m_ch;
	intType m_count;
	std::string m_code;

	bool operator!=(const CharInfo &info)
    {
        return m_count != info.m_count;
    }
    CharInfo operator+(const CharInfo &info)
    {
        CharInfo ret;
        ret.m_count = m_count + info.m_count;
        return ret;
    }
    bool operator<(const CharInfo &info)
    {
        return m_count > info.m_count;
    }

};
/*********************************
//我们调用的接口类
	1:获取哈夫曼对应的编码
	2:进行压缩
	3:进行解压
*********************************/
class FileCompress
{
	struct TmpInfo //为了写入文件中
    {
        charType m_ch;//字符
        intType m_count;//次数
    };
public:
	FileCompress()
	{//所有字符 初始化为0
		for(int i=0; i<256; i++)
		{
			m_infos[i].m_ch = i;
            m_infos[i].m_count = 0;
		}
	}
	~FileCompress(){}

public:
	typedef HhffManTreeNode<CharInfo> Node;
	//遍历哈夫曼树 取编码存入m_infos中 字符  出现的次数以及编码
	void GetHuffManCode(Node *root, string code)
	{
		if (root == NULL)
            return;
        //前序遍历生成编码
        if (root->m_pLeft == NULL && root->m_pRight == NULL)
        {
            m_infos[(unsigned char)root->m_weight.m_ch].m_code = code;
            return;
        }
        GetHuffManCode(root->m_pLeft, code + '0');
        GetHuffManCode(root->m_pRight, code + '1');
	}
	/*************************************
	//压缩 未优化 直接全部压缩 ==>c传入的是文件名
	打开源文件,遍历每个字符,统计个数
	构建哈夫曼树
	根据哈夫曼树生成编码
	把生成哈夫曼树的 碼表存入 ==>存入结束标识
	取字符,查找对应的编码存入文件中
	**************************************/
	void Compress(const char* file)
	{
		unsigned long fileSize = getFileSize(file);
		//代开源文件,获取字符次数
		FILE *fout = fopen(file, "rb");
		assert(fout);
		char ch = fgetc(fout);
		while (feof(fout) == 0) //如文件结束，则返回值为1，否则为0
        {
            m_infos[(unsigned char)ch].m_count++;     // 计算对应字符出现的频率
            ch = fgetc(fout);
        }

        //构造哈夫曼树,第三个参数这里无用,可优化
        CharInfo invalid;
        invalid.m_count = 0;
        HuffManTree<CharInfo> tree(m_infos, 256, invalid);

        //创建文件并打开,打开时白就不必了 ==>源文件名加后缀
        string compressfile = file;     
        compressfile += ".huffman";  
        FILE *fin = fopen(compressfile.c_str(), "wb"); //打开压缩文件
        assert(fin);

        //参数为了递归取编码
        string temp_code;
        GetHuffManCode(tree.GetRoot(), temp_code);

        //新打开的文件中写入编码频率
        int writeNum = 0;
        int objSize = sizeof(TmpInfo);
        for (size_t i = 0; i < 256; ++i)            // 这里实质是讲码表存储到文件的头部
        {
            if (m_infos[i].m_count > 0) 
            {
                TmpInfo info;
                info.m_ch = m_infos[i].m_ch;
                info.m_count = m_infos[i].m_count;
                printf("codec ch:%u, cout:%u\n", (unsigned char)info.m_ch, info.m_count);
                fwrite(&info, objSize, 1, fin);         // 将对应的码流信息写入
                writeNum++;
            }
        }
        //写入终结符号
        TmpInfo info;
        info.m_count = -1;
        printf("code objSize:%d\n", objSize);
        fwrite(&info, objSize, 1, fin); //把info._count = -1写进去作为结束标志位
        
        //把源文件字符对应的编码写入,实现压缩的效果
        //按位写入   构造1个字节写入
        int writeCount = 0;
        int readCount = 0;
        fseek(fout, 0, SEEK_SET); //文件指针、偏移量、参照位置
        ch = fgetc(fout);
        readCount++;
        unsigned char value = 0;
        size_t pos = 0;
        while (feof(fout) == 0)           // 一个个字符读取,效率虽然低一些，但不影响实验
        {
            // 读取数据，查找对应编码
            string &code = m_infos[(unsigned char)ch].m_code; // 查找对应的编码
            printf("code[%d]:%u:%s\n", readCount, (unsigned char)ch, code.c_str());
            // code 实际存储的是对应 某个字符的哈夫曼编码
            for (size_t i = 0; i < code.size(); ++i)
            {
                if (code[i] == '1')
                    value |= (1 << pos);
                else if (code[i] == '0')
                {
                    value &= ~(1 << pos);
                }
                else
                {
                    assert(false);
                    printf("assert(false); ??????????");
                }
                ++pos;
                if (pos == 8)
                {
                    writeCount++;
                    fputc(value, fin);      // 够8个bit存储
                    value = 0;
                    pos = 0;
                }
            }
            readCount++;
            ch = fgetc(fout);
        }

        if (pos > 0)
        {
            writeCount++;
            fputc(value, fin); //最后的数据写入压缩文件（fin）
        }
        //哈夫曼碼表的长度+实际数据的长度
        printf("huffman code table  size::%d\n", objSize * (writeNum + 1));
        printf("compress file data  size:%d\n", writeCount);
        unsigned long totalSize = writeCount + objSize *  (writeNum + 1);
        printf("compress file total size:%lu, orign file size:%lu, ratio:%0.2f\n",
             totalSize, fileSize,  (float)(totalSize*1.0/fileSize));
        fclose(fout);
        fclose(fin);
	}
	void UnCompress(const char *file)
	{
		//更改加压后的文件名  打开要解压的文件  以及要写入的文件
		string uncompressfile = file;           //file:Input.txt.huffman
        size_t pos = uncompressfile.rfind('.'); //找到倒数第一个'.'
        assert(pos != string::npos);
        uncompressfile.erase(pos);                       //删除掉'.'后面字符串
        uncompressfile += ".unhuffman";                  //Input.txt+'.unhuffman'
        FILE *fin = fopen(uncompressfile.c_str(), "wb"); //打开解压缩文件
        assert(fin);
        FILE *fout = fopen(file, "rb"); //打开压缩文件
        assert(fout);

        //3.0 读入碼表
        TmpInfo info;
        int cycleNum = 1;
        int objSize = sizeof(TmpInfo);
        fread(&info, objSize, 1, fout);

        while (info.m_count != -1)           //-1为结束标志 
        {
            //  printf("decodec ch:%u, cout:%u\n", (unsigned char)info._ch, info._count);
            m_infos[(unsigned char)info.m_ch].m_ch = info.m_ch;
            m_infos[(unsigned char)info.m_ch].m_count = info.m_count;

            fread(&info, objSize, 1, fout);
            cycleNum++;
        }
        //重新构造哈夫曼树
        CharInfo invalid;
        invalid.m_count = 0;
        HuffManTree<CharInfo> tree(m_infos, 256, invalid); //参数：数组，256个，无效值（出现0次）
       	
       	Node *root = tree.GetRoot();
        Node *cur = root;
        intType n = root->m_weight.m_count; //所有叶子节点的和（源文件字符的个数） 有些字符没有
        char ch = fgetc(fout);             //从fout(压缩文件)读字符
        int readCount = 0;
        //0往左边找 1往右边找 直到找到最低下
        while (ch != EOF || n > 0)
        {
            for (size_t i = 0; i < 8; ++i)
            {
                if ((ch & (1 << i)) == 0)
                    cur = cur->m_pLeft;   // 往左边找
                else
                    cur = cur->m_pRight; // 往右边找
                if (cur->m_pLeft == NULL && cur->m_pRight == NULL)
                {
                    //cout << cur->_weight._ch;
                    readCount++;
                    if(readCount % 1024 == 0) 
                    {
                        // printf("uncompress %dk data\n", readCount/1024);
                    }
                    fputc(cur->m_weight.m_ch, fin); //fin解压缩文件 写入对应的字符
                    cur = root;
                    if (--n == 0)
                        break;
                }
            }
            ch = fgetc(fout);
        }
        printf("uncompress end\n");
        fclose(fin);
        fclose(fout);
	}
protected:
	CharInfo m_infos[256]; //基本字符
};

int main(int argc, char** argv)
{
	if(argc!=2)
	{
		std::cout << "usage: " << argv[0] << ": <input_file> " << std::endl;
		return -1;
	}
	//把文件压缩后加后缀 .huffman
	FileCompress fc;
    FileCompress fc1;
    std::string fileName(argv[1]);

    fc.Compress(fileName.c_str());      // 压缩文件
    fileName += ".huffman";             // 压缩后的文件名
    fc1.UnCompress(fileName.c_str());   // 解压缩

	return 0;
}
