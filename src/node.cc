#include <cstring>
#include "encrypt/node.h"

PZ_Node::~PZ_Node()
{
	PZ_Node *pl = plchild;
	PZ_Node *pr = prchild;
	if (pl != 0)
		delete pl;
	if (pr != 0)
		delete pr;
}

// 设置左叶子节点，并将对应的table[index].bit[len]设置0,len++
void PZ_Node::LSet(PZ_StoreCode *table)
{
	if (plchild == 0 && prchild == 0) // 左孩子,右孩子为空，所以是叶子结点，直接设置该叶子结点
	{
		table[((unsigned char)character)].bit[table[((unsigned char)character)].len] = 0;
		table[((unsigned char)character)].len++;
	}
	else // 不是叶子结点，递归调用，设置左子树和右子树的叶子结点
	{
		if (plchild != 0) // 左子树存在的情况下，递归
			plchild->LSet(table);
		if (prchild != 0) // 右子树存在的情况下，递归
			prchild->LSet(table);
	}
}

// 设置右叶子节点，并将对应的table[index].bit[len]设置1,len++
void PZ_Node::RSet(PZ_StoreCode *table)
{
	if (plchild == 0 && prchild == 0) // 左孩子,右孩子为空，所以是叶子结点，直接设置该叶子结点
	{
		table[((unsigned char)character)].bit[table[((unsigned char)character)].len] = 1;
		table[((unsigned char)character)].len++;
	}
	else // 不是叶子结点，递归调用，设置左子树和右子树的叶子结点
	{
		if (plchild != 0) // 左子树存在的情况下，递归
			plchild->RSet(table);
		if (prchild != 0) // 右子树存在的情况下，递归
			prchild->RSet(table);
	}
}

// 取得左子节点或右子节点，由bit决定
bool PZ_Node::Movetonext(PZ_Node **phead, bool bit)
{
	if (phead == 0)
		return false;
	else
	{
		if (*phead == 0)
			return false;
		else
		{
			if (bit) // bit为true时，取右子节点
				*phead = (*phead)->prchild;
			else // bit为false时，取右子节点
				*phead = (*phead)->plchild;
			if (*phead == 0)
				return false;
			else
				return true;
		}
	}
}

PZ_List::~PZ_List()
{
	PZ_Node *p_iterator = head.pnext;
	while (p_iterator != 0)
	{
		PZ_Node *temp = p_iterator->pnext;
		delete p_iterator;
		p_iterator = temp;
	}
}

// 把新节点加入到链表中，升序排列
void PZ_List::AddNode(PZ_Node *pnode)
{
	PZ_Node *p_iterator = head.pnext; // 迭代器初始化，指向表的第一个结点(不存在结点时,为NULL)
	PZ_Node *prev = &head;			  // 插入结点的前一个结点
	while (p_iterator != 0)			  // List非空开始遍历,直到最后一个结点
	{
		if (p_iterator->frequency > pnode->frequency) // 找到第一个比pnode大的结点，退出循环
			break;
		prev = p_iterator;
		p_iterator = p_iterator->pnext;
	}
	pnode->pnext = prev->pnext; // 将结点插入到prev之后
	prev->pnext = pnode;
}

// 将链表中第一个frequency大于0的节点断开，用来建立树
PZ_Node *PZ_List::DelFirst()
{
	PZ_Node *p_iter = head.pnext; // 迭代器初始化
	PZ_Node *prev = &head;
	while (p_iter != 0) // List非空开始遍历，直到最后一个
	{
		if (p_iter->frequency > 0) // 找到第一个frequency大于0的结点，退出循环
			break;
		prev = p_iter;
		p_iter = p_iter->pnext;
	}
	if (p_iter != 0) // 如果存在满足条件(frequency>0)的结点，将该结点从list断开
		prev->pnext = p_iter->pnext;
	return p_iter; // 返回该节点指针
}

// 从table中构造节点，加入到链表。
int PZ_List::InitiaList(PZ_StoreCode *table)
{
	if (table == 0)
		return -1;
	PZ_Node *ptemp = 0;
	for (int i = 0; i < PZ_MAX; i++) // 遍历table
	{
		if (table[i].frequency > 0) // 提取frequency大于0的项
		{
			ptemp = new PZ_Node; // 新建结点
			ptemp->frequency = table[i].frequency;
			ptemp->character = i;
			ptemp->plchild = 0;
			ptemp->prchild = 0;
			AddNode(ptemp); // 将结点加到list中
		}
	}
	return 0;
}

// 保存链表中节点保存到buf中。
int PZ_List::SaveList(unsigned char *buf, int len)
{
	if (buf == 0 || len <= 0)
		return -1;
	int eof = 0;
	PZ_Node *iterator_list = head.pnext; // list迭代器初始化
	unsigned char *iterator_buf = buf;   // buf迭代器初始化
	eof += 4;
	if (len < eof + 1)
		return -1;
	iterator_buf += 4;		   // buf前4个字节预留,用来存放list长度,待返填
	int count = 0;			   // 统计list长度
	while (iterator_list != 0) // 非空表开始循环,直到最后一个结点
	{
		eof += (1 + PZ_INCREASE);
		if (len < eof + 1)
			return -1;
		*iterator_buf = iterator_list->character; // 将character放在buf中 1个字节
		iterator_buf++;
		*(MY_BYTE *)iterator_buf = (MY_BYTE)iterator_list->frequency; //将frequency放在buf中 4个字节
		iterator_buf += PZ_INCREASE;
		iterator_list = iterator_list->pnext;
		count++;
	}
	if (count > 0) // 如果表非空
	{
		*(int *)buf = count;			  // count返填到buf前4个字节
		*iterator_buf = 0;				  // 以空接结尾
		return (int)(iterator_buf - buf); // 返填len
	}
	else // 表为空的时候
	{
		return -1;
	}
}

// 从buf读取节点信息，构成链表
int PZ_List::LoadList(const unsigned char *buf, int *count)
{
	if (buf == 0 || count == 0)
		return -1;
	int len = *(int *)buf; // 取buf前4个字节
	if (len <= 0)		   // 如果buf不是预期的格式,这里只做长度检查
	{
		*count = 0;
		return -1;
	}
	const unsigned char *iterator_buf = buf;
	iterator_buf += 4;			  // 迭代器初始设定
	int frequency = 0;			  // 临时变量 存放frequency
	unsigned char character = 0;  // 临时变量 存放character
	PZ_Node *ptemp = 0;			  // 临时变量 存放新建的Node地址
	for (int i = 0; i < len; i++) // 循环 次数为len
	{
		character = *iterator_buf; // 取character 1字节
		iterator_buf++;
		frequency = *(MY_BYTE *)iterator_buf; // 取frequency 4字节
		iterator_buf += PZ_INCREASE;
		ptemp = new PZ_Node; // 新建结点,用character,frequency初始化后加到list中
		ptemp->character = character;
		ptemp->frequency = frequency;
		ptemp->pnext = 0;
		ptemp->plchild = 0;
		ptemp->prchild = 0;
		AddNode(ptemp);
	}
	*count = (int)(iterator_buf - buf); // 链表长度
	return 0;
}

PZ_Tree::~PZ_Tree()
{
	if (root != 0)
		delete root;
}

// 根据链表构造树，用于压缩数据时
int PZ_Tree::InitiaTree(PZ_List &list1, PZ_StoreCode *table)
{
	if (table == 0 || root != 0)
		return -1;
	PZ_Node *pnode1 = 0;
	PZ_Node *pnode2 = 0;
	pnode1 = list1.DelFirst();		// 从list断开并提取第一个frequency>0结点
	pnode2 = list1.DelFirst();		// 从list断开并提取第一个frequency>0结点
	if (pnode1 != 0 && pnode2 == 0) // 第一次提取成功，第二次提取失败，
	{								// 说明list中只有1个frequency>0的结点,特殊处理
		root = new PZ_Node;
		root->plchild = pnode1;
		root->prchild = 0;
		table[(unsigned char)(root->plchild->character)].len = 1;
		table[(unsigned char)(root->plchild->character)].bit[0] = 0;
	}
	else // 两次都成功提取 或者 都失败
	{
		while (pnode1 != 0 && pnode2 != 0) // 提取成功时进去循环，直到不能两次提取成功为止
		{
			root = new PZ_Node; // 构造树，树的左右结点为pnode1,pnode2
			root->plchild = pnode1;
			pnode1->LSet(table);
			root->prchild = pnode2;
			pnode2->RSet(table);
			root->frequency = pnode1->frequency + pnode2->frequency;
			list1.AddNode(root); // 新结点加到list中
			pnode1 = list1.DelFirst();
			pnode2 = list1.DelFirst();
		} // 循环终止的时候，树构造完成，所有frequency>0结点从list断开
	}
	if (root == 0)
		return -1;
	return 0;
}

// 根据链表构造树，用于解压数据时
int PZ_Tree::InitiaTree(PZ_List &list1)
{
	if (root != 0)
		return -1;
	PZ_Node *pnode1 = 0;
	PZ_Node *pnode2 = 0;
	pnode1 = list1.DelFirst();		// 从list断开并提取第一个frequency>0结点
	pnode2 = list1.DelFirst();		// 从list断开并提取第一个frequency>0结点
	if (pnode1 != 0 && pnode2 == 0) // 第一次提取成功，第二次提取失败，说明list中只有1个frequency>0的结点,特殊处理
	{
		root = new PZ_Node;
		root->plchild = pnode1;
		root->prchild = 0;
	}
	else // 两次都成功提取 或者 都失败
	{
		while (pnode1 != 0 && pnode2 != 0) // 提取成功时进去循环，直到不能两次提取成功为止
		{
			root = new PZ_Node; // 构造树，树的左右结点为pnode1,pnode2
			root->plchild = pnode1;
			root->prchild = pnode2;
			root->frequency = pnode1->frequency + pnode2->frequency;
			list1.AddNode(root); // 新结点加到list中
			pnode1 = list1.DelFirst();
			pnode2 = list1.DelFirst();
		} // 循环终止的时候，树构造完成，所有frequency>0结点从list断开
	}
	if (root == 0)
		return -1;
	return 0;
}

// 从source压缩sourceLen个字节到dest,压缩过程分为以下几步：
// 1、计算字符出现的频率；
// 2、按照字符出现频率升序，建立链表；
// 3、将链表中节点的character、frequency保存到dest中；
// 4、将链表中节点组成树；
// 5、向dest中输出霍夫曼编码。
int PZ_HandleCompress::Compress(unsigned char *dest, int destLen, const unsigned char *source, int sourceLen)
{
	if (dest == 0 || destLen <= 0 || source == 0 || sourceLen <= 0) // 参数检查
		return -1;
	PZ_StoreCode table[PZ_MAX];
	memset(table, 0, PZ_MAX * sizeof(PZ_StoreCode));
	if (-1 == Initia(source, sourceLen, table)) // 初始化table[MAX]的frequency
		return -1;
	PZ_List list_node;
	memset(&list_node, 0, sizeof(PZ_List));
	if (-1 == list_node.InitiaList(table)) // 用table[MAX]来初始化list_node
		return -1;
	int list_byte_len = 0;										   // list_node在dest中占用的字节
	if (-1 == (list_byte_len = list_node.SaveList(dest, destLen))) // 将list_node的必要信息存放到dest中
		return -1;
	PZ_Tree tree_node;
	memset(&tree_node, 0, sizeof(PZ_Tree));
	if (-1 == tree_node.InitiaTree(list_node, table)) // 用list_node构造树 该过程将霍夫曼编码返填到table中
		return -1;
	int deflate_len = 0;																						// deflate过程的返回值
	if (-1 == (deflate_len = deflate(dest + list_byte_len, destLen - list_byte_len, source, sourceLen, table))) // 压缩，传递已被初始化的table参数
		return -1;
	return (list_byte_len + deflate_len); // 返填长度到destLen
}

// 从source解压sourceLen个字节到dest,解压过程分为以下几步：
// 1、从source得到节点信息，组成链表；
// 2、把链表中节点组成树；
// 3、逐渐从source中得到霍夫曼编码，搜索树，查找叶子节点，得到字符。
int PZ_HandleCompress::UnCompress(unsigned char *dest, int destLen, const unsigned char *source, int sourceLen)
{
	if (dest == 0 || destLen <= 0 || source == 0 || sourceLen <= 0) // 参数检查
		return -1;
	PZ_List list_node;
	memset(&list_node, 0, sizeof(PZ_List));
	int list_byte_len = 0;
	if (list_node.LoadList(source, &list_byte_len) == -1) // 初始化链表
		return -1;
	PZ_Tree tree_node;
	memset(&tree_node, 0, sizeof(PZ_Tree));
	if (tree_node.InitiaTree(list_node) == -1) // 构造树
		return -1;
	int reallen = 0;
	if ((reallen = inflate(dest, destLen, source + list_byte_len, sourceLen - list_byte_len, tree_node)) == -1) // 解压数据
		return -1;
	return reallen;
}

// 将source中各字节出现的次数，存储在table中
int PZ_HandleCompress::Initia(const unsigned char *source, int sourceLen, PZ_StoreCode *table)
{

	if (source == 0 || sourceLen < 1 || table == 0) // 检查输入是否合法
		return -1;
	else
	{
		const unsigned char *iterator = source; // source迭代器
		int index = 0;							// 每个字节对应table的下标
		for (int i = 0; i < sourceLen; i++)		// 循环初始化table各项
		{
			iterator = source + i;
			index = *((unsigned char *)iterator);
			table[index].frequency++; // 每个字符出现的次数
		}
		return 0;
	}
}

// 将source压缩到dest中，参数table是一个已经被初试化,霍夫曼编码已被返填
int PZ_HandleCompress::deflate(unsigned char *dest, int destLen, const unsigned char *source, int sourceLen, PZ_StoreCode *table)
{
	if (source == 0 || dest == 0 || destLen <= 0 || sourceLen <= 0) // 参数检查
		return -1;
	else
	{
		int eof = 0;
		const unsigned char *iterator_in = source; // source 的迭代器
		unsigned char *iterator_out = dest;		   // dest 的迭代器
		eof += 4;
		if (destLen < eof + 1)
			return -1;
		iterator_out += 4;					// 预留4个字节,记录位数
		int index_out = 0;					// dest每个字节当前位的下标 从0到7循环进1
		int count_out = 0;					// 记录位数(不是字节数)
		for (int i = 0; i < sourceLen; i++) // 循环遍历source
		{
			iterator_in = source + i;
			int index = *(unsigned char *)iterator_in; // 找到source对应的table
			int count = table[index].len;			   // 取出霍夫曼编码的长度
			for (int j = count - 1; j > -1; j--)	   // 将编码取出,设置到dest的相应位置中
			{
				if (table[index].bit[j]) // 为1的时候
				{
					Set(iterator_out, index_out); // 设置当前位为1
					index_out++;				  // 取下一位
					count_out++;				  // 位数+1
					if (index_out == 8)			  // 如果当前位是从7加到8的,取下一个字节,当前位下标置0
					{
						index_out = 0;
						eof += 1;
						if (destLen < eof + 1)
							return -1;
						iterator_out++;
					}
				}
				else // 为0的时候
				{
					ReSet(iterator_out, index_out); // 设置当前位为0
					index_out++;					// 取下一位
					count_out++;					// 位数+1
					if (index_out == 8)				// 如果当前位是从7加到8的,取下一个字节,当前位下标置0
					{
						index_out = 0;
						if (destLen < eof + 1)
							return -1;
						iterator_out++;
					}
				}
			}					  //end for
		}						  //end for
		*(int *)dest = count_out; // 位数返填到dest的前4个字节
		if (index_out != 0)		  // 如果index非0说明iterator_out所指示的字节被填写过,所以iterator_out++
			iterator_out++;
		*iterator_out = 0;				   // 以空结尾
		return (int)(iterator_out - dest); // 返填长度到destLen
	}									   //end if...else
}

// 将source解压到dest中，参数tree_node是已经构造好的霍夫曼树
int PZ_HandleCompress::inflate(unsigned char *dest, int destLen, const unsigned char *source, int sourceLen, PZ_Tree &tree_node)
{
	if (source == 0 || dest == 0 || destLen <= 0 || sourceLen <= 0) // 检查参数
		return -1;
	else
	{
		int eof = 0;
		const unsigned char *iterator_in = source; // source 迭代器
		unsigned char *iterator_out = dest;		   // dest 迭代器
		int size = *(int *)iterator_in;			   // 从 source 去前4个字节(2进制编码长度)
		if (size <= 0)
			return -1;
		iterator_in += 4;
		int index_in = 0;					 // dest当前字节的当前位下标(0到7循环进1)
		int count_out = 0;					 // 统计写到dest的字节数
		bool bit = false;					 // 从source取出来的位的值用bit表示1为true,0为false
		PZ_Node *tree_iter = tree_node.root; // 在树中,指向当前结点的指针
		for (int i = 0; i < size; i++)		 // 循环遍历source
		{
			GetBit(iterator_in, index_in, &bit); // 将当前字节的当前位取出放在bit里
			index_in++;							 // index循环进1
			if (index_in == 8)					 // 从7到8时,应该循环进1到0,并取下一个字节
			{
				index_in = 0;
				iterator_in++;
			}
			{
				if (PZ_Node::Movetonext(&tree_iter, bit)) // tree_iter依据bit值取子结点,false取左,true取右
				{
					if (tree_iter->plchild == 0 && tree_iter->prchild == 0) // 取到的结点是叶子结点,解码,并将tree_iter重置为tree_node.root
					{
						eof += 1;
						if (destLen < eof + 1)
							return -1;
						*iterator_out = tree_iter->character;
						iterator_out++;
						count_out++;
						tree_iter = tree_node.root;
					}
				}
				else // tree_iter Move 失败的情况
					return -1;
			}
		} //end for
		//*destLen=count_out;									// 将写到dest字节个数返填到destLen中
		*iterator_out = 0; // 空结尾
		return count_out;
	} //end if...else
}

// 将*pch第i位置1
bool PZ_HandleCompress::Set(unsigned char *pch, int i)
{
	unsigned char *byte = (unsigned char *)pch;
	if (byte == 0 || i < 0 || i > 7)
		return false;
	else
	{
		switch (i)
		{
		case 0:
			*byte = *byte | SET_0;
			break;
		case 1:
			*byte = *byte | SET_1;
			break;
		case 2:
			*byte = *byte | SET_2;
			break;
		case 3:
			*byte = *byte | SET_3;
			break;
		case 4:
			*byte = *byte | SET_4;
			break;
		case 5:
			*byte = *byte | SET_5;
			break;
		case 6:
			*byte = *byte | SET_6;
			break;
		case 7:
			*byte = *byte | SET_7;
			break;
		}
		return true;
	}
}

// 将*pch第i位置0
bool PZ_HandleCompress::ReSet(unsigned char *pch, int i)
{
	unsigned char *byte = (unsigned char *)pch;
	if (byte == 0 || i < 0 || i > 7)
		return false;
	else
	{
		switch (i)
		{
		case 0:
			*byte = *byte & RESET_0;
			break;
		case 1:
			*byte = *byte & RESET_1;
			break;
		case 2:
			*byte = *byte & RESET_2;
			break;
		case 3:
			*byte = *byte & RESET_3;
			break;
		case 4:
			*byte = *byte & RESET_4;
			break;
		case 5:
			*byte = *byte & RESET_5;
			break;
		case 6:
			*byte = *byte & RESET_6;
			break;
		case 7:
			*byte = *byte & RESET_7;
			break;
		}
		return true;
	}
}

// 将*pch第i位值取出，结果放在result中
bool PZ_HandleCompress::GetBit(const unsigned char *pch, int i, bool *result)
{
	const unsigned char *byte = (const unsigned char *)pch;
	if (byte == 0 || i < 0 || i > 7 || result == 0)
		return false;
	else
	{
		unsigned char temp = 0;
		switch (i)
		{
		case 0:
			temp = *byte & SET_0;
			if (temp == SET_0)
				*result = true;
			else
				*result = false;
			break;
		case 1:
			temp = *byte & SET_1;
			if (temp == SET_1)
				*result = true;
			else
				*result = false;
			break;
		case 2:
			temp = *byte & SET_2;
			if (temp == SET_2)
				*result = true;
			else
				*result = false;
			break;
		case 3:
			temp = *byte & SET_3;
			if (temp == SET_3)
				*result = true;
			else
				*result = false;
			break;
		case 4:
			temp = *byte & SET_4;
			if (temp == SET_4)
				*result = true;
			else
				*result = false;
			break;
		case 5:
			temp = *byte & SET_5;
			if (temp == SET_5)
				*result = true;
			else
				*result = false;
			break;
		case 6:
			temp = *byte & SET_6;
			if (temp == SET_6)
				*result = true;
			else
				*result = false;
			break;
		case 7:
			temp = *byte & SET_7;
			if (temp == SET_7)
				*result = true;
			else
				*result = false;
			break;
		}
		return true;
	}
}