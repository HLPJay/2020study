
#include <stdio.h>
#include <openssl/evp.h>
int main()
{
	//引入上下文环境 整个过程存在
	EVP_ENCODE_CTX encode_ctx;
	//800bit /6===>133
	unsigned char in[100], bae64[150];
	//初始化上下文环境及内容
	EVP_EncodeInit(&encode_ctx);
	for(int i=0;i<100;i++)
	{
		in[i] =i;
	}
	int outl;
	int intl = 100;
	EVP_EncodeUpdate(&encode_ctx, bae64,&outl, in, inl);

	int total = outl;
	//编码 对结果进行输出
	EVP_EncodeFinal(&encode_ctx, bae64+total, &outl);
	printf("%s %d\n", bae64, outl)
/*******************************
base64和摘要 仅仅是为了不传明文
单独传英文 不需要base64 中文可以用
对称加密：
非对称加密：
*******************************/
	//解码
	EVP_ENCODE_CTX dctx;
	EVP_DecodeInit(&dctx);

	unsigned char decode[100];
	int deLen;
	EVP_DecodeUpdate(&dctx，decode,  &deLen, bae64, total);
	EVP_DecodeFinal(&dctx，decode, &deLen);
	for (int i=0;i<100;i++)
	{
		printf("%d", decode[i]);
	}
	printf("\n");
	return 0;
}
