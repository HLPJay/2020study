#ifndef __DEFLATE_STREAM_TEST_H_
#define __DEFLATE_STREAM_TEST_H_
/************************************************
简单测试：
	1：初始化 压缩类型 压缩级别0～9 压缩后的文件名
	2：执行压缩动作 
		要压缩的数据和长度
		要输出的数据 为0说明还在缓冲区

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

*************************************************/
#include <zlib.h>
#include <iostream>
enum StreamType
{
	STREAM_ZLIB = 0, /// Create a zlib header, use Adler-32 checksum.
	STREAM_GZIP		 /// Create a gzip header, use CRC-32 checksum.
};

class DeflateStream
{
public:
	DeflateStream(const StreamType type, const int level, const std::string orignName);
	~DeflateStream();
	//要压缩的数据  数据长度   输出的字符串
	int Deflate(char *inBuf, size_t inLen, std::string &outStr);

private:
	enum 
	{
		STREAM_BUFFER_SIZE = 1024,
		DEFLATE_BUFFER_SIZE = 32768,
		ORIGN_NAME_BUFFER_SIZE = 128
	};

	z_stream m_zlibStream;
	gz_header m_gzipHead;

	char* m_buffer;
	char* m_orignNameBuffer; //起源名称缓冲区
	bool m_flag;
};


#endif //__DEFLATE_STREAM_TEST_H_