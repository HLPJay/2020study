/******************************8
排序：
	
冒泡 选择  插入
希尔 归并  快速排序
堆 计数  桶  基数

内排序，外排序

内排序：指在排序期间数据对象全部存放在内存的排序。

外排序：指在排序期间全部对象太多，不能同时存放在内存中，必须根据排序过程的要求，不断在内，外存间移动的排序。
*******************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define DATA_ARRAY_LENGTH 12

void display(int *data, unsigned int length);
int bubble_sort(int *data, unsigned int length);
int select_sort(int *data, unsigned int length);
int insert_sort(int *data, unsigned int length);
int shell_sort(int *data, unsigned int length);
int merge_sort(int *data, int *temp, int start, int end) ;
int fast_sort(int *data, unsigned int length);
int stack_sort(int *data, unsigned int length);
int count_sort(int *data, unsigned int length);
int barrel_sort(int *data, unsigned int length);
int base_sort(int *data, unsigned int length);
int main(void)
{
	
	int ret = 0;
	//冒泡排序
	int data[DATA_ARRAY_LENGTH] = {23, 64, 24, 12, 9, 16, 53, 57, 71, 79, 87, 97};
	bubble_sort(data, DATA_ARRAY_LENGTH);
	display(data, DATA_ARRAY_LENGTH);

	//选择排序
	int data1[DATA_ARRAY_LENGTH] = {23, 64, 24, 12, 9, 16, 53, 57, 71, 79, 87, 97};
	select_sort(data1, DATA_ARRAY_LENGTH);
	display(data1, DATA_ARRAY_LENGTH);
	
	//插入排序
	int data2[DATA_ARRAY_LENGTH] = {23, 64, 24, 12, 9, 16, 53, 57, 71, 79, 87, 97};
	ret != insert_sort(data2, DATA_ARRAY_LENGTH);
	display(data2, DATA_ARRAY_LENGTH);

	//希尔排序 
	int data3[DATA_ARRAY_LENGTH] = {23, 64, 24, 12, 9, 16, 53, 57, 71, 79, 87, 97};
	ret != shell_sort(data3, DATA_ARRAY_LENGTH);
	display(data3, DATA_ARRAY_LENGTH);

	//归并排序
	int temp[DATA_ARRAY_LENGTH] = {0};
	int data4[DATA_ARRAY_LENGTH] = {23, 64, 24, 12, 9, 16, 53, 57, 71, 79, 87, 97};
	ret != merge_sort(data4, temp, 0, DATA_ARRAY_LENGTH-1);
	display(data4, DATA_ARRAY_LENGTH);

	//快速排序
	int data5[DATA_ARRAY_LENGTH] = {23, 64, 24, 12, 9, 16, 53, 57, 71, 79, 87, 97};
	ret != fast_sort(data5, DATA_ARRAY_LENGTH);
	display(data4, DATA_ARRAY_LENGTH);

	//堆排序
	int data6[DATA_ARRAY_LENGTH] = {23, 64, 24, 12, 9, 16, 53, 57, 71, 79, 87, 97};
	ret != stack_sort(data6 , DATA_ARRAY_LENGTH);
	display(data6, DATA_ARRAY_LENGTH);

	//计数排序
	int data7[DATA_ARRAY_LENGTH] = {23, 64, 24, 12, 9, 16, 53, 57, 71, 79, 87, 97};
	ret != count_sort(data7 , DATA_ARRAY_LENGTH);
	display(data7, DATA_ARRAY_LENGTH);

	//桶排序
	int data8[DATA_ARRAY_LENGTH] = {23, 64, 24, 12, 9, 16, 53, 57, 71, 79, 87, 97};
	ret != barrel_sort(data8 , DATA_ARRAY_LENGTH);
	display(data8, DATA_ARRAY_LENGTH);

	//基数排序
	int data9[DATA_ARRAY_LENGTH] = {23, 64, 24, 12, 9, 16, 53, 57, 71, 79, 87, 97};
	ret != base_sort(data9 , DATA_ARRAY_LENGTH);
	display(data9, DATA_ARRAY_LENGTH);

	if(ret != 0)
	{
		printf("error \n");
	}
	return 0;
}

void display(int *data, unsigned int length)
{
	printf("sort :");
	for(int i =0 ;i <length; i++)
	{
		printf("%d  ", data[i]);
	}
	printf("\n");
}

//冒泡排序 交换相邻元素，直到没有交换,最后一个元素是必定的
int bubble_sort(int *data, unsigned int length)
{
	int i, j, temp;
	for(i = 0; i<length-1; i++)//遍历次数
	{
		for(j=0; j<length-i-1; j++)//遍历比较
		{
			if(data[j] >data[j+1])
			{
				temp = data[j];
				data[j] = data[j+1];
				data[j+1] = temp;
			}
		}
	}
	return 0;
}
//选择排序  依次找出最小元素
int select_sort(int *data, unsigned int length)
{
	int i, j, temp;
	for(i =0; i<length-1; i++)
	{
		for(j = i+1; j<length; j++)
		{
			if(data[i] >data[j])
			{
				temp = data[i];
				data[i] = data[j];
				data[j] = temp;
			}
		}
	}
	return 0;
}
//23, 64, 24, 12, 9, 16, 53, 57, 71, 79, 87, 97
int insert_sort(int *data, unsigned int length)
{
	int i, j, temp;
#if 0
	for(i = 1; i<length; i++)
	{
		temp = data[i];
		bool flag = false;
		for(j = 0; j<=i; j++)
		{
			if(data[j] > temp)
			{
				flag = true;
			}
			if(flag)
			{
				int temp1;
				temp1 = data[j];
				data[j] = temp;
				temp = temp1;
			}
		}
	}
#elif 0
#else
	for( i =1; i<length; i++)
	{
		temp = data[i];
		for(j = i; j>0&& data[j-1]>temp; j--)
		{
			data[j] = data[j-1];
		}
		data[j] = temp;
	}
#endif
	return 0;
}

//希尔排序 按增量分组插入排序
int shell_sort(int *data, unsigned int length)
{
	int gap = 0;
	int i = 0, j = 0;
	int temp;

	for (gap = length / 2;gap >= 1; gap /= 2) {
		for (i = gap;i < length;i ++) 
		{
			temp = data[i];
			for (j = i-gap;j >= 0 && temp < data[j];j = j - gap) 
			{
				data[j+gap] = data[j];
			}
			data[j+gap] = temp;
		}
	}
	return 0;
}

// 归并排序 先使每个子序列有序，再使子序列段间有序
void merge(int *data, int *temp, int start, int middle, int end);
int merge_sort(int *data, int *temp, int start, int end) 
{
	int middle;
	if(start < end)
	{
		middle = (start + end) /2;
		merge_sort(data, temp, start, middle);
		merge_sort(data, temp, middle+1, end);

		merge(data, temp, start, middle, end);
	}
	return 0;
}

void merge(int *data, int *temp, int start, int middle, int end)
{
	int i = start, j = middle+1, k = start;

	while (i <= middle && j <= end)
	{
		if(data[i] >data[j])
		{
			temp[k++] = data[j++];
		} else 
		{
			temp[k++] = data[i++];
		}
	}

	while ( i <= middle)
	{
		temp[k++] = data[i++];
	}

	while( j <= end)
	{
		temp[k++] = data[j++];
	}

	for(i = start; i<=end; i++)
	{
		data[i] = temp[i];
	}
}

//快速排序 取临界值，小的放在临界值左边，大的放在临界值右边
void sort(int *data, int left, int right);
int fast_sort(int *data, unsigned int length)
{
	sort(data, 0, length-1);
	return 0;
}

void sort(int *data, int left, int right) 
{
	if(left>right)
		return ;

	int i = left;
	int j = right;
	int key = data[left];

	while(i<j)
	{
		while(i<j && key<=data[j])
		{
			j--;
		}
		data [i] = data [j];
		while(i<j && key>= data[i])
		{
			i++;
		} 
		data [j] = data[i]; 
	}
	data[i] = key;
	sort(data, left, i - 1);
	sort(data, i + 1, right);
	return ;
}
//堆排序
void swap(int *a, int *b);
void max_heapify(int arr[], int start, int end);
int stack_sort(int *data, unsigned int length)
{
	int i = 0 ;
	//构建大堆顶 最后一个非叶子结点开始  从下至上 从右到左
	for( i = (length-1)/2; i>=0; i--)
		max_heapify(data, i, length -1);
	//把最大元素根节点元素沉底
	for( i = length -1; i>0; i--)
	{
		swap(&data[0], &data[i]);
		max_heapify(data, 0, i-1);
	}
	return 0;
}
//取出根节点中最大的 遍历儿子节点
void max_heapify(int arr[], int start, int end) 
{
	int dad = start;
	int son = dad * 2 +1; //左右节点相邻，这是左节点
	while(son <= end)
	{
		if(son+1 <= end && arr[son]<arr[son+1])
		{
			son++;
		}

		if(arr[dad] > arr[son])
		{
			return ;
		}
		else
		{
			swap(&arr[dad], &arr[son]);
			dad = son;
			son = dad*2+1;
		}
	}
	return ;
}

void swap(int *a, int *b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}
//计数排序   遍历比当前数小的个数，然后构建数组存储
int count_sort(int *data, unsigned int length)
{
	int i,j,count,*data_p;
    data_p=(int*)malloc(sizeof(int)*length);
    // memset(data_p, 0, sizeof(int) *length);
    for(i=0;i<length;i++)//初始化data_p
        data_p[i]=0;
    for(i=0;i<length;i++)
    {
        count=0;
        for(j=0;j<length;j++)//扫描待排序数组
            if(data[j]<data[i])//统计比data[i]值小的值的个数
                count++;
        while(data_p[count]!=0)
        {
        	count++;
        }
        //对于相等非0的数据，应向后措一位。数据为0时，因数组data_p被初始化为0，故不受影响。
        /* 注意此处应使用while循环进行判断，若用if条件则超过三个重复值后有0出现 */    
        
        data_p[count]=data[i];//存放到data_p中的对应位置
    }
        //用于检查当有多个数相同时的情况
    i=0,j=length;
    while(i<j)
    {
        if(data_p[i]==0)
        {
            data_p[i]=data_p[i-1];
        }
        i++;
    }//of  while
    for(i=0;i<length;i++)//把排序完的数据复制到data中
        data[i]=data_p[i];
    free(data_p);//释放data_p
    return 0;
}
//桶排序 和计数排序类似，桶个数越多，效率越高，桶内排序
//假设 max-min个桶
int barrel_sort(int *data, unsigned int length)
{

	if(data == NULL)
		return -1;
	int max =data[0],  min = data[0];
	for(int i =0; i<length; i++)
	{
		if(data[i] > max)
		{
			max = data[i];
		}
		if(data[i] < min)
		{
			min = data[i];
		}
	}
	int size = (max - min + 1);
	int *count = (int *) malloc(sizeof(int)*(max - min + 1));
	memset(count, 0, sizeof(int) *(max - min + 1));
	for(int i=0;i<length;i++)
	{
		count[data[i]-min]++;
	}
	int z =0;
	for(int i=0; i<size; i++)
	{
		if(count[i] == 0)
			continue;
		for(int j =0; j<count[i]; j++)
		{
			data[z++] = i+min;
		}
	}
	free(count);
	count = NULL;
	return 0;
}

//基数排序
int GetMaxNum(int *p, int n);
int getLoopTimes(int num);
void sort2(int *p, int length, int loop);
int base_sort(int *data, unsigned int length)
{
	int max = GetMaxNum(data, length);
	int loopTimes = getLoopTimes(max);
	for(int i=1; i<loopTimes; i++)
	{
		 sort2(data, length, i);
	}
	return 0;
}

int GetMaxNum(int *p, int n)
{
    int max = 0;
    for(int i = 0; i < n; i++)
    {
        if(*(p + i) > max)
        {
            max = *(p + i);
        }
    }
    return max;
}

int getLoopTimes(int num)
{
	int count = 1;
    int temp = num / 10;
    while(temp != 0)
    {
        count++;
        temp = temp / 10;
    }
    return count;
}

void sort2(int *p, int length, int loop)
{
	int buckets[10][20] = {};//20根据最大数可以分配
	int tempNum = (int)pow(10, loop );
	int i, j;
    for(i = 0; i < length; i++)
    {
        int row_index = (*(p + i) / tempNum) % 10;
        for(j = 0; j < 20; j++)
        {
            if(buckets[row_index][j] == 0)
            {
                buckets[row_index][j] = *(p + i);
                break;
            }
        }
    }

    int k = 0;
    for(i = 0; i < 10; i++)
    {
        for(j = 0; j < 20; j++)
        {
            if(buckets[i][j] != 0)
            {
                *(p + k) = buckets[i][j];
                buckets[i][j] = 0;
                k++;
            }
        }
    }
    return;
}