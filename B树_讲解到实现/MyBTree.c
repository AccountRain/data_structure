#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "MyBTree.h"

#define cmp(a, b) ( ( ((a)-(b)) >= (0) ) ? (1) : (0) )

void BTree_create(BTree* tree,const KeyType* data,int length) {
	int i;
	printf("\n ��ʼ���� B-�����ؼ���Ϊ:\n");
	for (i = 0; i < length; i++) {
		printf("%c", data[i]);
	}											//ÿ�����ؼ���1<=x<=3
	printf("\n");

	for (i = 0; i < length; i++) {
		printf("\n����ؼ��� %c:\n", data[i]);
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
//������һ��ָ�����⣺
		//&tree���ݵ�BTree_createʱ��BTree_create��ָ������ָ��tree�����գ�tree���൱��&tree
		//��ʱ*tree��ָ�����ݾ���Ȥ�ˣ�*tree��ͨ�����洢�ĵ�ַ�ҵ���ַ����Ӧ��ֵ������ָ��������tree��Ҳ����&tree���е�tree
		//����*tree�ı���ܸı�ԭ������ֵNULL��
BTNode* BTree_search(const BTree tree, int key, int* pos) {
	printf("BTree_search:\n");
	if (!tree) {
		printf("BTree is NULL!\n");
		return NULL;
	}
	*pos = -1;
	return BTree_recursive_search(tree, key, pos);

}
//keynum A0 K0 A1 K1....AN,KN,AN+1���ǽ������ 
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
	//�ݹ�Ĳ���������
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

	//���������������ǰ��Ҫ���з��ѵ���
	if (root->Keynum==ORDER-1)
	{
		//�����½ڵ㵱����
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
		//��node�ĺ� BTree_D - 1���ؼ���ת�Ƶ��½ڵ�
		newNode->Keynum = BTree_D - 1;
		for ( i = 0; i < newNode->Keynum; i++)
		{
			newNode->key[i] = node->key[BTree_D + i];
			node->key[BTree_D + i] = 0;
		}

		//ע��Ҷ�ӽ��ָ���ǿ���ֱ�Ӳ���Ľ��
		if(!node->isLeaf){
			for (i = 0; i < BTree_D; i++) {
				newNode->child[i] = node->child[BTree_D + i];
				node->child[BTree_D + i] = NULL;
			}
		}
		node->Keynum = BTree_D-1;
		
		//parentָ��
		for (i = parent->Keynum;i > index;--i)
			parent->child[i + 1] = parent->child[i];
		
		parent->child[index + 1] = newNode;
		
		//parent�ؼ���
		for ( i = parent->Keynum-1; i>=index ; --i)
		{
			parent->key[i + 1] = parent->key[i];
		}
		parent->key[index] = node->key[BTree_D-1];
		
		++parent->Keynum;
		
		//д�����
		disk_write(parent);		//parent->key[index] = node->key[BTree_D-1];
		disk_write(newNode);    //��BTree_D-1
		disk_write(node);		//ǰBTree_D-1
}

void BTree_insert_nonfull(BTNode* node, KeyType key) {
	int i;
	//�����Ҷ�ӽ��
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
		// �Ӵ��̶�ȡ���ӽڵ�
		disk_read(&node->child[i]);

		if (node->child[i]->Keynum == (ORDER - 1)) {
			BTree_split_child(node, i, node->child[i]);
			//���������Ĺؼ��ִ��ڸ÷��ѽ�������Ƶ����ڵ�Ĺؼ��֣��ڸùؼ��ֵ��Һ��ӽ���н��в��������
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
	// B-���ı�������֮һ��
	// �Ǹ��ڵ���ڲ��ڵ�Ĺؼ�����Ŀ�������� BTree_D - 1
	// ������������BTree_D��
	int i, j, index;
	BTNode* root = *tree;
	BTNode* node = root;

	if (!root) {
		printf("Failed to remove %c, it is not in the tree!\n", key);
		return;
	}

	index = 0;
	//���ѭ����������е�Ԫ�أ��������key=node->key[index]���ҵ���
	//key<node->key[index]���ֵ��node->child[index]��ָ�Ľ���С�
	while (index<node->Keynum&&key>node->key[index])
	{
		++index;
	}
	BTNode *leftChild, *rightChild;
	KeyType leftKey, rightKey;
	//index<node->Keynum�Ǳ���ģ���Ϊ�п���index=node->Keynum
	//��ʱ��node->key[index]�����ڡ���ʱ���ȥ���һ�������ҡ�
	if (index < node->Keynum && node->key[index] == key) {
		/*
		���ڽڵ���Ҷ�ӽڵ㣬ֱ��ɾ��
		����Ľ��һ���Ǹ��ġ�����ָ�ؼ��ָ�������BTree_D-1
		�����л��Ʊ�֤��
		*/
		if (node->isLeaf) {
			for (i = index; i < node->Keynum - 1; ++i) {
				node->key[i] = node->key[i + 1];
				//�����ؼ��ֶ��ˣ��ùؼ��ֶ�Ӧ��child��һ��Ҫ��ģ���������Ҷ�ӽ���ʡ�ˡ�
				//�������ǲ���node->child[i] = node->child[i+1];ȥ�ı�child,�ᵼ�����һ��û�и�������
				//node->child[i + 1] = node->child[i + 2];����Ҫ���������
				//�������ﶼ��NULL,���ÿ��ǡ�
			}
			node->key[node->Keynum - 1] = 0;
			node->Keynum--;

			//����ֻ���Ǹ����ΪҶ�ӽ�㣬��ֻ��һ���ؼ�����
			//Ŀǰ������������
			if (node->Keynum == 0)
			{
				free(node);
				*tree = NULL;
			}
			return;
		}

		//�����������Ƚϸ���������������ֵȥ����Ҫɾ����ֵ��
		//�����ɾ�������á�
		//��Ȼ��Ҫɾ����������ֵ�����Ծ��ǵݹ�ɾ����
		else if (node->child[index]->Keynum >= BTree_D)
		{
			leftChild = node->child[index];
			leftKey = leftChild->key[leftChild->Keynum - 1];
			node->key[index] = leftKey;
			BTree_recursive_remove(&leftChild, leftKey);
		}
		//ͬ��
		else if (node->child[index + 1]->Keynum >= BTree_D) {
			rightChild = node->child[index + 1];
			rightKey = rightChild->key[0];
			node->key[index] = rightKey;
			BTree_recursive_remove(&rightChild, rightKey);
		}

		//���Ƕ�����ƶ��ɾ��ǰ��Ҫ���ӽ��ĺϲ�����
		else if (node->child[index]->Keynum == BTree_D - 1
			&& node->child[index + 1]->Keynum == BTree_D - 1) {
			leftChild = node->child[index];
			BTree_merge_child(tree, node, index);
			//node��BTNode���͵�ָ�룬ת��ȥ��node-> ��ͨ����ַ�������Զ�ԭ������Ӱ�졣
			BTree_recursive_remove(&leftChild, key);

		}
	}
	//�������ڸý����û�ҵ��������
	else {
		BTNode *leftSibling, *rightSibling, *child;
		//keyӦ����node->child[index]����
		child = node->child[index];
		if (!child) {
			printf("Failed to remove %c, it is not in the tree!\n", key);
			return;
		}
		/*Ҫ���ҵĽ�����ƶ�����*/
		if (child->Keynum == BTree_D - 1) {
			leftSibling = NULL;
			rightSibling = NULL;

			if (index - 1 >= 0)
				leftSibling = node->child[index-1];
			if (index + 1 <= node->Keynum)
				rightSibling = node->child[index+1];
			//���ӻ����Һ�����һ���������Ǹ���
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

					for (j = 0; j < rightSibling->Keynum - 1; ++j) {//Ԫ��ǰ��
						rightSibling->key[j] = rightSibling->key[j + 1];
						rightSibling->child[j] = rightSibling->child[j + 1];
					}
					rightSibling->key[rightSibling->Keynum - 1] = 0;
					rightSibling->child[rightSibling->Keynum - 1] = rightSibling->child[rightSibling->Keynum];
					rightSibling->child[rightSibling->Keynum] = NULL;
					--rightSibling->Keynum;
				}
				else {//�������ֵ���Ը��У���ú����򸸽ڵ��һ��Ԫ�أ����ֵ��е����Ԫ�����������ڵ�����λ�ã���������Ӧ������
					for (j = child->Keynum; j > 0; --j) {
						child->key[j] = child->key[j - 1];
						child->child[j + 1] = child->child[j];
					}//����
					child->child[1] = child->child[0];//�����������
					child->child[0] = leftSibling->child[leftSibling->Keynum];
					child->key[0] = node->key[index - 1];
					++child->Keynum;
					//����
					node->key[index - 1] = leftSibling->key[leftSibling->Keynum - 1];
					//����
					leftSibling->key[leftSibling->Keynum - 1] = 0;
					leftSibling->child[leftSibling->Keynum] = NULL;

					--leftSibling->Keynum;
				}
			}
			else if ((!leftSibling || (leftSibling && leftSibling->Keynum == BTree_D - 1))
				&& (!rightSibling || (rightSibling && rightSibling->Keynum == BTree_D - 1))) {
				if (leftSibling && leftSibling->Keynum == BTree_D - 1) {

					BTree_merge_child(tree, node, index - 1);//node�е��Һ���Ԫ�غϲ��������С�
					//���Լ��ϲ������
					child = leftSibling;
				}

				else if (rightSibling && rightSibling->Keynum == BTree_D - 1) {
					//���ұߺϲ����Լ�
					BTree_merge_child(tree, node, index);//node�е��Һ���Ԫ�غϲ��������С�
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
	
	//��node����key[index]���Ƶ����ӣ������Һ��ӵĵ�һ�����Ӹ��Ƹ����ӵ����һ�����ӡ�
	leftChild->key[leftChild->Keynum] = key;
	leftChild->child[leftChild->Keynum + 1] = rightChild->child[0];
	++leftChild->Keynum;
	
	//�к��ӵ�Ԫ�غϲ������ӵ��С�
	for (i = 0; i < rightChild->Keynum; ++i) {
		leftChild->key[leftChild->Keynum] = rightChild->key[i];
		leftChild->child[leftChild->Keynum + 1] = rightChild->child[i + 1];
		++leftChild->Keynum;
	}

	//��Ȼkey[index]���ƣ���Ҫ��index�����Ԫ��ǰ��
	for (i = index;i < node->Keynum - 1;i++) {
		node->key[i] = node->key[i+1];
		node->child[i + 1] = node->child[i+2];
		//���ӻ��ڣ��Һ��ӱ�����
	}
	node->child[node->Keynum] = NULL;
	node->Keynum--;
	node->key[node->Keynum - 1] = 0;
	
	//node->Keynum == 0ֻ�и�������������
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
	//��ӡ������е�ȫ��Ԫ�أ�������Բ鿴keynum֮���Ԫ���Ƿ�Ϊ0(���Ƿ������������)��������keynum��Ԫ�ء�
	printf("�����д��ڵ�");
	for (int i = 0;i<ORDER - 1;i++) {
		printf("%c", node->key[i]);
	}
	printf("\n");
}

void disk_read(BTNode** node)
{
	//��ӡ������е�ȫ��Ԫ�أ�������Բ鿴keynum֮���Ԫ���Ƿ�Ϊ0(���Ƿ������������)��������keynum��Ԫ�ء�
	printf("����̶�ȡ�ڵ�");
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
		printf("�� %d �㣬 %d node : ", layer, node->Keynum);

		//��ӡ������е�ȫ��Ԫ�أ�������Բ鿴keynum֮���Ԫ���Ƿ�Ϊ0(���Ƿ������������)��������keynum��Ԫ�ء�
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
		printf("��Ϊ�ա�\n");
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
			return mid;//�����±ꡣ
		}
	}
	return 0;//δ�ҵ�����0.
}
