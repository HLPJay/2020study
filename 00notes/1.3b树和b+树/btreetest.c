
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

typedef int KEY_TYPE;
#define NDEGREE 3

//DEGREE = NDEGREE *2   M
typedef struct _btree_node{
	KEY_TYPE *keys;   //存的值
	struct _btree_node **childrens;  //存储子节点指针

	int num;
	int leaf;//是否是最底层节点
}btree_node;

typedef struct _btree{
	struct _btree_node *root;
	int degree;
}btree;

//创建 创建btree   创建node
btree_node* btree_create_node (int degree, int leaf);
void btree_create(btree* T, int d);
void btree_destroy_node(btree_node *node);


void btree_split_child(btree *T, btree_node *x, int i);
void btree_insert_nonfull(btree *T, btree_node *x, KEY_TYPE key);
void btree_insert(btree *T, KEY_TYPE key);

void btree_print(btree *T, btree_node *node, int layer);

void btree_merge(btree *T, btree_node *node, int idx);
void btree_delete_key(btree *T, btree_node *node, KEY_TYPE key);
int btree_delete(btree *T, KEY_TYPE key);
void btree_traverse(btree_node *T);
int main()
{
	btree T = {0};

	btree_create(&T, 3);

	char key[26] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	for(int i=0; i<26; i++)
	{
		printf("%c ", key[i]);
		btree_insert(&T, key[i]);
	}

	btree_print(&T, T.root, 0);

	for(int i=0;i<26;i++)
	{
		printf("\n-------------------------------\n");
		btree_delete(&T, key[25-i]);
		btree_traverse(T.root);
		btree_print(&T, T.root, 0);
	}
}


//创建 创建btree   创建node
btree_node* btree_create_node (int degree, int leaf)
{
	btree_node *node = (btree_node*)calloc(1, sizeof(btree_node));
	if(node == NULL)
	{
		assert(0);
	}
	node->leaf = leaf;
	node->keys = (KEY_TYPE*)calloc(1, (2*degree-1)*sizeof(KEY_TYPE));
	node->childrens = (btree_node**)calloc(1, (2*degree) * sizeof(btree_node));
	node->num = 0;
	return node;

}
void btree_create(btree* T, int d)
{
	T->degree = d;

	btree_node *x = btree_create_node(d, 1);
	T->root = x;
}

void btree_destroy_node(btree_node *node)
{
	assert(node);
	free(node->childrens);
	free(node->keys);
	free(node);
}

void btree_insert(btree *T, KEY_TYPE key)
{
	btree_node *r = T->root;
	if(r->num == 2 * T->degree -1)//创建根节点
	{
		btree_node *node = btree_create_node(T->degree, 0);
		T->root = node;

		node->childrens[0] = r;

		btree_split_child(T, node, 0);

		int i=0;
		if(node->keys[0]<key)
		{
			i++;
		}
		btree_insert_nonfull(T, node->childrens[i], key);
	}
	else{
		btree_insert_nonfull(T, r, key);
	}
}

//裂变
void btree_split_child(btree *T, btree_node *x, int i)
{
	int t = T->degree;

	btree_node *y = x->childrens[i];
	btree_node *z = btree_create_node(t, y->leaf);

	z->num = t-1;

	int j = 0;
	for(j = 0; j <t-1; j++)
	{
		z->keys[j] = y->keys[j+t];
	}

	if(y->leaf == 0)
	{
		for(j = 0; j<t; j++)
		{
			z->childrens[j] = y->childrens[j+t];
		}
	}

	y->num = t-1;
	for(j=x->num; j>=i+1; j--)
	{
		x->childrens[j+1] = x->childrens[j];
	}
	x->childrens[i+1] = z;
	for(j = x->num-1; j>=i; j--)
	{
		x->keys[j+1] = x->keys[j];
	}
	x->keys[i] = y->keys[t-1];
	x->num += 1;
}

void btree_insert_nonfull(btree *T, btree_node *x, KEY_TYPE key)
{
	int i = x->num -1;
	if(x->leaf == 1)//根结点
	{
		while(i>= 0 && x->keys[i] > key)
		{
			x->keys[i+1] = x->keys[i];
			i --;
		}
		x->keys[i+1] = key;
		x->num += 1;
	}
	else
	{
		while(i>=0 && x->keys[i] >key)
		{
			i--;
		}
		if (x->childrens[i+1]->num == (2*(T->degree))-1) {
			btree_split_child(T, x, i+1);
			if (key > x->keys[i+1]) 
			{
				i++;
			}
		}
		btree_insert_nonfull(T, x->childrens[i+1], key);
	}
}


void btree_print(btree *T, btree_node *node, int layer)
{
	btree_node *p = node;
	int i;
	if(p)
	{
		printf("\nlayer = %d keynum = %d is_leaf = %d\n", layer, p->num, p->leaf);
		for(i = 0; i < node->num; i++)
			printf("%c ", p->keys[i]);
		printf("\n");
		layer++;
		for(i = 0; i <= p->num; i++)
		{
			if(p->childrens[i])
			{
				btree_print(T, p->childrens[i], layer);
			}
		}
	}
	else 
	{
		printf("the tree is empty\n");
	}
}

int  btree_delete(btree *T, KEY_TYPE key)
{
	if(!T->root)
		return -1;
	btree_delete_key(T, T->root, key);
	return 0;
}

void btree_traverse(btree_node *x)
{
	if(x == NULL)
	{
		return ;
	}
	int i =0 ;
	for(i = 0 ;i<x->num; i++)
	{
		if(x->leaf == 0)
		{
			btree_traverse(x->childrens[i]);
		}
		printf("%C ", x->keys[i]);
	}
	if(x->leaf == 0)
	{
		btree_traverse(x->childrens[i]);
	}
}

void btree_delete_key(btree *T, btree_node *node, KEY_TYPE key)
{
	if(node == NULL)
	{
		return ;
	}

	int idx = 0, i;

	while(idx <node->num && key>node->keys[idx])
	{
		idx++;
	}

	if(idx <node->num && key == node->keys[idx])
	{
		if(node->leaf)
		{
			for(i = idx; i <node->num-1; i++)
			{
				node->keys[i] = node->keys[i+1];
			}
			node->keys[node->num-1] = 0;
			node->num --;
			if(node->num ==0)
			{
				free(node);
				T->root = NULL;
			}
			return ;
		}
		else if(node->childrens[idx]->num >= T->degree)
		{
			btree_node *left = node->childrens[idx];
			node->keys[idx] = left->keys[left->num -1];
			btree_delete_key(T,left, left->keys[left->num - 1]);
		}else if(node->childrens[idx+1]->num >= T->degree)
		{
			btree_node *right = node->childrens[idx+1];
			node->keys[idx] = right->keys[0];

			btree_delete_key(T, right, right->keys[0]);
		}else
		{
			btree_merge(T, node, idx);
			btree_delete_key(T, node->childrens[idx], key);
		}
	}else
	{
		btree_node *child = node->childrens[idx];
		if(child ==NULL)
		{
			printf("Cannot delte key = %d \n", key);
			return ;
		}
		if(child ->num ==T->degree -1)
		{
			btree_node *left = NULL;
			btree_node *right = NULL;
			if(idx -1 >=0)
			{
				left = node->childrens[idx -1];
			}
			if(idx + 1 <node->num)
			{
				right = node->childrens[idx+1];
			}
			if((left &&left->num >=T->degree)||(right && right->num >= T->degree))
			{
				int richR = 0;
				if(right)
					richR = 1;
				if(left && right)
					richR = (right->num > left->num) ? 1 : 0;
				if (right && right->num >= T->degree && richR) 
				{
					child ->keys[child->num] = node->keys[idx];
					child ->childrens[child->num+1] = right->childrens[0];
					child ->num ++;

					node->keys[idx] = right->keys[0];
					for (i = 0;i < right->num - 1;i ++) {
						right->keys[i] = right->keys[i+1];
						right->childrens[i] = right->childrens[i+1];
					}

					right->keys[right->num-1] = 0;
					right->childrens[right->num-1] = right->childrens[right->num];
					right->childrens[right->num] = NULL;
					right->num --;
				}
				else
				{
					for(i = child->num; i>0; i--)
					{
						child->keys[i] = child->keys[i-1];
						child->childrens[i+1] = child->childrens[i];
					}
					child->childrens[1] = child->childrens[0];
					child->childrens[0] = left->childrens[left->num];
					child->keys[0] = node->keys[idx-1];
					child->num ++;

					left->keys[left->num-1] = 0;
					left->childrens[left->num] = NULL;
					left->num --;
				}
			}
			else if((!left ||(left->num == T->degree -1))
				&& (!right ||(right->num == T->degree -1)))
			{
				if(left && left->num ==  T->degree -1)
				{
					btree_merge(T, node, idx-1);
					child = left;
				}else if(right && right->num == T->degree-1)
				{
					btree_merge(T, node, idx);
				}
			}
		}
		btree_delete_key(T, child, key);
	}
}
//总共 T->degree-1个
void btree_merge(btree *T, btree_node *node, int idx)
{
	btree_node *left = node->childrens[idx];
	btree_node *right = node->childrens[idx+1];

	int i=0;

	left->keys[T->degree-1] = node->keys[idx];
	for (i = 0;i < T->degree-1;i ++) {
		left->keys[T->degree+i] = right->keys[i];
	}

	if (!left->leaf) {
		for (i = 0;i < T->degree;i ++) {
			left->childrens[T->degree+i] = right->childrens[i];
		}
	}
	left->num += T->degree;

	//destroy right
	btree_destroy_node(right);

	//node 
	for (i = idx+1;i < node->num;i ++) {
		node->keys[i-1] = node->keys[i];
		node->childrens[i] = node->childrens[i+1];
	}
	node->childrens[i+1] = NULL;
	node->num -= 1;

	if (node->num == 0) {
		T->root = left;
		btree_destroy_node(node);
	}
}