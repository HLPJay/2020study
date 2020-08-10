/*********************************
每个节点是红色或者黑色
根节点是黑色
每一个叶子节点是黑色
如果一个节点是红色，它的孩子节点就为黑色
从任意一个节点到叶子节点，经过的黑色节点是一样的


sk_buffer
linux进程调度CFS
内存管理
Nginx Timer事件管理
epoll

1：定义结构体
2：研究左旋和右旋
3: 插入一个节点  插入红色的好     只需要关注第四规则
	1：找到插入点
**********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int KEY_TYPE;

#define   RED     1
#define   BLACK   2

typedef struct _rbtree_node{
	unsigned char color;
	struct _rbtree_node *left;
	struct _rbtree_node *right;
	struct _rbtree_node *parent;

	KEY_TYPE key;
	void *value;
}rbtree_node;

typedef struct _rbtree {
	rbtree_node * root;
	rbtree_node * nil;//叶子结点，所有的节点指向同一个哨兵节点
}rbtree;


void rbtree_insert(rbtree *T, rbtree_node *z);
void rbtree_insert_fixup(rbtree *T, rbtree_node *z);
void rbtree_left_rotate(rbtree *T, rbtree_node *x);
void rbtree_right_rotate(rbtree *T, rbtree_node *y);

rbtree_node *rbtree_mini(rbtree *T, rbtree_node *x);
rbtree_node * rbtree_successor(rbtree *T, rbtree_node *x);
void rbtree_delete_fixup(rbtree *T, rbtree_node *x);
rbtree_node * rbtree_delete(rbtree *T, rbtree_node *z);

rbtree_node *rbtree_search(rbtree *T, KEY_TYPE key);
void rbtree_traversal(rbtree *T, rbtree_node *node);
void rbtree_first_traversal(rbtree *T, rbtree_node *node);
void rbtree_last_traversal(rbtree *T, rbtree_node *node);
int main(void)
{
	int keyArray[20] = {24,25,13,35,23, 26,67,47,38,98, 20,19,17,49,12, 21,9,18,14,15};
	rbtree *T = (rbtree *)malloc(sizeof(rbtree));
	if(T == NULL)
	{
		printf("malloc failed");
		return -1;
	}

	T->nil = (rbtree_node *)malloc(sizeof(rbtree_node));
	T->nil ->color = BLACK;
	T->root = T->nil;

	rbtree_node *node = T->nil;
	int i = 0;
	for(i=0;i<20;i++)
	{
		node = (rbtree_node*)malloc(sizeof(rbtree_node));
		node->key = keyArray[i];
		node->value = NULL;

		rbtree_insert(T, node);
	}
	printf("---中序-------------------------------------\n");
	rbtree_traversal(T, T->root);
	printf("----先序------------------------------------\n");
	rbtree_first_traversal(T, T->root);
	printf("----后续-----------------------------------\n");
	rbtree_last_traversal(T, T->root);
	printf("----------------------------------------\n");
	for (i = 0;i < 20;i ++) {

		rbtree_node *node = rbtree_search(T, keyArray[i]);
		printf("delete :%d \n", keyArray[i]);
		rbtree_node *cur = rbtree_delete(T, node);
		free(cur);

		rbtree_traversal(T, T->root);
		printf("----------------------------------------\n");
	}
	

}

//左旋
void rbtree_left_rotate(rbtree *T, rbtree_node *x)
{
	rbtree_node *y = x->right;
	x->right = y->left;
	if(y->left != T->nil)
	{
		y->left->parent = x;
	}

	y->parent = x->parent;
	if(x->parent == T->nil)
	{
		T->root = y;
	}else if(x == x->parent->left)
	{
		x->parent->left = y;
	}else
	{
		x->parent->right = y;
	}
	y->left = x;
	x->parent = y;
}

//右旋
void rbtree_right_rotate(rbtree *T, rbtree_node *y)
{
	rbtree_node *x = y->left;

	y->left = x->right;
	if (x->right != T->nil) {
		x->right->parent = y;
	}

	x->parent = y->parent;
	if (y->parent == T->nil) {
		T->root = x;
	} else if (y == y->parent->right) {
		y->parent->right = x;
	} else {
		y->parent->left = x;
	}

	x->right = y;
	y->parent = x;
}

/*************
对红黑树做调整，满足第四条
*************/
void rbtree_insert_fixup(rbtree *T, rbtree_node *z)
{
	//若父节点是黑色，则不作处理
	while(z->parent->color == RED)
	{
		//叔父节点位于左节点
		if(z->parent == z->parent->parent->left)
		{
			rbtree_node * y = z->parent->parent->right;
			if(y->color == RED)
			{
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent->color = RED;//祖父节点满足红黑树

				z = z->parent->parent;
			}
			else //y 右旋
			{
				if( z == z->parent->right)
				{
					z = z->parent;
					rbtree_left_rotate(T, z);
				}

				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				rbtree_right_rotate(T, z->parent->parent);
			}
		}else {//若果父节点位于右节点
			rbtree_node *y = z->parent->parent->left;
			if(y->color == RED)
			{
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent->color = RED;

				z = z->parent->parent; //z --> RED
			}
			else
			{
				if(z==z->parent->left)
				{
					z=z->parent;
					rbtree_right_rotate(T, z);
				}
				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				rbtree_left_rotate(T, z->parent->parent);
			}
		}
	}
	T->root->color = BLACK;
}

//插入位置
void rbtree_insert(rbtree *T, rbtree_node *z)
{
	rbtree_node *y = T->nil;
	rbtree_node *x = T->root;

	while( x != T->nil)
	{
		y = x;
		if (z->key < x->key) {
			x = x->left;
		} else if (z->key > x->key) {
			x = x->right;
		} else { //Exist
			return ;
		}
	}

	z->parent = y;
	if (y == T->nil) {
		T->root = z;
	} else if (z->key < y->key) {
		y->left = z;
	} else {
		y->right = z;
	}

	z->left = T->nil;
	z->right = T->nil;
	z->color = RED;
	rbtree_insert_fixup(T, z);
}


/*************************
删除节点：
	1：覆盖节点
	2：删除节点
	3：轴心节点

*************************/

rbtree_node *rbtree_mini(rbtree *T, rbtree_node *x)
{
	while(x->left != T->nil)
	{
		x=x->left;
	}
	return x;
}

rbtree_node * rbtree_successor(rbtree *T, rbtree_node *x)
{
	rbtree_node *y = x->parent;
	if(x->right != T->nil)
	{
		return rbtree_mini(T, x->right);
	}

	while((y!=T->nil)&&(x == y->right))
	{
		x = y;
		y=y->parent;
	}
	return y;
}
/*****、
覆盖当前节点 z
确定删除节点 y
轴心节点   x
*****/
rbtree_node * rbtree_delete(rbtree *T, rbtree_node *z)
{
	rbtree_node *y = T->nil;
	rbtree_node *x = T->nil;
//确定删除节点y
	if((z->left == T->nil) ||(z->right == T->nil))
	{
		y=z;
	}
	else {
		y = rbtree_successor(T,z);
	}

//确定轴心节点x
	if(y->left != T->nil)
	{
		x = y->left;
	}else if(y->right != T->nil)
	{
		x = y->right;
	}

	x->parent = y->parent;
	if(y->parent == T->nil)
	{
		T->root = x;
	}
	else if(y == y->parent->left)
	{
		y->parent->left = x;
	}
	else
	{
		y->parent->right = x;
	}

	if (y != z) {
		z->key = y->key;
		z->value = y->value;
	}
	if (y->color == BLACK) {
		rbtree_delete_fixup(T, x);
	}
	return y;
}
//对删除后的红黑树进行颜色调整
void rbtree_delete_fixup(rbtree *T, rbtree_node *x)
{
	while ((x != T->root) && (x->color == BLACK)) {
		if (x == x->parent->left) {

			rbtree_node *w= x->parent->right;
			if (w->color == RED) {
				w->color = BLACK;
				x->parent->color = RED;

				rbtree_left_rotate(T, x->parent);
				w = x->parent->right;
			}

			if ((w->left->color == BLACK) && (w->right->color == BLACK)) {
				w->color = RED;
				x = x->parent;
			} else {

				if (w->right->color == BLACK) {
					w->left->color = BLACK;
					w->color = RED;
					rbtree_right_rotate(T, w);
					w = x->parent->right;
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->right->color = BLACK;
				rbtree_left_rotate(T, x->parent);

				x = T->root;
			}

		} else {

			rbtree_node *w = x->parent->left;
			if (w->color == RED) {
				w->color = BLACK;
				x->parent->color = RED;
				rbtree_right_rotate(T, x->parent);
				w = x->parent->left;
			}

			if ((w->left->color == BLACK) && (w->right->color == BLACK)) {
				w->color = RED;
				x = x->parent;
			} else {

				if (w->left->color == BLACK) {
					w->right->color = BLACK;
					w->color = RED;
					rbtree_left_rotate(T, w);
					w = x->parent->left;
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->left->color = BLACK;
				rbtree_right_rotate(T, x->parent);

				x = T->root;
			}
		}
	}

	x->color = BLACK;
}


//中序遍历     先根 再左  再右  中序遍历左子树  根节点  中序遍历右子树
void rbtree_traversal(rbtree *T, rbtree_node *node)
{
	if(node != T->nil)
	{
		rbtree_traversal(T, node->left);
		printf("key:%d, color:%d\n", node->key, node->color);
		rbtree_traversal(T, node->right);
	}
}

//先序遍历     先根节点，再左遍历，再右遍历
void rbtree_first_traversal(rbtree *T, rbtree_node *node)
{
	if(node != T->nil)
	{
		printf("key:%d, color:%d\n", node->key, node->color);
		rbtree_traversal(T, node->left);
		rbtree_traversal(T, node->right);
	}
}

//后序列遍历   先左  再右  最根节点
void rbtree_last_traversal(rbtree *T, rbtree_node *node)
{
	if(node != T->nil)
	{
		rbtree_traversal(T, node->left);
		rbtree_traversal(T, node->right);
		printf("key:%d, color:%d\n", node->key, node->color);
	}
}

rbtree_node *rbtree_search(rbtree *T, KEY_TYPE key)
{
	rbtree_node *node = T->root;
	while (node != T->nil) {
		if (key < node->key) {
			node = node->left;
		} else if (key > node->key) {
			node = node->right;
		} else {
			return node;
		}	
	}
	return T->nil;
}

