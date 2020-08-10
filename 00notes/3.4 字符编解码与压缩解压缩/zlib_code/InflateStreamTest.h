#ifndef __INFLATE_STREAM_TEST_H_
#define __INFLATE_STREAM_TEST_H_
#include <iostream>
#include <zlib.h>

class InflateStream
{
public:
	enum StreamType
	{
		STREAM_ZLIB, /// Expect a zlib header, use Adler-32 checksum.
		STREAM_GZIP /// Expect a gzip header, use CRC-32 checksum.
	};
	InflateStream(StreamType type);
	~InflateStream();
	int Inflate(const char *inBuf, const size_t inLen, std::string &outBuf);
private:
	enum 
	{
		STREAM_BUFFER_SIZE  = 1024,
		INFLATE_BUFFER_SIZE = 32768,
        ORIGN_NAME_BUFFER_SIZE = 128
	};
	z_stream m_zlibStream;
	gz_header m_gzipHead;

	char* m_buffer;
	char* m_orignNameBuffer; //起源名称缓冲区
	bool m_flag;
};

#endif//__INFLATE_STREAM_TEST_H_