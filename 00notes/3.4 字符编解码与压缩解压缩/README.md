
# 编码和解压相关源码分析:
### 预期练习:
1. 编码规则相关演示:
2. file -i命令 <br/>
3. iconv命令以及通过iconv相关api实现的代码 ===>查看相关转换的差异<br/>
4. mysql写入特殊字符查看场景 ===>未完成<br/>
5. nginx中特殊字符的场景    ===>未完成.<br/>
6. 代码测试

### 实际练习:
#### 1. 观察不同编码规则文件的差异. <br/>
-  用file -i 来查看文件的格式,编码方式<br/>
```cpp
hlp@hlp:~/2020腾讯课堂/0.github_code/编解码/src/unicode/iconv$ file -i *
GB2312.txt:   text/plain; charset=iso-8859-1
GBK.txt:      text/plain; charset=iso-8859-1
UTF-16BE.txt: application/octet-stream; charset=binary
UTF16BE.txt:  application/octet-stream; charset=binary
UTF-16LE.txt: application/octet-stream; charset=binary
UTF16LE.txt:  application/octet-stream; charset=binary
UTF-16.txt:   text/plain; charset=utf-16le
UTF16.txt:    text/plain; charset=utf-16le
UTF-32BE.txt: application/octet-stream; charset=binary
UTF-32LE.txt: application/octet-stream; charset=binary
UTF-32.txt:   application/octet-stream; charset=binary
utf8.txt:     text/plain; charset=utf-8
UTF-8.txt:    text/plain; charset=utf-8
```
#### 其他知识点:
**MIME Type:**资源的媒体类型===>浏览器区分文件类型用<br/>
&nbsp;&nbsp;  文件的MIME-type是application/octet-stream（二进制文件没有特定或已知的 subtype，即、故使用 application/octet-stream）<br/>
通过**charset**可以了解到，该文件编码方式是二进制.<br/>

- 通过文件的十六进制形式,对比不同编码格式,以及大小端问题.<br/>
提示,ubuntu以十六进制显示文件的方式:
```cpp
1:使用vim
先用vim -b data 以2进制打开文件，然后用xxd工具转化，在vim的命令行模式下：
     :%!xxd        --将当前文本转化为16进制格式
     :%!xxd -r    --将16进制转化为普通文本
hlp@hlp:~/2020腾讯课堂/0.github_code/编解码/src/unicode/iconv$ vim -b UTF16.txt
0000000: fffe d65e 865e cc5b 6400 6100 7200 7200  ...^.^.[d.a.r.r.
0000010: 6500 6e00 0180 085e                      e.n....^
hlp@hlp:~/2020腾讯课堂/0.github_code/编解码/src/unicode/iconv$ vim -b UTF16LE.txt
0000000: d65e 865e cc5b 6400 6100 7200 7200 6500  .^.^.[d.a.r.r.e.
0000010: 6e00 0180 085e                           n....^
hlp@hlp:~/2020腾讯课堂/0.github_code/编解码/src/unicode/iconv$ vim -b UTF16BE.txt
0000000: 5ed6 5e86 5bcc 0064 0061 0072 0072 0065  ^.^.[..d.a.r.r.e
0000010: 006e 8001 5e08                           .n..^.
```
通过观察:可以发现UTF16.txt带标识,默认是小端(高字节显示在高位上)显示<br/>
    
```cpp
2:用hexdump命令  ==>貌似hexdump命令是自带大端显示处理的?
    分页查看可以用:    hexdump data | less  ===>注意这个命令是小端显示
    与ASCII对比可以用: hexdump -C data | less

使用相关命令对UTF16作对比:
hlp@hlp:~/2020腾讯课堂/0.github_code/编解码/src/unicode/iconv$ hexdump UTF16.txt  ==>默认是小端
0000000 feff 5ed6 5e86 5bcc 0064 0061 0072 0072
0000010 0065 006e 8001 5e08                    
0000018
hlp@hlp:~/2020腾讯课堂/0.github_code/编解码/src/unicode/iconv$ hexdump -C UTF16.txt  
00000000  ff fe d6 5e 86 5e cc 5b  64 00 61 00 72 00 72 00  |...^.^.[d.a.r.r.|
00000010  65 00 6e 00 01 80 08 5e                           |e.n....^|
00000018

hlp@hlp:~/2020腾讯课堂/0.github_code/编解码/src/unicode/iconv$ hexdump UTF16BE.txt ===>hexdump小端显示
0000000 d65e 865e cc5b 6400 6100 7200 7200 6500
0000010 6e00 0180 085e                         
0000016
hlp@hlp:~/2020腾讯课堂/0.github_code/编解码/src/unicode/iconv$ hexdump -C UTF16BE.txt ==>不转序存
00000000  5e d6 5e 86 5b cc 00 64  00 61 00 72 00 72 00 65  |^.^.[..d.a.r.r.e|
00000010  00 6e 80 01 5e 08                                 |.n..^.|
00000016

hlp@hlp:~/2020腾讯课堂/0.github_code/编解码/src/unicode/iconv$ hexdump UTF16LE.txt 
0000000 5ed6 5e86 5bcc 0064 0061 0072 0072 0065
0000010 006e 8001 5e08                         
0000016
hlp@hlp:~/2020腾讯课堂/0.github_code/编解码/src/unicode/iconv$ hexdump -C UTF16LE.txt ==>不转序存
00000000  d6 5e 86 5e cc 5b 64 00  61 00 72 00 72 00 65 00  |.^.^.[d.a.r.r.e.|
00000010  6e 00 01 80 08 5e                                 |n....^|
00000016
```

##### 测试:

ABCD对应的编码:<br/>
Unicode编码：00000041000000420000004300000044<br/>
UTF8编码：41424344<br/>
UTF16BE编码：FEFF 0041 0042 0043 0044       ====>大端(低地址存放高有效位)  ==>按顺序存<br/>
UTF16LE编码：FFFE 4100 4200 4300 4400       ====>小端    ==>按字节逆序存<br/>
UTF32BE编码：0000FEFF 0000 0041 0000 0042 0000 0043 0000 0044<br/>
UTF32LE编码：FFFE0000 4100 0000 4200 0000 4300 0000 4400 0000<br/>

**大端模式（Big-endian）**：按顺序存<br/>
	将数据的高位字节(最前面的字节)存储到内存的低地址(数据越前越是低地址)，低位字节存储到内存的高地址。<br/>
**小端模式（Little-endian）**：字节倒序存<br/>
	将数据的低位字节存储到内存的低地址，高字节存储到内存的低地址<br/>

**总结**:UTF16进制显示,若文件名不带BE,LE的标识,则文件中前面会自带标识,默认是小端显示.<br/>

===>可以用代码来试一下大小端: 0x12345667   1是数据的最高字节   是最低地址

```cpp
#include <stdio.h>

int main()
{
	int a = 0x12345678;
	printf("%2x %x \n",*(char*) &a, a); //78  12345678
	return 0;
}
//从结果上看   0x12345678 的首地址是78   系统默认是小端显示
//同时,打印首地址的方式,取地址,转为char* 打印首地址对应的值观察
```

#### 2. 通过样例代码,实现编码格式的转换:

1. 了解iconv相关命令. 
```cpp
iconv命令用于文件编码的转换;
查看支持的转码格式  ==>iconv --list
转码命令:    iconv -f 原编码 -t 新编码 filename -o newfile
```

2. 了解iconv相关接口api. ===>restrict关键字,编译器优化,定义指针只能这个对象访问到
```cpp
typedef void *iconv_t;
extern iconv_t iconv_open (const char *__tocode, const char *__fromcode);
extern size_t iconv (iconv_t __cd, char **__restrict __inbuf,
		     size_t *__restrict __inbytesleft,
		     char **__restrict __outbuf,
		     size_t *__restrict __outbytesleft);
extern int iconv_close (iconv_t __cd);
```
3. 实现iconv的代码测试.==》iconv_first.c
4. iconv封装成类测试.
5. 实践应用===>连接mysql时,特殊字符识别  utf8->utf8mb4
6. nginx的中文乱码问题
