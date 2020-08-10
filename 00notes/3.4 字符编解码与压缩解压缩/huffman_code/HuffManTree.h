/*****************************88
构建哈夫曼树的接口

greater和less是xfunctional.h中的两个结构体，代码如下：
1 //升序队列，小顶堆
2 priority_queue <int,vector<int>,greater<int> > q;
3 //降序队列，大顶堆
4 priority_queue <int,vector<int>,less<int> >q;

********************************/

#ifndef HUFFMAN_TREE_H_
#define HUFFMAN_TREE_H_
#include <iostream>
#include <vector>
#include <queue>
using namespace std;
//哈夫曼树其实就是二叉树 定义其节点
template <class T>
struct HhffManTreeNode
{
	HhffManTreeNode(const T &weight)
	:m_pLeft(NULL)
	,m_pRight(NULL)
	,m_pParent(NULL)
	,m_weight(weight)
	{}
	struct HhffManTreeNode<T>* m_pLeft;
	struct HhffManTreeNode<T>* m_pRight;
	struct HhffManTreeNode<T>* m_pParent;
	T m_weight;
};
// HuffManNode
//管理构建哈夫曼树
template <class T>
class HuffManTree
{
	typedef HhffManTreeNode<T>* PNode;
public:
	//默认是空的哈夫曼
	HuffManTree():m_pRoot(NULL){}
	HuffManTree(T * array, size_t size, const T & invalid)
	{
		CreateHuffManTree(array, size, invalid);
	}
	~HuffManTree() {
		Destory(m_pRoot);
	}

//这里会不会有问题   m_pRoot成员变量一直在变
	void Destory(PNode& m_pRoot)
	{
		if(m_pRoot)
		{
			Destory(m_pRoot->m_pLeft);
			Destory(m_pRoot->m_pRight);
			delete m_pRoot;
			m_pRoot = NULL;
		}
	}
	PNode GetRoot()
	{
		return m_pRoot;
	}

private:
	//每个字符出现的次数  invalid有些字符出现的次数未0 就不构建
	void CreateHuffManTree(T * array, size_t size, const T & invalid)
	{
		//重载比较函数 ==>为堆做准备
		struct PtrNodeCompare{
			bool operator() (PNode n1, PNode n2)
			{
				return n1->m_weight < n2->m_weight;
			}
		};
		//大堆栈 数据类型  数组实现的容器类型 比较函数 ==>构建堆
		priority_queue<PNode, vector<PNode>, PtrNodeCompare> hp; 

		//构建小堆  从小到大排序
		for(int i=0; i<size; i++)
		{
			if(array[i] != invalid) //invalid 为出现次数为0的字符
			{
				hp.push(new HhffManTreeNode<T>(array[i]));  //塞入到哈夫曼树的节点中
			}
		}
		//堆为NULL
		if(hp.empty())
		{
			m_pRoot = NULL;
		}

		//构建哈夫曼树  一直往上构建
		while(hp.size() >1)
		{
			PNode pLeft = hp.top();
            hp.pop();
            PNode pRight = hp.top();
            hp.pop();
            PNode pParent = new HhffManTreeNode<T>(pLeft->m_weight + pRight->m_weight);//左加右的权值，作为新节点
            pParent->m_pLeft = pLeft;
            pLeft->m_pParent = pParent;

            pParent->m_pRight = pRight;
            pRight->m_pParent = pParent;
            hp.push(pParent);
		}
		m_pRoot = hp.top();
	};
	//销毁哈夫曼树

public:
	PNode m_pRoot;
};



#endif //HUFFMAN_TREE_H_
