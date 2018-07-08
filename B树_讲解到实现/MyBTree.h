#include <stdbool.h>
#define BTree_D 2
#define ORDER    (BTree_D*2)
typedef int KeyType;
typedef struct BTNode {
	int Keynum;
	KeyType key[ORDER - 1];
	struct BTNode *child[ORDER];
	bool isLeaf;
}BTNode, *BTree;		//B-树结点和B-的类型

typedef struct {
	BTree pt; //找到的结点
	int i;		//1.....ORDER-1，在结点中关键字序号
	bool tag;   //是否找到
}Result;

void BTree_create(BTree *tree, const KeyType* data, int length);
void BTree_destroy(BTree*  tree);//销毁BTree，释放内存空间
void BTree_insert(BTree* tree, KeyType key);
void BTree_remove(BTree* tree, KeyType key);
void BTree_print(const BTree tree, int layer);//深度遍历BTree打印各层结点信息
BTNode* BTree_search(const BTree tree, int key, int* pos);