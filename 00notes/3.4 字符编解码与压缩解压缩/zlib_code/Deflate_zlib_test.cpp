/********************************************
使用zlib库对压缩和解压进行测试：
	1：zlib的下载安装
		wget http://www.zlib.net/zlib-1.2.11.tar.gz
		tar -zxvf zlib-1.2.11.tar.gz
		cd zlib-1.2.11
		./configure
		make
		make check ===>
		sudo make install
	2:通过zlib.h了解相关接口api
		1：实际接口zconf.h
		2：使用了一种压缩方式，其他的有接口
			deflate流
	3：知识点：
		1：__declspec关键字
		2：far CDECL  //与指针  存储相关
		3：typedef unsigned (*in_func) OF((void FAR *,
                                z_const unsigned char FAR * FAR *));的使用
*********************************************/

#include "DeflateStreamTest.h"

#include <iostream>
#include <fstream>
#include <stdio.h>

//g++ -o deflate DeflateStreamTest.h DeflateStreamTest.cpp zlib_test.cpp -lz

int compressOneFile(char *infilename, char* outfilename);
int main(int argc, char** argv)
{
	if(argc != 3)
	{
		std::cout << "usage: " << argv[0] << ": <input_file> <output_file.gz>" << std::endl
		          << "       read <input_file>, deflate (compress) it and write the result to <output_file>" << std::endl;
		return 1;
	}
	compressOneFile(argv[1], argv[2]);
	return 0;
}

#define DATA_SIZE 1024
int compressOneFile(char *infilename, char* outfilename)
{
	//打开要压缩的文件和目标文件
	int numRead = 0;
    char inbuffer[DATA_SIZE] = {0};
    FILE *infile = fopen(infilename, "rb");
    FILE *outfile = fopen(outfilename, "wb");
    std::string outStr; 
    if (!infile || !outfile )
    {
        return -1;
    }

    DeflateStream * defalte = NULL;
    try
    {
    	//Z_DEFAULT_COMPRESSION 按照默认级别
        defalte = new DeflateStream(STREAM_GZIP, Z_DEFAULT_COMPRESSION, std::string(infilename));
    }
    catch (const char* c)
	{
		printf(" new DeflateStream failed, it is %s \n", c);
        if(defalte)
            delete defalte;
        return -1;
	}
	//读文件
	while ((numRead = fread(inbuffer, 1, sizeof(inbuffer), infile)) > 0)
    {
        outStr.clear();
        int ret = defalte->Deflate(inbuffer, numRead, outStr); // 压缩数据
        printf("%s(%d) ret:%d, outStr size:%ld\n", __FUNCTION__, __LINE__, ret, outStr.size());
        // 写入压缩数据
        if(outStr.size() > 0)
            fwrite(outStr.c_str(), outStr.size(), 1, outfile);
    }
    outStr.clear();
    int ret = defalte->Deflate(NULL, 0, outStr);
    printf("%s(%d) ret:%d, outStr size:%ld\n",  __FUNCTION__, __LINE__, ret, outStr.size());
    fwrite(outStr.c_str(), outStr.size(), 1, outfile);

    fclose(infile);
    fclose(outfile);

    delete defalte;

    return 0;
    
}