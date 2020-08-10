# 使用zlib库实现压缩和解压
1. 安装，并且观察相关接口。
wget http://www.zlib.net/zlib-1.2.11.tar.gz<br/>
tar -zxvf zlib-1.2.11.tar.gz<br/>
2. 调用接口实现压缩 Deflate<br/>
3. 调用接口实现解压 Inflate<br/>
4. zlib库中 流的逻辑 ===》还需理解<br/>
		压缩和解压的主流程需要梳理<br/>
5. 相关文件操作的接口<br/>
6. 记住编译的时候  一定要加 -lz<br/>
7. 有都是默认的压缩方案，有三种显示结果的方案==》按照默认==>参考官网或者课程课件<br/>
&nbsp; windowBits: 窗口比特数<br/>
&nbsp; -(15 ~ 8) : 纯 deflate 压缩<br/>
&nbsp; +(15 ~ 8) : 带 zlib 头和尾<br/>
&nbsp; > 16 : 带 gzip 头和尾<br/>
    
##### 压缩编译：
g++ -o deflate DeflateStreamTest.h DeflateStreamTest.cpp zlib_test.cpp -lz
##### 解压编译：
g++ -o inflate InflateStreamTest.h InflateStreamTest.cpp Inflate_zlib_test.cpp -lz
