#include <stdbool.h>
#define BTree_D 2
#define ORDER    (BTree_D*2)
typedef int KeyType;
typedef struct BTNode {
	int Keynum;
	KeyType key[ORDER - 1];
	struct BTNode *child[ORDER];
	bool isLeaf;
}BTNode, *BTree;		//B-������B-������

typedef struct {
	BTree pt; //�ҵ��Ľ��
	int i;		//1.....ORDER-1���ڽ���йؼ������
	bool tag;   //�Ƿ��ҵ�
}Result;

void BTree_create(BTree *tree, const KeyType* data, int length);
void BTree_destroy(BTree*  tree);//����BTree���ͷ��ڴ�ռ�
void BTree_insert(BTree* tree, KeyType key);
void BTree_remove(BTree* tree, KeyType key);
void BTree_print(const BTree tree, int layer);//��ȱ���BTree��ӡ��������Ϣ
BTNode* BTree_search(const BTree tree, int key, int* pos);