/**********************************
iconv的简单使用测试,观察是否能转换成功.

主要用到三个函数 :
	iconv_open
	iconv
	iconv_close
//需要找找源代码看看

utf8:e5 bb 96 e5 ba 86 e5 af 8c 64 61 72 72 65 6e e8 80 81 e5 b8 88 
before ==>srcbuff = 0x7ffcde6ec020, desbuff = 0x7ffcde6ebbc0, no conv = 21, remainSpace = 1024
end    ==>srcbuff = 0x7ffcde6ec035, desbuff = 0x7ffcde6ebbd8, no conv = 0, outsize= 24, remainSpace = 1000
utf16:ff fe d6 5e 86 5e cc 5b 64 00 61 00 72 00 72 00 65 00 6e 00 01 80 08 5e 
UTF8编码：E5BB96E5BA86E5AF8C64617272656EE88081E5B888
UTF16LE编码：FFFED65E865ECC5B640061007200720065006E000180085E

Unicode编码：00005ED600005E8600005BCC00000064000000610000007200000072000000650000006E0000800100005E08
UTF8编码：E5BB96E5BA86E5AF8C64617272656EE88081E5B888
UTF16BE编码：FEFF5ED65E865BCC00640061007200720065006E80015E08
UTF16LE编码：FFFED65E865ECC5B640061007200720065006E000180085E
UTF32BE编码：0000FEFF00005ED600005E8600005BCC00000064000000610000007200000072000000650000006E0000800100005E08
UTF32LE编码：FFFE0000D65E0000865E0000CC5B000064000000610000007200000072000000650000006E00000001800000085E0000
**********************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>

int main()
{
	//定义目的编码和原编码
	unsigned char* encTo = "UTF-16";
	unsigned char* encFrom = "UTF-8";
	//确定转换 返回iconv_t句柄 ???
	iconv_t cd = iconv_open(encTo, encFrom);
    if (cd == (iconv_t)-1)
    {
        perror("iconv_open");
    }

    //需要转换的字符串 
    unsigned char inbuf[1024] = "廖庆富darren老师"; // 21
    size_t srclen = strlen(inbuf);
    printf("srclen=%ld\n", srclen);

    //输出一下源文件的utf8内容:
    printf("utf8:");
    int i=0;
    for (i = 0; i < strlen(inbuf); i++)
    {
        printf("%02x ", inbuf[i]);
    }
    printf("\n");

    // 存放转换后的字符串 
    size_t outlen = 1024;
    unsigned char outbuf[outlen];
    size_t utf16_len = outlen;
    memset(outbuf, 0, outlen);

//iconv会修改参数指针
    char * srcbuff =(char * ) inbuf;
    char * desbuff = (char * )outbuf;
//进行转换,   
    //句柄  要转换的字符  剩余没有转的   转换后的字符    转换后剩余的空间
    printf("before ==>srcbuff = %p, desbuff = %p, no conv = %ld, remainSpace = %ld\n", srcbuff,desbuff, srclen, outlen);
    size_t ret = iconv(cd, &srcbuff, &srclen, &desbuff, &outlen);
    if (ret == -1)
    {
        perror("iconv");
    }
    utf16_len = utf16_len - outlen;
    printf("end    ==>srcbuff = %p, desbuff = %p, no conv = %ld, outsize= %ld, remainSpace = %ld\n", srcbuff,desbuff, srclen, utf16_len, outlen);
	
    printf("utf16:");

    for (i = 0; i < utf16_len; i++)
    {
        printf("%02x ", outbuf[i]);
    }
    printf("\n");
    /* 关闭句柄 */
    iconv_close(cd);

	return 0;
}
