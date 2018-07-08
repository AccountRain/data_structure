#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "MyBTree.h"

#define cmp(a, b) ( ( ((a)-(b)) >= (0) ) ? (1) : (0) )

void BTree_create(BTree* tree,const KeyType* data,int length) {
	int i;
	printf("\n 开始创建 B-树，关键字为:\n");
	for (i = 0; i < length; i++) {
		printf("%c", data[i]);
	}											//每个结点关键字1<=x<=3
	printf("\n");

	for (i = 0; i < length; i++) {
		printf("\n插入关键字 %c:\n", data[i]);
		int pos = -1;
		BTree_search(*tree,data[i],&pos);
		if (pos != -1) {
			printf("this key %c is in the B-tree,not to insert.\n", data[i]);
		}
		else {
			BTree_insert(tree,data[i]);
		}
		BTree_print(*tree,0);
	}
	printf("\n");

}
//这里有一个指针问题：
		//&tree传递到BTree_create时，BTree_create用指向树的指针tree来接收，tree是相当于&tree
		//这时*tree所指的内容就有趣了，*tree是通过所存储的地址找到地址所对应的值，这里指的是树（tree）也就是&tree当中的tree
		//所以*tree改变就能改变原函数的值NULL。
BTNode* BTree_search(const BTree tree, int key, int* pos) {
	printf("BTree_search:\n");
	if (!tree) {
		printf("BTree is NULL!\n");
		return NULL;
	}
	*pos = -1;
	return BTree_recursive_search(tree, key, pos);

}
//keynum A0 K0 A1 K1....AN,KN,AN+1这是结点数据 
BTNode* BTree_recursive_search(const BTree tree, KeyType key, int* pos) {
	int i = 0;
	while (i<tree->Keynum&&key>tree->key[i])
	{
		++i;
	}

	if (i<tree->Keynum&&tree->key[i]==key) {
		*pos = i;
		return tree;
	}

	if (tree->isLeaf)
	{
		return NULL;
	}
	disk_read(&tree->child[i]);
	return BTree_recursive_search(tree->child[i], key, pos);
	//递归的查找子树。
}

void BTree_insert(BTree* tree, KeyType key) {
	printf("BTree_insert:\n");
	BTNode* node;
	BTNode* root = *tree;
	if (root==NULL)
	{
		root = (BTNode*)calloc(sizeof(BTNode), 1);
		if (!root) {
			printf("Error! out of memory!\n");
			return;
		}
		root->isLeaf = true;
		root->Keynum = 1;
		root->key[0] = key;
		root->child[0] = NULL;

		*tree = root;
		disk_write(root);
		return;
	}

	//根结点已满，插入前需要进行分裂调整
	if (root->Keynum==ORDER-1)
	{
		//产生新节点当作根
		node = (BTNode*)calloc(sizeof(BTNode), 1);
		if (!node) {
			printf("Error! out of memory!\n");
			return;
		}
		*tree = node;
		node->isLeaf = false;
		node->child[0] = root;
		node->Keynum = 0;
		BTree_split_child(node,0,root);

		BTree_insert_nonfull(node,key);
	}
	else {
		BTree_insert_nonfull(node, key);
	}
}

void BTree_split_child(BTNode* parent, int index, BTNode* node) {
		int i;
		BTNode* newNode= (BTNode*)calloc(sizeof(BTNode), 1);
		if (!newNode) {
			printf("Error! out of memory!\n");
			return;
		}

		newNode->isLeaf = node->isLeaf;
		//把node的后 BTree_D - 1个关键字转移到新节点
		newNode->Keynum = BTree_D - 1;
		for ( i = 0; i < newNode->Keynum; i++)
		{
			newNode->key[i] = node->key[BTree_D + i];
			node->key[BTree_D + i] = 0;
		}

		//注意叶子结点指的是可以直接插入的结点
		if(!node->isLeaf){
			for (i = 0; i < BTree_D; i++) {
				newNode->child[i] = node->child[BTree_D + i];
				node->child[BTree_D + i] = NULL;
			}
		}
		node->Keynum = BTree_D-1;
		
		//parent指针
		for (i = parent->Keynum;i > index;--i)
			parent->child[i + 1] = parent->child[i];
		
		parent->child[index + 1] = newNode;
		
		//parent关键字
		for ( i = parent->Keynum-1; i>=index ; --i)
		{
			parent->key[i + 1] = parent->key[i];
		}
		parent->key[index] = node->key[BTree_D-1];
		
		++parent->Keynum;
		
		//写入磁盘
		disk_write(parent);		//parent->key[index] = node->key[BTree_D-1];
		disk_write(newNode);    //后BTree_D-1
		disk_write(node);		//前BTree_D-1
}

void BTree_insert_nonfull(BTNode* node, KeyType key) {
	int i;
	//结点是叶子结点
	if (node->isLeaf)
	{
		i = node->Keynum - 1;
		while (i >= 0 && key < node->key[i])
		{
			i--;
		}
		node->key[i + 1] = key;
		++node->Keynum;
		disk_write(node);
	}
	else
	{
		i = node->Keynum - 1;
		while (i >= 0 && key < node->key[i]) {
			--i;
		}

		++i;
		// 从磁盘读取孩子节点
		disk_read(&node->child[i]);

		if (node->child[i]->Keynum == (ORDER - 1)) {
			BTree_split_child(node, i, node->child[i]);
			//如果待插入的关键字大于该分裂结点中上移到父节点的关键字，在该关键字的右孩子结点中进行插入操作。
			if (key > node->key[i]) {
				++i;
			}
		}
		BTree_insert_nonfull(node->child[i],key);
	}
}

void BTree_remove(BTree* tree, KeyType key) {
	if (*tree == NULL) {
		printf("BTree is NULL! you need to remove");
		return;
	}

	BTree_recursive_remove(tree,key);
}

void BTree_recursive_remove(BTree* tree, KeyType key) {
	// B-数的保持条件之一：
	// 非根节点的内部节点的关键字数目不能少于 BTree_D - 1
	// 即子树必须是BTree_D个
	int i, j, index;
	BTNode* root = *tree;
	BTNode* node = root;

	if (!root) {
		printf("Failed to remove %c, it is not in the tree!\n", key);
		return;
	}

	index = 0;
	//这个循坏遍历结点中的元素，如果遇到key=node->key[index]则找到。
	//key<node->key[index]则该值在node->child[index]所指的结点中。
	while (index<node->Keynum&&key>node->key[index])
	{
		++index;
	}
	BTNode *leftChild, *rightChild;
	KeyType leftKey, rightKey;
	//index<node->Keynum是必须的，因为有可能index=node->Keynum
	//这时候node->key[index]不存在。这时候会去最后一个孩子找。
	if (index < node->Keynum && node->key[index] == key) {
		/*
		所在节点是叶子节点，直接删除
		这里的结点一定是富的。富是指关键字个数大于BTree_D-1
		后面有机制保证。
		*/
		if (node->isLeaf) {
			for (i = index; i < node->Keynum - 1; ++i) {
				node->key[i] = node->key[i + 1];
				//本来关键字动了，该关键字对应的child是一定要变的，但是这是叶子结点就省了。
				//这里若是采用node->child[i] = node->child[i+1];去改变child,会导致最后一个没有跟上来，
				//node->child[i + 1] = node->child[i + 2];则不需要考虑这个。
				//但是这里都是NULL,则不用考虑。
			}
			node->key[node->Keynum - 1] = 0;
			node->Keynum--;

			//这里只能是根结点为叶子结点，且只有一个关键字了
			//目前来看是这样。
			if (node->Keynum == 0)
			{
				free(node);
				*tree = NULL;
			}
			return;
		}

		//若是左子树比较富，则用左结点的最大值去覆盖要删除的值。
		//起变相删除的作用。
		//当然还要删除左结点的最大值。所以就是递归删除。
		else if (node->child[index]->Keynum >= BTree_D)
		{
			leftChild = node->child[index];
			leftKey = leftChild->key[leftChild->Keynum - 1];
			node->key[index] = leftKey;
			BTree_recursive_remove(&leftChild, leftKey);
		}
		//同上
		else if (node->child[index + 1]->Keynum >= BTree_D) {
			rightChild = node->child[index + 1];
			rightKey = rightChild->key[0];
			node->key[index] = rightKey;
			BTree_recursive_remove(&rightChild, rightKey);
		}

		//若是都刚脱贫。删除前需要孩子结点的合并操作
		else if (node->child[index]->Keynum == BTree_D - 1
			&& node->child[index + 1]->Keynum == BTree_D - 1) {
			leftChild = node->child[index];
			BTree_merge_child(tree, node, index);
			//node是BTNode类型的指针，转过去后，node-> 是通过地址操作所以对原函数有影响。
			BTree_recursive_remove(&leftChild, key);

		}
	}
	//现在是在该结点中没找到的情况。
	else {
		BTNode *leftSibling, *rightSibling, *child;
		//key应该在node->child[index]里面
		child = node->child[index];
		if (!child) {
			printf("Failed to remove %c, it is not in the tree!\n", key);
			return;
		}
		/*要查找的结点刚脱贫的情况*/
		if (child->Keynum == BTree_D - 1) {
			leftSibling = NULL;
			rightSibling = NULL;

			if (index - 1 >= 0)
				leftSibling = node->child[index-1];
			if (index + 1 <= node->Keynum)
				rightSibling = node->child[index+1];
			//左孩子或者右孩子有一个存在且是富的
			if ((leftSibling && leftSibling->Keynum >= BTree_D)
				|| (rightSibling && rightSibling->Keynum >= BTree_D))
			{
				int richR = 0;
				if (rightSibling) richR = 1;
				if (leftSibling && rightSibling) {
					richR = cmp(rightSibling->Keynum, leftSibling->Keynum);
				}

				if (rightSibling && rightSibling->Keynum >= BTree_D && richR) {
					child->key[child->Keynum] = node->key[index];
					child->child[child->Keynum + 1] = rightSibling->child[0];
					++child->Keynum;

					node->key[index] = rightSibling->key[0];

					for (j = 0; j < rightSibling->Keynum - 1; ++j) {//元素前移
						rightSibling->key[j] = rightSibling->key[j + 1];
						rightSibling->child[j] = rightSibling->child[j + 1];
					}
					rightSibling->key[rightSibling->Keynum - 1] = 0;
					rightSibling->child[rightSibling->Keynum - 1] = rightSibling->child[rightSibling->Keynum];
					rightSibling->child[rightSibling->Keynum] = NULL;
					--rightSibling->Keynum;
				}
				else {//相邻左兄弟相对富有，则该孩子向父节点借一个元素，左兄弟中的最后元素上移至父节点所借位置，并进行相应调整。
					for (j = child->Keynum; j > 0; --j) {
						child->key[j] = child->key[j - 1];
						child->child[j + 1] = child->child[j];
					}//后移
					child->child[1] = child->child[0];//继续上面后移
					child->child[0] = leftSibling->child[leftSibling->Keynum];
					child->key[0] = node->key[index - 1];
					++child->Keynum;
					//上移
					node->key[index - 1] = leftSibling->key[leftSibling->Keynum - 1];
					//调整
					leftSibling->key[leftSibling->Keynum - 1] = 0;
					leftSibling->child[leftSibling->Keynum] = NULL;

					--leftSibling->Keynum;
				}
			}
			else if ((!leftSibling || (leftSibling && leftSibling->Keynum == BTree_D - 1))
				&& (!rightSibling || (rightSibling && rightSibling->Keynum == BTree_D - 1))) {
				if (leftSibling && leftSibling->Keynum == BTree_D - 1) {

					BTree_merge_child(tree, node, index - 1);//node中的右孩子元素合并到左孩子中。
					//把自己合并到左边
					child = leftSibling;
				}

				else if (rightSibling && rightSibling->Keynum == BTree_D - 1) {
					//把右边合并到自己
					BTree_merge_child(tree, node, index);//node中的右孩子元素合并到左孩子中。
				}

			}
		}
		BTree_recursive_remove(&child, key);
	}
}



void BTree_merge_child(BTree* tree, BTNode* node, int index) {
	printf("BTree_merge_child!\n");
	int i;
	KeyType key = node->key[index];
	BTNode* leftChild = node->child[index];
	BTNode* rightChild = node->child[index+1];
	
	//将node结点的key[index]下移到左孩子，并把右孩子的第一个孩子复制给左孩子的最后一个孩子。
	leftChild->key[leftChild->Keynum] = key;
	leftChild->child[leftChild->Keynum + 1] = rightChild->child[0];
	++leftChild->Keynum;
	
	//有孩子的元素合并到左孩子当中。
	for (i = 0; i < rightChild->Keynum; ++i) {
		leftChild->key[leftChild->Keynum] = rightChild->key[i];
		leftChild->child[leftChild->Keynum + 1] = rightChild->child[i + 1];
		++leftChild->Keynum;
	}

	//既然key[index]下移，则要将index后面的元素前移
	for (i = index;i < node->Keynum - 1;i++) {
		node->key[i] = node->key[i+1];
		node->child[i + 1] = node->child[i+2];
		//左孩子还在，右孩子被覆盖
	}
	node->child[node->Keynum] = NULL;
	node->Keynum--;
	node->key[node->Keynum - 1] = 0;
	
	//node->Keynum == 0只有根结点可以做到了
	if (node->Keynum == 0) {
		if (*tree == node) {
			*tree = leftChild;
		}

		free(node);
		node = NULL;
	}

	free(rightChild);
	rightChild = NULL;

}

void BTree_destroy(BTree* tree)
{
	int i;
	BTNode* node = *tree;

	if (node) {
		for (i = 0; i <= node->Keynum; i++) {
			BTree_destroy(&node->child[i]);
		}

		free(node);
	}

	*tree = NULL;
}

void disk_write(BTNode* node)
{
	//打印出结点中的全部元素，方便调试查看keynum之后的元素是否为0(即是否存在垃圾数据)；而不是keynum个元素。
	printf("向磁盘写入节点");
	for (int i = 0;i<ORDER - 1;i++) {
		printf("%c", node->key[i]);
	}
	printf("\n");
}

void disk_read(BTNode** node)
{
	//打印出结点中的全部元素，方便调试查看keynum之后的元素是否为0(即是否存在垃圾数据)；而不是keynum个元素。
	printf("向磁盘读取节点");
	for (int i = 0;i<ORDER - 1;i++) {
		printf("%c", (*node)->key[i]);
	}
	printf("\n");
}


void BTree_print(const BTree tree, int layer)
{
	int i;
	BTNode* node = tree;

	if (node) {
		printf("第 %d 层， %d node : ", layer, node->Keynum);

		//打印出结点中的全部元素，方便调试查看keynum之后的元素是否为0(即是否存在垃圾数据)；而不是keynum个元素。
		for (i = 0; i < ORDER - 1; ++i) {
			//for (i = 0; i < node->keynum; ++i) {
			printf("%c ", node->key[i]);
		}

		printf("\n");

		++layer;
		for (i = 0; i <= node->Keynum; i++) {
			if (node->child[i]) {
				BTree_print(node->child[i], layer);
			}
		}
	}
	else {
		printf("树为空。\n");
	}
}

int binarySearch(BTNode* node, int low, int high, KeyType Fkey)
{
	int mid;
	while (low <= high)
	{
		mid = low + (high - low) / 2;
		if (Fkey<node->key[mid])
		{
			high = mid - 1;
		}
		if (Fkey>node->key[mid])
		{
			low = mid + 1;
		}
		if (Fkey == node->key[mid])
		{
			return mid;//返回下标。
		}
	}
	return 0;//未找到返回0.
}
