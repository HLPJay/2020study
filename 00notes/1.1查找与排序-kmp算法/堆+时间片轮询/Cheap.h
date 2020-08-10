#ifndef CHEAP_H
#define CHEAP_H
#include <iostream>
#include <string>
#include <vector>
//设计一个堆的模板类 二叉树的形式
typedef enum {
    MIN_HEAP = 0,
    MAX_HEAP=1
}heap_type;

//堆里存的数据类型   堆的大小堆方式    是否真正删除
/***************************************************
基于堆实现的时间片轮询事件
	堆==》可以被看做是完全二叉树
	可以用数组来实现堆的存储，根据结构去进行上浮或者下沉
区分大堆和小堆
	left  = 2*p + 1 - i;(从i开始的索引p)
	right = 2*p + 2 - i;
	p = (left - 1 + i) / 2;
	p = (right - 1 + i) / 2;

 * ***************************************************/
 
 
template<class _Type, const heap_type _type = MIN_HEAP, const bool _flag = true>
class Cheap
{
public:
    Cheap();
    ~Cheap();
public://增加  删除 获取  输出
    void Insert(const _Type &value);
    void DeleteTop();
    _Type & GetTop();

     //const修饰函数 说明内部不会修改任何成员变量
    bool Empty() const {return m_heapsize<=0;}
    bool Size() const {return m_heapsize;}
public://仅仅为测试，不合理类型
    void Print();
    void TestSort(std::vector<_Type> &vec);
    void initHeapWithVector(std::vector<_Type> &vec);
private:
    int GetLastIndex() const {return m_topIndex+m_heapsize-1;}
    int GetTopIndex() const {return m_topIndex; }

    void Up(int idx);
    void Down(int idx);
    bool Compare(int pIndex, int cIndex);
    bool OutRange(int idx) const;

    int GetParentIndex(int idx) {return (idx-1 +GetTopIndex())/2;}
    int GetLeftIndex(int idx) { return idx*2+1 -GetTopIndex();}
private:
    int m_topIndex;
    int m_heapsize;
    std::vector <_Type> m_heap;
};



template<class _Type, const heap_type _type, const bool _flag>
Cheap<_Type,_type,_flag >::Cheap():m_topIndex(0), m_heapsize(0)
{}

template<class _Type,  heap_type _type, const bool _flag>
Cheap<_Type,_type,_flag >::~Cheap()
{}
//最底层的最右边，然后上浮
template<class _Type, const heap_type _type, const bool _flag>
void Cheap<_Type,_type,_flag >::Insert(const _Type &value)
{
    //堆数据的插入 计算索引，插在最后索引+1的地方，然后上浮
    int idx = Empty()?0:GetLastIndex()+1;
    m_heap.insert(m_heap.begin()+idx, value);
    m_heapsize++;
    Up(GetLastIndex());
}

template<class _Type, const heap_type _type, const bool _flag>
void Cheap<_Type,_type,_flag >::DeleteTop()
{
    if(Empty())  {return;}
    //把堆顶的元素和最后的元素更换  然后下沉
//    _Type top = GetTop();
    std::swap(m_heap[GetTopIndex()], m_heap[GetLastIndex()]);
    if(_flag) m_heap.erase(m_heap.end()-1);
    m_heapsize --;
    Down(GetTopIndex());
}

template<class _Type, const heap_type _type, const bool _flag>
_Type & Cheap<_Type,_type,_flag >::GetTop()
{
    return m_heap[GetTopIndex()];
}


template<class _Type, const heap_type _type, const bool _flag>
void Cheap<_Type,_type,_flag >::Up(int idx)
{
    if(Empty() || (idx == GetTopIndex()))
    {
        return;
    }
    //获取父节点的值，和父节点作比较，交换
    int index = idx;
    int parent_index = GetParentIndex(index);
    while(Compare(parent_index, idx) && index> GetTopIndex())
    {
        std::swap(m_heap[parent_index], m_heap[index]);
        index = parent_index;
        parent_index =GetParentIndex(index);
    }

}

template<class _Type, const heap_type _type, const bool _flag>
void Cheap<_Type,_type,_flag >::Down(int idx)
{
     if(Empty() || (idx == GetLastIndex()))
     {
         return;
     }
     //获取子节点，判断子节点是否越界，然后进行交换
    int index = idx;
    int child =GetLeftIndex(index);
    while(!OutRange(child))//GetLastIndex()
    {
        if(Compare(child, child+1))
        {
            child++;
        }
        if(!Compare(index, child))
        {
            break;
        }
        std::swap(m_heap[index], m_heap[child]);
        index = child;
        child = GetLeftIndex(index);
    }
    return ;
}
template<class _Type, const heap_type _type, const bool _flag>
bool Cheap<_Type,_type,_flag >::Compare(int pIndex, int cIndex)
{
    if(OutRange(pIndex) || OutRange(cIndex))
    {
        return false;
    }
    //返回ture需要调整
    if(_type==MIN_HEAP && m_heap[pIndex] >m_heap[cIndex])
    {
        return true;
    }
    if(_type==MAX_HEAP && m_heap[pIndex] <m_heap[cIndex])
    {
        return true;
    }
    return false;
}
template<class _Type, const heap_type _type, const bool _flag>
bool Cheap<_Type,_type,_flag >::OutRange(int idx) const
{
    return (idx>= GetTopIndex()+m_heapsize)?true:false;
}

//从头到尾从左到右依次输出，
template<class _Type, const heap_type _type, const bool _flag>
void Cheap<_Type,_type,_flag >::Print()
{
    if(Empty())
    {
       std:: cout<<"m_heap is null \n";
        return ;
    }
    for(int i=GetTopIndex(); i<=GetLastIndex();i++)
    {
        std::cout<<m_heap[i]<<" ";
    }
    std::cout<<std::endl;
}
/***********************************************
//塞入一个无序队列，测试其可用性
        构建堆；
        打印堆；
        删除堆；
        交换vector
***********************************************/
template<class _Type, const heap_type _type, const bool _flag>
void Cheap<_Type,_type,_flag >::TestSort(std::vector<_Type> &vec)
{
     if(_flag) {
         std::cout<<"really delete \n";
     }
    initHeapWithVector(vec);
    while(Size())
        DeleteTop();
    m_heap.swap(vec);
    m_heapsize =0;
}

template<class _Type, const heap_type _type, const bool _flag>
void Cheap<_Type,_type,_flag >::initHeapWithVector(std::vector<_Type> &vec)
{
    m_heap.clear();
    m_heap.swap(vec);

    //如果TopIndex不为0 则要插入在对应的位置
    m_heapsize = m_heap.size();

    //构建堆结构，从一半向前的位置开始下沉
    for(int i = GetLastIndex()/2; i>=GetTopIndex(); --i)
    {
        Down(i);
    }
}
#endif // CHEAP_H