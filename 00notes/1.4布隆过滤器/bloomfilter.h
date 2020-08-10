#ifndef __BLOOMFILTER_H_
#define __BLOOMFILTER_H_
/******************************************8

__attribute__ 其实是个编译器指令，告诉编译器声明的特性，或者让编译器进行更多的错误检查和高级优化
__attribute__((always_inline)) 强制内联
**********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>


#define __BLOOMFILTER_VERSION__ "1.1"
#define __MGAIC_CODE__          (0x01464C42)

#define BYTE_BITS           (8)
#define MIX_UINT64(v)       ((uint32_t)((v>>32)^(v)))
#define FORCE_INLINE         __attribute__((always_inline))

#define SETBIT(filter, n)   (filter->pstFilter[n/BYTE_BITS] |= (1 << (n%BYTE_BITS)))
#define GETBIT(filter, n)   (filter->pstFilter[n/BYTE_BITS] & (1 << (n%BYTE_BITS)))


#pragma pack(1)
//bloomfulter 结构定义
typedef struct {
	uint8_t cInitFlag;                              // 初始化标志，为0时的第一次Add()会对stFilter[]做初始化
    uint8_t cResv[3];

    uint32_t dwMaxItems;                            // n - BloomFilter中最大元素个数 (输入量)
    double dProbFalse;                              // p - 假阳概率 (输入量，比如万分之一：0.00001)
    uint32_t dwFilterBits;                          // m = ceil((n * log(p)) / log(1.0 / (pow(2.0, log(2.0))))); - BloomFilter的比特数
    uint32_t dwHashFuncs;                           // k = round(log(2.0) * m / n); - 哈希函数个数
    
    uint32_t dwSeed;                                // MurmurHash的种子偏移量
    uint32_t dwCount;                               // Add()的计数，超过MAX_BLOOMFILTER_N则返回失败

    uint32_t dwFilterSize;                          // dwFilterBits / BYTE_BITS
    unsigned char *pstFilter;                       // BloomFilter存储指针，使用malloc分配
    uint32_t *pdwHashPos;
}BaseBloomFilter;
//bloomfilter 文件头部定义
typedef struct
{
    uint32_t dwMagicCode;                           // 文件头部标识，填充 __MGAIC_CODE__
    uint32_t dwSeed;
    uint32_t dwCount;

    uint32_t dwMaxItems;                            // n - BloomFilter中最大元素个数 (输入量)
    double dProbFalse;                              // p - 假阳概率 (输入量，比如万分之一：0.00001)
    uint32_t dwFilterBits;                          // m = ceil((n * log(p)) / log(1.0 / (pow(2.0, log(2.0))))); - BloomFilter的比特数
    uint32_t dwHashFuncs;                           // k = round(log(2.0) * m / n); - 哈希函数个数

    uint32_t dwResv[6];
    uint32_t dwFileCrc;                             // (未使用)整个文件的校验和
    uint32_t dwFilterSize;                          // 后面Filter的Buffer长度
} BloomFileHead;
#pragma pack()

//种子偏移量  目标经度 和数据个数
 inline int InitBloomFilter(BaseBloomFilter *pstBloomfilter, uint32_t dwSeed, uint32_t dwMaxItems, double dProbFalse);
 inline int FreeBloomFilter(BaseBloomFilter *pstBloomfilter);
 //第一次add的时候去初始化
 inline int ResetBloomFilter(BaseBloomFilter *pstBloomfilter);
 inline int RealResetBloomFilter(BaseBloomFilter *pstBloomfilter);

 FORCE_INLINE int BloomFilter_Add(BaseBloomFilter *pstBloomfilter, const void * key, int len);
 FORCE_INLINE int BloomFilter_Check(BaseBloomFilter *pstBloomfilter, const void * key, int len);

 inline int SaveBloomFilterToFile(BaseBloomFilter *pstBloomfilter, char *szFileName);
 inline int LoadBloomFilterFromFile(BaseBloomFilter *pstBloomfilter, char *szFileName);


//static inline void  _CalcBloomFilterParam(uint32_t n, double p, uint32_t *pm, uint32_t *pk);
inline void  _CalcBloomFilterParam(uint32_t n, double p, uint32_t *pm, uint32_t *pk)
{
	uint32_t m,k;
	//比特数
	m =  (uint32_t) ceil(-1 * log(p) * n / 0.6185);
	m = (m - m % 64) + 64; //八字节对齐

	//哈希函数个数
	k = (uint32_t) (0.6931 * m / n);
    k++;

    *pm = m;
    *pk = k;
    return ;
}

inline int InitBloomFilter(BaseBloomFilter *pstBloomfilter, uint32_t dwSeed, uint32_t dwMaxItems, double dProbFalse)
{
	if(pstBloomfilter == NULL)
	{
		return -1;
	}
	if(dProbFalse <0 || dProbFalse >1)
	{
		return -2;
	}

	if(pstBloomfilter->pstFilter != NULL)
	{
		free(pstBloomfilter->pstFilter);
	}
	if(pstBloomfilter->pdwHashPos != NULL)
	{
		free(pstBloomfilter->pdwHashPos);
	}

	memset(pstBloomfilter, 0, sizeof(BaseBloomFilter));
	pstBloomfilter->dwMaxItems = dwMaxItems;//最大值
	pstBloomfilter->dProbFalse = dProbFalse;//失误率
    pstBloomfilter->dwSeed = dwSeed;	//种子偏移量

    //计算哈希个数和布隆过滤器二进制位数
    _CalcBloomFilterParam(pstBloomfilter->dwMaxItems, pstBloomfilter->dProbFalse,
    			&pstBloomfilter->dwFilterBits, &pstBloomfilter->dwHashFuncs);

    pstBloomfilter->dwFilterSize = pstBloomfilter->dwFilterBits / BYTE_BITS;
    pstBloomfilter->pstFilter = (unsigned char *) malloc(pstBloomfilter->dwFilterSize);

    if(pstBloomfilter->pstFilter == NULL)
    {
    	return -100;
    }

    pstBloomfilter->pdwHashPos = (uint32_t*) malloc(pstBloomfilter->dwHashFuncs * sizeof(uint32_t));
    if(pstBloomfilter->pdwHashPos == NULL)
    {
    	return -200;
    }

    printf(">>> Init BloomFilter(n=%u, p=%f, m=%u, k=%d), malloc() size=%.2fMB\n",
            pstBloomfilter->dwMaxItems, pstBloomfilter->dProbFalse, pstBloomfilter->dwFilterBits,
            pstBloomfilter->dwHashFuncs, (double)pstBloomfilter->dwFilterSize/1024/1024);

    memset(pstBloomfilter->pstFilter, 0, pstBloomfilter->dwFilterSize);
    pstBloomfilter->cInitFlag = 1;
    return 0;
}

inline int FreeBloomFilter(BaseBloomFilter *pstBloomfilter)
{
	if(pstBloomfilter == NULL)
	{
		return -1;
	}
	pstBloomfilter->cInitFlag = 0;
	pstBloomfilter->dwCount = 0;

	free(pstBloomfilter->pstFilter);
	pstBloomfilter->pstFilter = NULL;
	free(pstBloomfilter->pdwHashPos);
	pstBloomfilter->pdwHashPos = NULL;
	return 0;
}

inline int ResetBloomFilter(BaseBloomFilter *pstBloomfilter)
{
	if(pstBloomfilter == NULL)
	{
		return -1;
	}
	pstBloomfilter->cInitFlag = 0;
    pstBloomfilter->dwCount = 0;
    return 0;
}

inline int RealResetBloomFilter(BaseBloomFilter *pstBloomfilter)
{
    if (pstBloomfilter == NULL)
        return -1;
    
    memset(pstBloomfilter->pstFilter, 0, pstBloomfilter->dwFilterSize);
    pstBloomfilter->cInitFlag = 1;
    pstBloomfilter->dwCount = 0;
    return 0;
}

FORCE_INLINE void bloom_hash(BaseBloomFilter *pstBloomfilter, const void * key, int len);
FORCE_INLINE int BloomFilter_Add(BaseBloomFilter *pstBloomfilter, const void * key, int len)
{
	if(pstBloomfilter == NULL || key== NULL || len<=0)
	{
		return -1;
	}

	int i;
	if(pstBloomfilter->cInitFlag!=1)
	{
		memset(pstBloomfilter->pstFilter, 0, pstBloomfilter->dwFilterSize);
		pstBloomfilter->cInitFlag = 1;
	}

	bloom_hash(pstBloomfilter, key, len);
	//给比特位塞值
	for (i = 0; i < (int)pstBloomfilter->dwHashFuncs; i++)
    {
        SETBIT(pstBloomfilter, pstBloomfilter->pdwHashPos[i]);
    }
    
    // 增加count数
    pstBloomfilter->dwCount++;
    if (pstBloomfilter->dwCount <= pstBloomfilter->dwMaxItems)
        return 0;
    else
        return 1;  
}

//检查元素是否存在 获取比特位，有任意位为0，则说明不存在
FORCE_INLINE int BloomFilter_Check(BaseBloomFilter *pstBloomfilter, const void * key, int len)
{
	if ((pstBloomfilter == NULL) || (key == NULL) || (len <= 0))
	{
		return -1;
	}
    int i;
    
    bloom_hash(pstBloomfilter, key, len);
    for (i = 0; i < (int)pstBloomfilter->dwHashFuncs; i++)
    {
        // 如果有任意bit不为1，说明key不在bloomfilter中
        // 注意: GETBIT()返回不是0|1，高位可能出现128之类的情况
        if (GETBIT(pstBloomfilter, pstBloomfilter->pdwHashPos[i]) == 0)
            return 1;
    }
    
    return 0;
}

//选择必要的信息存入文件中
inline int SaveBloomFilterToFile(BaseBloomFilter *pstBloomfilter, char *szFileName)
{
	if ((pstBloomfilter == NULL) || (szFileName == NULL))
	{
		return -1;
	}

	int iRet;
    FILE *pFile;
    BloomFileHead stFileHeader = {0};

    pFile = fopen(szFileName, "wb");
    if (pFile == NULL)
    {
        perror("fopen");
        return -11;
    }

    // 先写入文件头
    stFileHeader.dwMagicCode = __MGAIC_CODE__;
    stFileHeader.dwSeed = pstBloomfilter->dwSeed;
    stFileHeader.dwCount = pstBloomfilter->dwCount;
    stFileHeader.dwMaxItems = pstBloomfilter->dwMaxItems;
    stFileHeader.dProbFalse = pstBloomfilter->dProbFalse;
    stFileHeader.dwFilterBits = pstBloomfilter->dwFilterBits;
    stFileHeader.dwHashFuncs = pstBloomfilter->dwHashFuncs;
    stFileHeader.dwFilterSize = pstBloomfilter->dwFilterSize;

    iRet = fwrite((const void*)&stFileHeader, sizeof(stFileHeader), 1, pFile);
    if (iRet != 1)
    {
        perror("fwrite(head)");
        return -21;
    }

    // 接着写入BloomFilter的内容
    iRet = fwrite(pstBloomfilter->pstFilter, 1, pstBloomfilter->dwFilterSize, pFile);
    if ((uint32_t)iRet != pstBloomfilter->dwFilterSize)
    {
        perror("fwrite(data)");
        return -31;
    }

    fclose(pFile);
    return 0;
}

inline int LoadBloomFilterFromFile(BaseBloomFilter *pstBloomfilter, char *szFileName)
{
	if ((pstBloomfilter == NULL) || (szFileName == NULL))
	{
		return -1;
	}
	int iRet;
    FILE *pFile;
    BloomFileHead stFileHeader = {0};

    if (pstBloomfilter->pstFilter != NULL)
        free(pstBloomfilter->pstFilter);
    if (pstBloomfilter->pdwHashPos != NULL)
        free(pstBloomfilter->pdwHashPos);

    //
    pFile = fopen(szFileName, "rb");
    if (pFile == NULL)
    {
        perror("fopen");
        return -11;
    }

    // 读取并检查文件头
    iRet = fread((void*)&stFileHeader, sizeof(stFileHeader), 1, pFile);
    if (iRet != 1)
    {
        perror("fread(head)");
        return -21;
    }

    if ((stFileHeader.dwMagicCode != __MGAIC_CODE__)
        || (stFileHeader.dwFilterBits != stFileHeader.dwFilterSize*BYTE_BITS))
        return -50;

    // 初始化传入的 BaseBloomFilter 结构
    pstBloomfilter->dwMaxItems = stFileHeader.dwMaxItems;
    pstBloomfilter->dProbFalse = stFileHeader.dProbFalse;
    pstBloomfilter->dwFilterBits = stFileHeader.dwFilterBits;
    pstBloomfilter->dwHashFuncs = stFileHeader.dwHashFuncs;
    pstBloomfilter->dwSeed = stFileHeader.dwSeed;
    pstBloomfilter->dwCount = stFileHeader.dwCount;
    pstBloomfilter->dwFilterSize = stFileHeader.dwFilterSize;

    pstBloomfilter->pstFilter = (unsigned char *) malloc(pstBloomfilter->dwFilterSize);
    if (NULL == pstBloomfilter->pstFilter)
        return -100;
    pstBloomfilter->pdwHashPos = (uint32_t*) malloc(pstBloomfilter->dwHashFuncs * sizeof(uint32_t));
    if (NULL == pstBloomfilter->pdwHashPos)
        return -200;


    // 将后面的Data部分读入 pstFilter
    iRet = fread((void*)(pstBloomfilter->pstFilter), 1, pstBloomfilter->dwFilterSize, pFile);
    if ((uint32_t)iRet != pstBloomfilter->dwFilterSize)
    {
        perror("fread(data)");
        return -31;
    }
    pstBloomfilter->cInitFlag = 1;

    printf(">>> Load BloomFilter(n=%u, p=%f, m=%u, k=%d), malloc() size=%.2fMB\n",
        pstBloomfilter->dwMaxItems, pstBloomfilter->dProbFalse, pstBloomfilter->dwFilterBits,
        pstBloomfilter->dwHashFuncs, (double)pstBloomfilter->dwFilterSize/1024/1024);

    fclose(pFile);
    return 0;
}

FORCE_INLINE uint64_t MurmurHash2_x64 ( const void * key, int len, uint32_t seed );
//两个哈希结果
FORCE_INLINE void bloom_hash(BaseBloomFilter *pstBloomfilter, const void * key, int len)
{
	int i;
	uint32_t dwFilterBits = pstBloomfilter->dwFilterBits;;
	uint64_t hash1 = MurmurHash2_x64(key, len, pstBloomfilter->dwSeed);
	uint64_t hash2 = MurmurHash2_x64(key, len, MIX_UINT64(hash1));

	//哈希函数的个数
	for(i=0; i<(int)pstBloomfilter->dwHashFuncs; i++)
	{
		pstBloomfilter->pdwHashPos[i] = (hash1 + i*hash2) % dwFilterBits;
	}
	return ;
}

FORCE_INLINE uint64_t MurmurHash2_x64 ( const void * key, int len, uint32_t seed )
{
	const uint64_t m = 0xc6a4a7935bd1e995;
	const int r = 47;

	uint64_t h = seed ^ (len * m);
	const uint64_t * data = (const uint64_t *)key;
	const uint64_t * end = data + (len/8);

	while(data != end)
	{
		uint64_t k = *data++;

		k *= m; 
		k ^= k >> r; 
		k *= m; 
		
		h ^= k;
		h *= m;
	}

	const uint8_t * data2 = (const uint8_t*)data;

	switch(len & 7)
	{
		case 7: h ^= ((uint64_t)data2[6]) << 48;
		case 6: h ^= ((uint64_t)data2[5]) << 40;
		case 5: h ^= ((uint64_t)data2[4]) << 32;
		case 4: h ^= ((uint64_t)data2[3]) << 24;
		case 3: h ^= ((uint64_t)data2[2]) << 16;
		case 2: h ^= ((uint64_t)data2[1]) << 8;
		case 1: h ^= ((uint64_t)data2[0]);
		        h *= m;
	};
 
	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}
#endif