#include <iostream>
#include <fstream>
#include <stdio.h>
#include "InflateStreamTest.h"

#define DATA_SIZE 1024
int decompressOneFile(char *infilename, char *outfilename);
int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cout << "usage: " << argv[0] << ": <input_file> <output_file>" << std::endl
		          << "       read <input_file>, deflate (compress) it and write the result to <output_file>" << std::endl;
		return 1;
	}

	decompressOneFile(argv[1], argv[2]);
	return 0;
}

int decompressOneFile(char *infilename, char *outfilename)
{
	//打开要读出的文件和  要写入的文件
	int numRead = 0;
    char inbuffer[DATA_SIZE] = {0};
    FILE *infile = fopen(infilename, "rb");
    FILE *outfile = fopen(outfilename, "wb");
    std::string outStr; 
    if (!infile || !outfile )
    {
        return -1;
    }

    InflateStream *infalte = NULL;
    try
    {
        infalte = new InflateStream(InflateStream::STREAM_GZIP);
    }
    catch (const char* c)
	{
		printf(" new InflateStream failed, it is %s \n", c);
        if(infalte)
            delete infalte;
        return -1;
	}

	//读出文件 然后调用接口解压出 然后写入文件
    while ((numRead = fread(inbuffer, 1, sizeof(inbuffer), infile)) > 0)
    {
        outStr.clear();
        int ret = infalte->Inflate(inbuffer, numRead, outStr);
        printf("%s(%d) ret:%d, outStr size:%ld\n", __FUNCTION__, __LINE__, ret, outStr.size());
        fwrite(outStr.c_str(), outStr.size(), 1, outfile);
    }
    outStr.clear();
    int ret = infalte->Inflate(NULL, 0, outStr);
    printf("%s(%d) ret:%d, outStr size:%ld\n",  __FUNCTION__, __LINE__, ret, outStr.size());
    fwrite(outStr.c_str(), outStr.size(), 1, outfile);

    fclose(infile);
    fclose(outfile);

    delete infalte;
    
    return 0;
}