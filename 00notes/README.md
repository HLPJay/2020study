# vip课程学习架构
## 数据结构
### 1.1）===》算法，KMP，队列，栈，堆，分治，递归。
1. 十种排序算法，KMP排序。   --》已完成
2. 队列，堆，栈。           --》已完成
3. 分治，递归。
4. 作业：基于堆实现时间片轮询事件。   --》已完成

### 1.2）===》二叉树，红黑树。
1. 二叉树和红黑树的源码实现
2. 应用场景

### 1.3）===》b树，b+树。
1. b树和b+树的源码实现
2. 应用场景
3. b树和b+树的差异

### 1.4）===》布隆过滤器, hash，bitmap。
1. 布隆过滤器的实现源码
2. hash及哈希算法
3. bitmap
4. 布隆过滤器及bitmap的差异

### 1.5) ===》发布订阅者模式 观察者模式。
1. 观察者模式的实现
2. 发布订阅者模式的实现
3. 在开源库中的使用

### 1.6）===》责任链，过滤器，工厂模式，单例模式。
1. 责任链过滤器模式的代码实现
2. 工厂模式
3. 单例模式（多种实现方式）

-----
## 数据库+组件
### 2.1）===》Mysql，redis，mongodb。
1. 相关数据库的基本操作
2. 代码调用api接口
3. 相关数据库的差异
4. 应用场景分析
5. 作业

### 2.2）===》nginx反向代理及负载均衡。
1. nginx搭建及基本操作
2. nginx反向代理和负载均衡
3. fastcgi（作业）

### 2.3）===》异步http的实现（RESTful http）。
1. 异步http请求的实现。
2. http的介绍及应用场景

### 2.4）===》ZeroMq
待定~~

### 其他：select，poll，epoll

-----
## 基础组件
### 3.1）===》线程池的原理及实现
1. 线程池的源码及实现   ==》已完成
2. 循环队列，循环链表的使用
3. 线程池的扩容和缩放--》如何实现？

### 3.2）===》mutex，spinlock，atomic，无锁队列，无锁CAS
1. 需要使用锁的场景
2. 无锁，mutex，spinlock，atomic的对比    ==》已完成
3. mutex，spinlock，atomic的性能对比及使用场景   ==》已完成
4. CAS无锁队列的实现    ==》已完成

### 3.3）===》内存池，ringbuffer，nginx。
1. 内存池的源码及实现   ==》已完成
2. 内存池的应用场景
3. ringbuffer与nginx中内存池的实现分析。
4. tcmalloc  jemalloc

-----
## 应用层协议
### 4.1）===》字符的编解码,压缩,解压缩(unicode,L77+哈夫曼, zlib)。
1. 常用的字符编码,为啥要用字符编码,大小端问题,首字符标识.
2. 压缩与解压缩概述.
3. L77与哈夫曼的实现+代码演示.
4. zlib的实现 +代码演示.
5. nginx中gzip的配置及使用  ==>相关接口使用可以参考本节的课程课件

### 4.2）===》json，xml，protobuff的学习与使用
1. cjson, jsoncpp，repidjson（腾讯开源C++ json解析器）的相关语法及接口梳理.
2. 编码：json开发的一般流程：基类+子类的开发方式.
3. xml（可扩展标记语言）一般做配置文件用：qt，Android，tomcat， pom
4. xml相关转义梳理，以及shema文档（规定一些标识）
5. 源码实现：tinyXML2来解析xml  tinyXML2只有两个文件<br/>
    &nbsp;通过dom（Document object model）整个xml加载到内存中<br/>
	&nbsp;&nbsp;加载进来是个树形结构 <br/>
	&nbsp;通过SAX(事件驱动)加载 ==》逐行扫描，一边扫描一边解析<br/>
	&nbsp;&nbsp;内容越少 用dom ==》二叉树保存<br/>
	&nbsp;&nbsp;内容越大 用sax<br/>
6. 扩展excel和word的解析

### 4.3）===》项目应用层协议设计，protobuf，flatbuffer
1. 设计一个通信协议需要考虑的核心问题有哪些？
2. 序列化及反序列化，以及常用的序列化方法有哪些？
3. 主流序列化协议源码分析。
4. 如何判断包的完整性，以及分包，粘包问题？
5. 常用的协议安全方案。
6. 数据压缩，http协议，redis协议。
7. 自己设计一个合理的协议通信

-----