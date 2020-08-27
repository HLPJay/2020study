#include <stdio.h>

//bitmap的内存申请 0x1F = 31
//i & 0x1F 取余数  也就是其他就刚好是倍数
// >>右移一位相当于除以2 多位相当于2的n次方  2^5=32
// i>>5其实就是i/32求出的是数组对应的位置 
// 然后 & 对应的i取余的位置 取数据或者标志
int a[1+10000000/32];
void set(int i)
{
	a[i>>5] |= (1<<(i & 0x1F));
}

void clear(int  i)
{
	a[i>>5] &= ~(1<<(i & 0x1F));
}

int test(int i)
{
	return a[i>>5] & (1<<(i & 0x1F));
}

int main()
{
	int i;
	for(i =0; i<10000000; i++)
	{
		clear(i);
	}

	while(scanf("%d", &i) != EOF)
		set(i);

	for(i =0; i<10000000; i++)
		if(test(i))
			printf("%d \n", i);
	return 0;
}
