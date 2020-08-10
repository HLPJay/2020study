#include "DeflateStreamTest.h"
#include <cstring>

/**************************************************
要关注的结构体：
typedef struct z_stream_s {
    z_const Bytef *next_in;     // next input byte 
    uInt     avail_in;  // number of bytes available at next_in 
    uLong    total_in;  // total number of input bytes read so far 

    Bytef    *next_out; // next output byte should be put there 
    uInt     avail_out; // remaining free space at next_out 
    uLong    total_out; // total number of bytes output so far 

    z_const char *msg;  // last error message, NULL if no error 
    struct internal_state FAR *state; // not visible by applications 

    alloc_func zalloc;  //used to allocate the internal state 
    free_func  zfree;   // used to free the internal state 
    voidpf     opaque;  // private data object passed to zalloc and zfree 

    int     data_type;  // best guess about the data type: binary or text 
    uLong   adler;      // adler32 value of the uncompressed data 
    uLong   reserved;   // reserved for future use 
} z_stream;
***************************************************/
//zlib/gzip类型 压缩级别 压缩的文件名
DeflateStream::DeflateStream(const StreamType type, const int level, const std::string orignName)
	:m_flag(false)
{
	m_zlibStream.zalloc = Z_NULL; //用于分配内部状态
	m_zlibStream.zfree = Z_NULL;  //用于释放内部状态
	m_zlibStream.opaque = Z_NULL; //传递给zalloc和zfree的私有数据对象
	m_zlibStream.next_in = 0;     //下一个输入字节
	m_zlibStream.avail_in = 0;    //next_in中可用的字节数
	m_zlibStream.next_out = 0;    //下一个输出字节应放在此处
	m_zlibStream.avail_out = 0;   //next_out的剩余可用空间

	m_buffer = new char[DEFLATE_BUFFER_SIZE];
	m_orignNameBuffer = new char[ORIGN_NAME_BUFFER_SIZE];

	//压缩要用到的流 压缩级别 压缩方法 8～15(gzip编码+16) 指定应分配多少内存 策略参数（影响压缩率）
	//第四个参数 8~15之间 此参数的值越大，效果越好压缩以牺牲内存使用为代价。如果是，则默认值为15
	//gzip编码 可以大于15加16在windowBits周围写一个简单的gzip标头和预告片压缩数据而不是zlib包装器
/********************************************************
	deflateInit2(strm, level, method, windowBits, memLevel, strategy)
	为压缩初始化内部流状态，zalloc，zfree 和 opaque 字段必须在调用之前初始化，
	如果zalloc和zfree 被初始化为 Z_NULL，deflateInit 会更新它们而使用默认的分配函数。

	压缩级别必须为 Z_DEFAULT_COMPRESSION，或者 0 到 9 之间的数；
		1 表示最快速度的压缩，9 表示最优压缩，0 不做任何压缩，
		Z_DEFAULT_COMPRESSION 是速度和最优压缩的折衷（一般为 6）。
	函数成功返回 Z_OK，如果没有足够的内存则返回 Z_MEM_ERROR，
		如果不是一个有效的压缩级别则返回 Z_STREAM_ERROR，版本不兼容则返回 Z_VERSION_ERROR。如果没有错误信息则 msg被设置为0。

	z_stream:这个是压缩上下文，官方给的例子进行初始化:
		strm.zalloc = NULL; 
		strm.zfree = NULL; 
		strm.opaque = NULL;
		strm.next_in = 你的待压缩数据
		strm.next_out = 压缩以后数据存储的 buffer
		strm.avail_in = 待压缩数据的长度
		strm.avail_out = 压缩数据存储 buffer 的长度.
	level: 压缩的等级，目前有四个值 :
		#define Z_NO_COMPRESSION 0 //不压缩
		#define Z_BEST_SPEED 1 //速度优先,可以理解为最低限度的压缩.
		#define Z_BEST_COMPRESSION 9 //压缩优先,但是速度会有些慢
		#define Z_DEFAULT_COMPRESSION (-1) //默认选项，compress 里面用的就是这个选项
	method: 值只有一个，当前唯一的 defalte 压缩方法，用于以后扩展#define Z_DEFLATED 8
	windowBits: 窗口比特数
		-(15 ~ 8) : 纯 deflate 压缩
		+(15 ~ 8) : 带 zlib 头和尾
		> 16 : 带 gzip 头和尾
	memLevel: 目前只有一个选项，MAX_MEM_LEVEL，无非是运行过程中对内存使用的限制
	strategy ：用于调整压缩算法，直接给默认就行 Z_DEFAULT_STRATEGY.
		#define Z_FILTERED 1        //用于由 filter（或者称为 predictor）生成的数据
		#define Z_HUFFMAN_ONLY 2    //用于强制哈夫曼编码（不做字符匹配）
		#define Z_RLE 3             //限制匹配长度为 1
		#define Z_FIXED 4           //阻止使用动态哈夫曼编码，从而允许获得更简单的解码
		#define Z_DEFAULT_STRATEGY 0  //用于普通数据
		Z_FILTERED， 用于由 filter（或者称为 predictor）生成的数据.过滤的数据包含很多小的随机数据。这种情况下，
		压缩算法能够获得更好的压缩效果。该选项可以强制更多的哈夫曼编码和更少的字符匹配 。
			有时候可以作为Z_DEFAULT_STRATEGY 和Z_HUFFMAN_ONLY 的折衷。
		Z_FIXED，阻止使用动态哈夫曼编码，从而允许获得更简单的解码。
		strategy 参数只影响压缩比，而不会影响到压缩输出的正确性，因此没有正确的设置也不要紧。
**********************************************************/

	int ret = deflateInit2(&m_zlibStream, 
				level,
				Z_DEFLATED,
				MAX_WBITS + (type == STREAM_GZIP ? 16 : 0),
				MAX_MEM_LEVEL,
				Z_DEFAULT_STRATEGY); //压缩策略
	if(ret != Z_OK)
	{
		delete [] m_buffer;
		delete [] m_orignNameBuffer;
		m_buffer = NULL;
		m_orignNameBuffer = NULL;
		printf("DeflatingStream failed:%s\n", zError(ret));
        throw "deflateInit2 failed";
        return;
	}
/*******************************

typedef struct gz_header_s {
    int     text;       // true if compressed data believed to be text 
    uLong   time;       // modification time 
    int     xflags;     // extra flags (not used when writing a gzip file) 
    int     os;         // operating system 
    Bytef   *extra;     // pointer to extra field or Z_NULL if none 
    uInt    extra_len;  // extra field length (valid if extra != Z_NULL) 
    uInt    extra_max;  // space at extra (only when reading header) 
    Bytef   *name;      // pointer to zero-terminated file name or Z_NULL 
    uInt    name_max;   // space at name (only when reading header) 
    Bytef   *comment;   // pointer to zero-terminated comment or Z_NULL 
    uInt    comm_max;   //space at comment (only when reading header) 
    int     hcrc;       // true if there was or will be a header crc 
    int     done;       // true when done reading gzip header (not used
                           when writing a gzip file) 
} gz_header;

*******************************/	
	//如果是gzip的头 开始处理gzip相关的流 也就是把gzip的头塞进去
	if(STREAM_GZIP == type)
	{
		memset(&m_gzipHead, sizeof(m_gzipHead), 0);
		strcpy(m_orignNameBuffer, orignName.c_str());
		m_gzipHead.name = (Bytef *)m_orignNameBuffer;
		m_gzipHead.name_max = strlen(orignName.c_str());
		printf("m_gzipHead.name:%s, size:%d\n", m_gzipHead.name, m_gzipHead.name_max);
		ret = deflateSetHeader(&m_zlibStream, &m_gzipHead);
		printf("deflateSetHeader = %d\n", ret);
	}
}

DeflateStream::~DeflateStream()
{
	if(m_buffer)
	{
		delete []m_buffer;
		m_buffer = NULL;
	}
	if(m_orignNameBuffer)
	{
		delete m_orignNameBuffer;
		m_orignNameBuffer = NULL;
	}
	// 释放deflateInit 中申请的空间
	deflateEnd(&m_zlibStream);
}

int DeflateStream::Deflate(char *inBuf, size_t inLen, std::string &outStr)
{
	if(!inBuf || inLen == 0)
	{
		m_flag = true;
	}
//定义输入流和输入长度  输出流和输出空间
	m_zlibStream.next_in = (unsigned char *) inBuf; //输入数据
	m_zlibStream.avail_in = static_cast<unsigned>(inLen); // 输入多少数据
	m_zlibStream.next_out = (unsigned char *)m_buffer; // 输出空间
	m_zlibStream.avail_out = DEFLATE_BUFFER_SIZE;  // 输出空间的大小

	if(!m_flag)//压缩
	{
		for(;;)
		{
			//开始去压缩
/******************************************************
deflate函数尽可能的压缩数据，当输入缓冲为空或者输出缓冲满了的时候会停止。
	它会带来输出延迟（读输入数据而没有输出）除非强行刷新缓冲区。
详细的语意如下，deflate 会执行下面的一个或者两个动作都执行：
	◼ 从 next_in 开始压缩输入数据从而更新 next_in 和 avail_in。
	如果不是所有输入数据可以被处理（因为输出缓冲没有足够的空间），next_in 和 avail_in 会更新，
	当再次调用 deflate()函数时输入数据会从这一点开始被处理。
	◼ 从 next_out 开始提供更多输出数据从而更新 next_out 和 avail_out，
	如果 flush参数不是为 0 的化这个动作是强制性的，经常性的强制刷新缓冲区会降低压缩比率，
	所以只有必要的时候才会设置这个参数（在交际程序时），一些数据也会输出即使刷新没有被设置。
在调用 deflate()函数之前，应用程序必须保证至少上面的一个动作被执行（avail_in或 avail_out 被设置），
	用提供更多输入数据或消耗更多输出数据的方式，从而更新avail_in 或 avail_out；
	avail_out 在函数调用之前千万不能为 0。
应用程序可以随时消耗被压缩的输出数据，
	举个例子：当输出缓冲满了或者在每次调用 deflate()之后，如果 deflate返回 Z_OK 并 avail_out 为 0 时，
	deflate()必须再次被调用（说明输出缓存区还有数据应该被读取）。
Int flush 的参数：
	Z_NO_FLUSH：通常设置为该值，允许压缩算法决定累积多少数据再产生输出，以达到压缩效率最高。
	Z_SYNC_FLUSH：将所有等待输出的数据刷新到输出缓冲区，以字节为边界进行对齐。
			该刷新可能会降低压缩算法的压缩效率，它只用于必要的时候。
	Z_FINISH：如果输入和待输出的数据都被处理完，则返回 Z_STREAM_END。
		如果返回 Z_OK or Z_BUF_ERROR 则需要再次调用 Z_FINISH 直到返回 Z_STREAM_END。
******************************************************/
			int ret = deflate(&m_zlibStream, Z_NO_FLUSH);
			printf("%s(%d) ret:%d, avail_in:%u, avail_out:%u\n",
                   __FUNCTION__, __LINE__, ret, m_zlibStream.avail_in, m_zlibStream.avail_out);
			if(ret != Z_OK)
			{
				perror("deflate error.");
                return -1;
			}
			//next_in 和 avail_in 是内部更新？？？
			//outStr，m_buffer存储着实际的数据   next_in和next_out只是中间处理  调整m_buffer用
			if (m_zlibStream.avail_out == 0)// 没有输出空间了 扩充输出空间
            {
                outStr.append(m_buffer, m_buffer + DEFLATE_BUFFER_SIZE);// 扩充输出空间 这里是扩充
                m_zlibStream.next_out = (unsigned char *)m_buffer;
                m_zlibStream.avail_out = DEFLATE_BUFFER_SIZE;
            } 
            else if(m_zlibStream.avail_in == 0) // 输入已经处理完成 还有剩余输出空间 减去输出空间
            {
                outStr.append(m_buffer, DEFLATE_BUFFER_SIZE - m_zlibStream.avail_out);//这里是缩减输出空间
                m_zlibStream.next_out = (unsigned char *)m_buffer;
                m_zlibStream.avail_out = DEFLATE_BUFFER_SIZE;
                break;
            } 
		}
	}
	else //不会再有数据进来  看看next_out是否还有数据
	{
		if(m_zlibStream.next_out)
		{
/******************************************************
    Z_NO_FLUSH：通常设置为该值，允许压缩算法决定累积多少数据再产生输出，以达到压缩效率最高。
	Z_SYNC_FLUSH：将所有等待输出的数据刷新到输出缓冲区，以字节为边界进行对齐。
			该刷新可能会降低压缩算法的压缩效率，它只用于必要的时候。
	Z_FINISH：如果输入和待输出的数据都被处理完，则返回 Z_STREAM_END。
		如果返回 Z_OK or Z_BUF_ERROR 则需要再次调用 Z_FINISH 直到返回 Z_STREAM_END。
******************************************************/
			int ret = deflate(&m_zlibStream, Z_FINISH);//加载缓冲区
            printf("%s(%d) ret:%d, avail_in:%u, avail_out:%u\n",
                   __FUNCTION__, __LINE__, ret, m_zlibStream.avail_in, m_zlibStream.avail_out);
            if (ret != Z_OK && ret != Z_STREAM_END)
            {
                perror("deflate ");
                return -1;
            }
            outStr.append(m_buffer, DEFLATE_BUFFER_SIZE - m_zlibStream.avail_out);
            m_zlibStream.next_out = (unsigned char *)m_buffer;
            m_zlibStream.avail_out = DEFLATE_BUFFER_SIZE;
            while (ret != Z_STREAM_END) //设置finish 等待返回值为Z_STREAM_END ==》数据拷贝完了
            {
                ret = deflate(&m_zlibStream, Z_FINISH);
                printf("%s(%d) ret:%d, avail_in:%u, avail_out:%u\n",
                       __FUNCTION__, __LINE__, ret, m_zlibStream.avail_in, m_zlibStream.avail_out);
                if (ret != Z_OK && ret != Z_STREAM_END)
                {
                    perror("deflate ");
                    return -1;
                }
                outStr.append(m_buffer, DEFLATE_BUFFER_SIZE - m_zlibStream.avail_out);
                m_zlibStream.next_out = (unsigned char *)m_buffer;
                m_zlibStream.avail_out = DEFLATE_BUFFER_SIZE;
            }
		}
	}
	return 0;
}