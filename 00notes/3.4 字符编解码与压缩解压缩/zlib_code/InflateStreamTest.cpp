#include <cstring>
#include "InflateStreamTest.h"

//参数 解压的类型
InflateStream::InflateStream(StreamType type)
{
	m_zlibStream.zalloc = Z_NULL; //用于分配内部状态
	m_zlibStream.zfree = Z_NULL;  //用于释放内部状态
	m_zlibStream.opaque = Z_NULL; //传递给zalloc和zfree的私有数据对象
	m_zlibStream.next_in = 0;     //下一个输入字节
	m_zlibStream.avail_in = 0;    //next_in中可用的字节数
	m_zlibStream.next_out = 0;    //下一个输出字节应放在此处
	m_zlibStream.avail_out = 0;   //next_out的剩余可用空间
	
	m_buffer = new char[INFLATE_BUFFER_SIZE];
	m_orignNameBuffer = new char[ORIGN_NAME_BUFFER_SIZE];
	//8~15之间 此参数的值越大，效果越好压缩以牺牲内存使用为代价。如果是，则默认值为15
	//gzip编码 可以大于15加16在windowBits周围写一个简单的gzip标头和预告片压缩数据而不是zlib包装器
	int ret = inflateInit2(&m_zlibStream, 15 + (type == STREAM_GZIP ? 16 : 0));
	if (ret != Z_OK)//初始化失败
    {
        delete [] m_buffer;
        delete [] m_orignNameBuffer;
        m_buffer = NULL;
        m_orignNameBuffer = NULL;
        printf("InflatingStream failed:%s\n", zError(ret));
        throw "inflateInit2 failed";
    }
    if (STREAM_GZIP == type)
    {
        memset(&m_gzipHead, sizeof(gz_header), 0);
        m_gzipHead.extra = (Bytef *)m_orignNameBuffer;
        m_gzipHead.extra_max = ORIGN_NAME_BUFFER_SIZE;
        m_gzipHead.name = (Bytef *)m_orignNameBuffer;
        m_gzipHead.name_max = ORIGN_NAME_BUFFER_SIZE;
        m_gzipHead.comment = (Bytef *)m_orignNameBuffer;
        m_gzipHead.comm_max = ORIGN_NAME_BUFFER_SIZE;
        int rc = inflateGetHeader(&m_zlibStream, &m_gzipHead);
        printf("inflateGetHeader = %d\n", rc);
    }

}

/**********************************************
typedef struct gz_header_s {
    int     text;       // 如果压缩数据被认为是文本，则为true
    uLong   time;       // 修改时间
    int     xflags;     // 额外的标志（编写gzip文件时不使用）
    int     os;         // 操作系统
    Bytef   *extra;     // 指向额外字段的指针，如果没有则指向Z_NULL
    uInt    extra_len;  // 额外的字段长度（如果多余！= Z_NULL，则有效）
    uInt    extra_max;  // 多余的空间（仅在读取标题时）
    Bytef   *name;      // 指向以零结尾的文件名或Z_NULL的指针
    uInt    name_max;   // 名称空间（仅在读取标题时）
    Bytef   *comment;   // 指向以零结尾的注释或Z_NULL的指针
    uInt    comm_max;   // 注释处的空格（仅在读取标题时）
    int     hcrc;       // 如果有或将有头crc，则为true
    int     done;       // 读取gzip标头后为true（未使用编写gzip文件时）
} gz_header;
***********************************************/

InflateStream::~InflateStream()
{
	if(m_buffer) delete []m_buffer;
	if(m_orignNameBuffer) delete [] m_orignNameBuffer;
	inflateEnd(&m_zlibStream);
}

int InflateStream::Inflate(const char *inBuf, const size_t inLen, std::string &outBuf)
{
	//要进行解压的字符和字符的长度
	if(!inBuf || inLen ==0)
	{
		m_flag = true;
	}
	/******************
	z_stream m_zlibStream;
	gz_header m_gzipHead;

	char* m_buffer;//存数据用
	char* m_orignNameBuffer; //起源名称缓冲区
	bool m_flag;
	******************/
	m_zlibStream.next_in = (unsigned char *)inBuf;
    m_zlibStream.avail_in = static_cast<unsigned>(inLen);
    m_zlibStream.next_out = (unsigned char *)m_buffer;
    m_zlibStream.avail_out = INFLATE_BUFFER_SIZE;
    if(!m_flag)
    {
    	for(;;)
    	{
    		int rc = inflate(&m_zlibStream, Z_NO_FLUSH);//开始解压
            if(m_gzipHead.done) //gzip
            {
                printf("inflate name:%s, extra:%s\n", m_gzipHead.name, m_gzipHead.extra);
            }
            if (rc == Z_STREAM_END)
            {
                outBuf.append(m_buffer, m_buffer + (INFLATE_BUFFER_SIZE - m_zlibStream.avail_out));
                break;
            }
            if (rc != Z_OK)
            {
                perror("inflate error: ");
                return -1;
            }
            if (m_zlibStream.avail_out == 0) //输出空间完了，统计输出大小
            {
                outBuf.append(m_buffer, m_buffer + INFLATE_BUFFER_SIZE);
                m_zlibStream.next_out = (unsigned char *)m_buffer;
                m_zlibStream.avail_out = INFLATE_BUFFER_SIZE;
            }
            else if (m_zlibStream.avail_in == 0)//TODO  这里应该是解压完了
            {
                outBuf.append(m_buffer, m_buffer + (INFLATE_BUFFER_SIZE - m_zlibStream.avail_out));
                m_zlibStream.next_out = (unsigned char *)m_buffer;
                m_zlibStream.avail_out = INFLATE_BUFFER_SIZE;
                break;
            }
    	}
    }
    else
    {
    	if (m_zlibStream.next_out)
        {
            int rc = inflate(&m_zlibStream, Z_FINISH);
            printf("%s(%d) ret:%d, avail_in:%u, avail_out:%u\n",
                   __FUNCTION__, __LINE__, rc, m_zlibStream.avail_in, m_zlibStream.avail_out);
            if (rc != Z_OK && rc != Z_STREAM_END)
            {
                perror("deflate ");
                return -1;
            }
            outBuf.append(m_buffer, INFLATE_BUFFER_SIZE - m_zlibStream.avail_out);
            m_zlibStream.next_out = (unsigned char *)m_buffer;
            m_zlibStream.avail_out = INFLATE_BUFFER_SIZE;
            while (rc != Z_STREAM_END)
            {
                rc = deflate(&m_zlibStream, Z_FINISH);
                printf("%s(%d) ret:%d, avail_in:%u, avail_out:%u\n",
                       __FUNCTION__, __LINE__, rc, m_zlibStream.avail_in, m_zlibStream.avail_out);
                if (rc != Z_OK && rc != Z_STREAM_END)
                {
                    perror("deflate ");
                    return -1;
                }
                outBuf.append(m_buffer, INFLATE_BUFFER_SIZE - m_zlibStream.avail_out);
                m_zlibStream.next_out = (unsigned char *)m_buffer;
                m_zlibStream.avail_out = INFLATE_BUFFER_SIZE;
            }
        }
    }
    return 0;
}