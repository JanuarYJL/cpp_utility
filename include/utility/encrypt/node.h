#ifndef PZ_COMPRESS_TEST
#define PZ_COMPRESS_TEST

#define SET_0 0x01   //  0000 0001
#define SET_1 0x02   //  0000 0010
#define SET_2 0x04   //  0000 0100
#define SET_3 0x08   //  0000 1000
#define SET_4 0x10   //  0001 0000
#define SET_5 0x20   //  0010 0000
#define SET_6 0x40   //  0100 0000
#define SET_7 0x80   //  1000 0000
#define RESET_0 0xFE //  1111 1110
#define RESET_1 0xFD //  1111 1101
#define RESET_2 0xFB //  1111 1011
#define RESET_3 0xF7 //  1111 0111
#define RESET_4 0xEF //  1110 1111
#define RESET_5 0xDF //  1101 1111
#define RESET_6 0xBF //  1011 1111
#define RESET_7 0x7F //  0111 1111
#define PZ_MAX 256

#define SET_BYTE_2

typedef unsigned int PZ_DWORD;
typedef unsigned short PZ_WORD;
typedef unsigned char PZ_BYTE;

#ifdef SET_BYTE_4
typedef PZ_DWORD MY_BYTE;
#define PZ_INCREASE 4
#endif

#ifdef SET_BYTE_2
typedef PZ_WORD MY_BYTE;
#define PZ_INCREASE 2
#endif

#ifdef SET_BYTE_1
typedef PZ_BYTE MY_BYTE;
#define PZ_INCREASE 1
#endif

// 备注：简单介绍霍夫曼编码压缩算法。
// 1、初始化，根据符号概率的大小按由大到小顺序(该文件用的是从小到大)对符号进行排序。如下表：
//	   符号		出现频率
//		A			0.3864
//		B			0.1795
//		C			0.1538
//		D			0.1538
//		E			0.1282
// 2、把概率最小的两个符号组成一个节点,如D和E组成节点P1。
// 3、重复步骤2，得到节点P2、P3和P4，形成一棵“树”，其中的P4称为根节点。
// 4、从根节点P4开始到相应于每个符号的“树叶”，从上到下标上“0”(上枝)或者“1”(下枝)，至于哪个为“1”哪个为“0”则无关紧要，
//	  最后的结果仅仅是分配的代码不同，而代码的平均长度是相同的。
// 5、从根节点P4开始顺着树枝到每个叶子分别写出每个符号的代码：A(0)、B(100)、C(101)、D(110)、E(111)。
//							0
//		A(0.3864)	----------------|-----
//					  0				|P4
//		B(0.1795)	-----|	0		|
//					  1	 |-----|	|
//		C(0.1538)	-----|	P2 |  1	|
//					  0		   |----|
//		D(0.1538)	-----|	1  |  P3
//					  1  |-----|
//		E(0.1282)	-----|	p1
// 解压缩时将输入缓冲区中的每个编码用对应的ASCII码逐个替换就可以了。只要记住，这里的输入缓冲区是一个包含每个ASCII值的编码的
// 位流。因此，为了用ASCII值替换编码，我们必须用位流搜索霍夫曼树，直到发现一个叶节点，然后将它的ASCII值添加到输出缓冲区中。

class PZ_Node;
class PZ_List;
class PZ_StoreCode;
class PZ_HandleCompress;
class PZ_Tree;
//-----------------------这里为测试增加的------------------------------
//#include<iostream>
//using namespace std;
//---------------------------------------------------------------------
// 类名：PZ_Node
// 功能：作为链表和树的节点，用来存储在一块内存区域中每个字节出现的次数。
// 用法：被PZ_List和PZ_Tree调用，构造链表和树
class PZ_Node
{
public:
	int frequency;			 // 次数
	unsigned char character; // 指明被统计的字节是character
	PZ_Node *pnext;			 // 用在建立List
	PZ_Node *plchild;		 // 用在建立树
	PZ_Node *prchild;		 // 用在建立树

	// 设置叶子节点为树的左叶子节点，存储编码设为0
	// @param1 [in][out]	table	  把叶子节点的信息存储到该内存中
	void LSet(PZ_StoreCode *table);

	// 设置叶子节点为树的右叶子节点，存储编码设为1
	// @param1 [in][out]	table	  把叶子节点的信息存储到该内存中
	void RSet(PZ_StoreCode *table);

	// 取得子节点
	// @return 取得子节点是否成功
	// @param1 [in][out]	phead	  叶子节点指针的指针
	// @param2 [in]			bit		  取左节点或右节点，false取左,true取右
	static bool Movetonext(PZ_Node **phead, bool bit);

	~PZ_Node();
};

// 类名：PZ_List
// 功能：构造链表中节点，以PZ_Node中frequency按升序排列成链表
// 用法：以霍夫曼编码压缩解压缩数据时，先构造链表，相当于备注中第1步。
class PZ_List
{
public:
	PZ_Node head; // 头结点

	// 在链表中加入一个新节点，按升序排列
	// @param1 [in]			pnode		  节点指针
	void AddNode(PZ_Node *pnode);

	// 将链表中第一个frequency大于0的结点从list断开，并返回该结点的指针(不释放该结点，该结点用来建立树的)
	// @return 找到节点的指针
	PZ_Node *DelFirst();

	// 用table中统计的frequency来建立List表
	// @return 建立链表是否成功，失败返回-1
	// @param1 [in]			table		  保存字符频率的指针
	int InitiaList(PZ_StoreCode *table);

	// 除开头结点，按顺序将所有结点的character,frequency存放在一个buf中，buf前四个字节存放List长度
	// @return 保存链表是否成功，失败返回-1。
	// @param1 [in][out]	buf			  保存链表信息的指针
	// @param2 [in]			len			  保存链表信息内存的长度
	int SaveList(unsigned char *buf, int len);

	// 从一块buf中提取结点，建立List(该buf前4个字节存放结点个数)
	// @return 建立链表是否成功，失败返回-1。
	// @param1 [in]			buf			  保存节点信息的地址
	// @param2 [out]		count		  链表长度
	int LoadList(const unsigned char *buf, int *count);

	~PZ_List();
};

// 类名：PZ_Tree
// 功能：由List初始化形成的最优二叉树(以Node.frequency为权)
// 用法：相当于备注中第2、3、4步。
class PZ_Tree
{
public:
	PZ_Node *root; //  指向根结点

	// Tree根据list来构造，将每个叶子结点对应的霍夫曼编码返填到table[(unsigned char)character]中，
	// 使用InitiaTree之前,确保Tree是颗空树。
	// @return 构造树是否成功，失败返回-1。
	// @param1 [in]			list1		  保存节点信息的链表
	// @param2 [in]			table		  保存节点信息的节点表
	int InitiaTree(PZ_List &list1, PZ_StoreCode *table);

	// 构造最优秀树，使用InitiaTree之前,确保Tree是颗空树。
	// @return 构造树是否成功，失败返回-1。
	// @param1 [in]			list1		  保存节点信息的链表
	int InitiaTree(PZ_List &list1);

	~PZ_Tree();
};

// 类名：PZ_StoreCode
// 功能：以StoreCode table[MAX]方式使用，统计字节出现的次数，用来初试化list，
//		 当list初始化完成并在构造树的过程中，将字节的霍夫曼编码反填到bit[]中。
// 用法：相当于备注中第1步前的准备工作。
class PZ_StoreCode
{
public:
	int len;		  //  霍夫曼编码长度
	bool bit[PZ_MAX]; //  存储编码
	int frequency;	//  统计次数
};

// 类名：PZ_HandleCompress
// 功能：处理数据的压缩、解压缩。
// 用法：直接调用类中的压缩、解压缩函数即可。
class PZ_HandleCompress
{
public:
	// 对数据进行压缩
	// @return 压缩后数据的长度
	// @param1 [out]		dest		  保存压缩后数据的指针
	// @param2 [in]			destLen		  保存压缩后数据地址的长度
	// @param3 [in]			source		  要压缩的数据
	// @param4 [in]			sourceLen	  要压缩数据的长度
	int Compress(
		unsigned char *dest,
		int destLen,
		const unsigned char *source,
		int sourceLen);

	// 对数据进行解压缩
	// @return 解压后数据的长度
	// @param1 [out]		dest		  保存解压后数据的指针
	// @param2 [in]			destLen		  保存解压后数据地址的长度
	// @param3 [in]			source		  要解压的数据
	// @param4 [in]			sourceLen	  要解压数据的长度
	int UnCompress(
		unsigned char *dest,
		int destLen,
		const unsigned char *source,
		int sourceLen);

private:
	// 计算要压缩数据中字符出现的次数，保存到table中
	// @return 计算字符出现次数是否成功，失败返回-1。
	// @param1 [in]			source		  要压缩数据的指针
	// @param2 [in]			sourceLen	  要压缩数据的长度
	// @param3 [out]		table		  保存字符出现次数的指针
	int Initia(
		const unsigned char *source,
		int sourceLen,
		PZ_StoreCode *table);

	// 对数据进行压缩，即把table中的霍夫曼编码输出
	// @return 压缩后数据长度
	// @param1 [out]		dest		  保存压缩后数据的指针
	// @param2 [in]			destLen		  保存压缩后数据的长度
	// @param3 [in]			source		  要压缩数据的指针
	// @param4 [in]			sourceLen	  要压缩数据的长度
	// @param5 [in]			table		  保存树节点霍夫曼编码的指针
	int deflate(
		unsigned char *dest,
		int destLen,
		const unsigned char *source,
		int sourceLen,
		PZ_StoreCode *table);

	// 对数据进行解压缩，即把霍夫曼编码转换成对应的字符
	// @return 解压后数据的长度
	// @param1 [out]		dest		  保存解压后数据的指针
	// @param2 [in]			destLen		  保存解压后数据的长度
	// @param3 [in]			source		  要解压数据的指针
	// @param4 [in]			sourceLen	  要解压数据的长度
	// @param5 [in]			tree_node	  构造的霍夫曼树
	int inflate(
		unsigned char *dest,
		int destLen,
		const unsigned char *source,
		int sourceLen,
		PZ_Tree &tree_node);

public:
	// 设置一个字节的当前位为1
	// @return 设置是否成功
	// @param1 [in][out]	pch			  要设置的字节
	// @param2 [in]			i			  要设置的位数
	static bool Set(unsigned char *pch, int i);

	// 设置一个字节的当前位为0
	// @return 设置是否成功
	// @param1 [in][out]	pch			  要设置的字节
	// @param2 [in]			i			  要设置的位数
	static bool ReSet(unsigned char *pch, int i);

	// 取得一个字节的某个位值
	// @return 取得一个字节某位值是否成功
	// @param1 [in][out]	pch			  要取得的字节
	// @param2 [in]			i			  要取得的位数
	// @param3 [out]		result		  取得的第i位的值，如果为1，返回true,为0，返回false
	static bool GetBit(const unsigned char *pch, int i, bool *result);
};

#endif