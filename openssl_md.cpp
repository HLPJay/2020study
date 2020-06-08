
//摘要加密算法 
//例如  ：md5是如何生成的，为什么会独一无二？（网上说）
int main()
{

	unsigned char in[] = "EVP_ENCODE_CTX dctx;\
	EVP_DecodeInit(&dctx);\
	unsigned char decode[100];\
	int deLen; \
	EVP_DecodeUpdate(&dctx，decode,  &deLen, bae64, total);\
	EVP_DecodeFinal(&dctx，decode, &deLen)";
//通过摘要和内容做对比判断是否被篡改
//和CRC一样 用来校验
unsigned char out[128] = {0};

	//摘要处理
	int n = strlen(in);
	MD4(in, n ,out);
	printf("MD4 result :\n");
	//MD4返回的是16个字节
	for(int i=0;i<16;i++)
	{
		printf("%x",out[i]);
	}
	printf("\n");

	MD5(in, n ,out);
	printf("MD4 result :\n");
	//MD4返回的是16个字节
	for(int i=0;i<16;i++)
	{
		printf("%x",out[i]);
	}
  printf("\n");

	SHA(in, n ,out);
	printf("MD4 result :\n");
	//MD4返回的是16个字节
	for(int i=0;i<20;i++)
	{
		printf("%x",out[i]);
	}
	printf("\n");

	SHA1(in, n ,out);
	printf("MD4 result :\n");
	//MD4返回的是16个字节
	for(int i=0;i<20;i++)
	{
		printf("%x",out[i]);
	}
	printf("\n");
//http一般用SHA256 或者SHA512
	SHA256(in, n ,out);
	printf("MD4 result :\n");
	//MD4返回的是16个字节
	for(int i=0;i<32;i++)
	{
		printf("%x",out[i]);
	}
	printf("\n");

	SHA512(in, n ,out);
	printf("MD4 result :\n");
	//MD4返回的是16个字节
	for(int i=0;i<64;i++)
	{
		printf("%x",out[i]);
	}
	printf("\n");
	return 0;
}
