/************************
KMP:字符串匹配算法

解决了目标串回溯的问题
*************************/
#include <stdio.h>
#include <string.h>

void make_next(const char *pattern, int *next);
int kmp(const char* text, const char* pattern, int * next);
int main(void)
{
	char *text = "ababxbababababcdababcabddcadfdsss";
	char *pattern = "abcabd";

	int next[20] = {0};
	int idx = kmp(text, pattern, next);
	printf("match pattern : %d\n", idx);
	for (int i = 0;i < strlen(pattern);i ++) {
		printf("%4d", next[i]);
	}
	printf("\n");
}

int kmp(const char* text, const char* pattern, int * next)
{
	int n = strlen(text);
	int m = strlen(pattern);

	make_next(pattern, next);
	int i, q;
	for (i = 0, q = 0;i < n;i ++)
	{
		while (q > 0 && pattern[q] != text[i]) 
		{
			q = next[q-1];
		}

		if (pattern[q] == text[i]) {
			q ++;
		}

		if (q == m) {
			//printf("Pattern occurs with shift: %d\n", (i-m+1));
			break;
		}
	}
	return i-q+1;
}
//前缀和后缀   最大公共元素单元  abcabd
void make_next(const char *pattern, int *next)
{
	int q, k;
	int m = strlen(pattern);

	next[0] = 0;

	for(k = 0, q=1; q<m; q++)
	{
		while(k>0 && pattern[q] != pattern[k])
		{
			k=next[k-1];
		}
		//公共元素为0 最后一个元素和第一个相等
		if(pattern[q] == pattern[k])
		{
			k++;
		}
		next[q] = k;
	}
}