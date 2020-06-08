
#include <stdio.h>
#include <openssl/bio.h>

#define OUT_LENGTH 128
int main()
{
	//类似于listen的accept 创建socket ==>这个接口被放弃了？？？
	int sock = BIO_get_accept_socket("8899", 0);
	//监听到socket
	BIO *b = BIO_new_socket(sock, BIO_NOCLOSE);

	char *addr = NULL;
	int ret = BIO_accept(sock, &addr);//获取客户端的ip和端口

	BIO_set_fd(b, ret, BIO_NOCLOSE);
	while(1)
	{
		char out[OUT_LENGTH] = {0};

		BIO_read(b, out, OUT_LENGTH);
		if(out[0] = 'q')
		{
			break;
		}
		printf("out = %s \n", out);

	}
	BIO_free(b);
	return 0;
}
