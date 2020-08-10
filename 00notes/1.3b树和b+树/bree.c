



typedef int KEY_VALUE;

typedef struct _btree_node {
	KEY_VALUE *keys;
	struct _btree_node **childrens;
	int num;
	int leaf;
}btree_node;

typedef struct _btree{
	btree_node * root;
	int t;
}btree;

btree_node * btree_create_node(int t, int leaf);
void btree_create(btree *T, int t);
int main()
{
	btree T = {0};

	bree_create(&T, 3);
}

void btree_create(btree *T, int t)
{
	T->t = t;
	btree_node *x = btree_create_node(t, 1);
	T->root = x;
}

//t值决定了每个节点存储的个数    2*t -1
btree_node * btree_create_node(int t, int leaf)
{
	btree_node *node = (btree_node*)calloc(1, sizeof(btree_node));
	if(node == NULL)
	{
		assert(0);
	}

	node->leaf = leaf;
	node->keys = (KEY_VALUE*)calloc(1, (2*t-1)*sizeof(KEY_VALUE));
	node->childrens = (btree_node**)calloc(1, (2*t) * sizeof(btree_node));
	node->num = 0;
	return node;
}