//可以操作内存，日志，socket
/**************************
BIO不必关注实现，多关注子系统的实现 ==》很优秀
	内存，文件，日志，标准输入输出，加密解密

总结关于BIO框架的代码
***************************/
#include <stdio.h>
#include <openssl/bio.h>
/****************************
两个线程公用内存可以用这个实现，内存的大小也是可变的==》解决同步问题
其实内部是内存池
**************************/
int main()
{
	//BIO_s_mem返回的是结构体一些列实例化的内存操作的方法
	BIO *b = BIO_new(BIO_s_mem());//参数传实例化的结构体

	//两种写数据的方法
	int len = BIO_write(b, "openssl write", 13);
	BIO_printf(b, "%s", "write test");

	//读取占用内存的长度
	len = BIO_ctrl_pending(b);//占用空间大小
	//BIO的长度读出来
	char out[126] ={0};
	len = BIO_read(b, out, len);
	printf("out = %s\n", out);
	BIO_free(b);
	return 0;
}
