#include <cstring>
#include "encrypt/lz77compress.h"

// 取a,b中的最小值
int Pz_lz77compress::Min1(int a, int b)
{
	if (b < a) // 当b<a时 将最小值a更新为b
		a = b;
	return a;
}

// a>0时取b,a<=0时取c
int Pz_lz77compress::cquestion(int a, int b, int c)
{
	if (a > 0)
		return b;
	else
		return c;
}
//--------------------------------------------------------------------------------------------
//  说明：
//  此函数由 CopyBits 调用，
//  CopyBits保证nDestPos+nBits<=8,nSrcPos+nBits<=8
//  因此不做错误检查，即
//  假定要复制的位都在一个字节范围内
//--------------------------------------------------------------------------------------------
void Pz_lz77compress::CopyBitsInAByte(
	unsigned char *memDest,
	int nDestPos,
	unsigned char *memSrc,
	int nSrcPos,
	int nBits)
{
	unsigned char b1, b2;
	b1 = *memSrc;					   // b1接受memSrc位置的值 将其做相应的位操作
	b1 = b1 << nSrcPos;				   // 将该memSrc前nSrcPos移除后的值 如:？？？_ _ _ _ ？ 变为_ _ _ _ ？000
	b1 = b1 >> (8 - nBits);			   // _ _ _ _ ? 000 变为 0000 _ _ _ _
	b1 = b1 << (8 - nBits - nDestPos); // 0000 _ _ _ _ 变为 (nDestPos)00 _ _ _ _ 00
	*memDest = *memDest | b1;		   // 将nBits中值为1的位拷贝到memDest

	b2 = 0xff;
	b2 = b2 << (8 - nDestPos); // 11000000
	b1 = b1 | b2;			   // b1变为11 _ _ _ _ 00
	b2 = 0xff;
	b2 = b2 >> (nDestPos + nBits); // 00000011
	b1 = b1 | b2;				   // b1变为11 _ _ _ _ 11
	*memDest = *memDest & b1;	  // 将nBits中值为0的位拷贝到memDest
}
//--------------------------------------------------------------------------------------------
//  说明：
//  起始位的表示约定为从字节的高位至低位（由左至右）
//  依次为 0，1，... , 7
//  要复制的两块数据区不能有重合
//--------------------------------------------------------------------------------------------
void Pz_lz77compress::CopyBits(
	unsigned char *memDest,
	int nDestPos,
	unsigned char *memSrc,
	int nSrcPos,
	int nBits)
{
	int iByteDest, iBitDest;	   // 分别是memDest当前字节，当前位的标识，用于迭代所有的位
	int iByteSrc, iBitSrc;		   // 分别是memSrc当前字节，当前位的标识，用于迭代所有的位
	int nBitsToFill, nBitsCanFill; // 分别是1次处理中需要填的位数，在一次字节内复制能够复制的位数
	iByteDest = 0;				   // 初始化
	iByteSrc = 0;
	iBitSrc = nSrcPos;
	while (nBits > 0) // nBits为剩余多少未复制的位 所有位处理完后退出，处理过程是依次处理memDest的0...n位字节
	{
		iBitDest = cquestion(iByteDest, 0, nDestPos);					  // 第0字节时当前位是nDestPos,其他情况下当前位为0
		nBitsToFill = Min1(nBits, cquestion(iByteDest, 8, 8 - nDestPos)); // 往当前memDest填写的位数
		nBitsCanFill = Min1(nBitsToFill, 8 - iBitSrc);					  // memSrc往memDest填的位数
		CopyBitsInAByte((unsigned char *)(memDest + iByteDest), iBitDest, (unsigned char *)(memSrc + iByteSrc), iBitSrc, nBitsCanFill);
		if (nBitsToFill > nBitsCanFill) // memSrc当前字节剩余位不够填满nBitsToFill的情况
		{
			iByteSrc++;				  // 取下一个字节
			iBitSrc = 0;			  // 当前位为0
			iBitDest += nBitsCanFill; // memDest当前位往后移动nBitsCanFill
			CopyBitsInAByte((unsigned char *)(memDest + iByteDest), iBitDest, (unsigned char *)(memSrc + iByteSrc), iBitSrc, nBitsToFill - nBitsCanFill);
			iBitSrc += nBitsToFill - nBitsCanFill; // 更新iBitSrc的位置
		}
		else // memSrc剩余位置够填写nBitsToFill位
		{
			iBitSrc += nBitsCanFill; // 更新iBitSrc的位置
			if (iBitSrc == 8)		 // memSrc当前字节所有的位都用完时，取下一个字节，位数更新为0
			{
				iByteSrc++;
				iBitSrc = 0;
			}
		}
		nBits -= nBitsToFill; // 更新剩余位数
		iByteDest++;		  // memDest的迭代指示字节变量更新到下一个字节
	}
}
//--------------------------------------------------------------------------------------------
//  将字节排列顺序翻转
//--------------------------------------------------------------------------------------------
void Pz_lz77compress::InvertDWord(int *pDW)
{
	unsigned char *puch;
	unsigned char b;
	puch = (unsigned char *)pDW;
	b = *(puch + 0), *(puch + 0) = *(puch + 3), *(puch + 3) = b; // 0位与3位交换
	b = *(puch + 1), *(puch + 1) = *(puch + 2), *(puch + 2) = b; // 1位与2位交换
}
//--------------------------------------------------------------------------------------------
//  设置一个字节中某一位的值
//--------------------------------------------------------------------------------------------
void Pz_lz77compress::SetBit(unsigned char &Abyte, int iBit, unsigned char aBit)
{
	unsigned j = 1;
	j = j << (7 - iBit); // 辅助字节 对应要设置的位置1
	if (aBit > 0)		 // 设置为1的情况
		Abyte = Abyte | j;
	else // 设置为0的情况
		Abyte = Abyte & ~j;
}
//--------------------------------------------------------------------------------------------
//  获取一个字节中指定的位的值
//--------------------------------------------------------------------------------------------
unsigned char Pz_lz77compress::GetBit(unsigned char Abyte, int iBit)
{
	unsigned j = 1;
	j = j << (7 - iBit); // 辅助字节 对应位置1 其他置0
	if ((Abyte & j) > 0) // 将指定位提取出来
		return 1;
	else
		return 0;
}
//--------------------------------------------------------------------------------------------
//  此函数用于当前处理位的迭代，移动num位后将piByte,iBit指示到新的位置
//--------------------------------------------------------------------------------------------
void Pz_lz77compress::MovePos(int &piByte, int &iBit, int num)
{
	num = num + iBit;
	iBit = num % 8;
	piByte += num / 8;
}
//--------------------------------------------------------------------------------------------
//  log2(n)上取整数
//--------------------------------------------------------------------------------------------
int Pz_lz77compress::UpperLog2(int n)
{
	int i, m;
	i = -1, m = 1;
	if (n > 0) //  n=1,2,3...
	{
		for (i = 0;; i++, m = m << 1) // 从m=2^0,2^1,2^2,2^3...中找到不小于n的最小的m
		{
			if (n <= m)
				break;
		}
	}
	return i;
}
//--------------------------------------------------------------------------------------------
//  log2(n)下取整数
//--------------------------------------------------------------------------------------------
int Pz_lz77compress::LowerLog2(int n)
{
	int i, m;
	i = 0, m = 1;
	if (n > 0) //  n=1,2,3...
	{
		for (i = 0;; i++, m = m << 1) // 从m=2^0,2^1,2^2,2^3...中找到大于n的最小的m
		{
			if (m > n)
				break;
		}
	}
	return i - 1;
}
//--------------------------------------------------------------------------------------------
//  将src开始长度srclen的buf压缩到dest
//  返回值>0 压缩到dest的字节数
//  返回值==0 压缩到dest的字节数没有降低，不能压缩
//  返回值<0 压缩异常(压缩是以不超过65536大小的块进行的)
//--------------------------------------------------------------------------------------------
int Pz_lz77compress::Compress(unsigned char *src, int srclen, unsigned char *dest)
{
	int i, off, len, destlen;
	CurByte = 0;		// 当前字节指示变量
	CurBit = 0;			// 当前字节的当前位指示变量
	if (srclen > 65536) // 超过压缩块的上限
		return -1;
	pWnd = src;		  // 窗口初始化
	_InitSortTable(); // 窗口初始化
	i = 0;			  // src当前字节指示变量
	while (i < srclen)
	{
		if (CurByte >= srclen) // 一旦发现压缩到dest的长度超过了srclen，表明不可压缩，返回0
			return 0;
		if (_SeekPhase(src, srclen, i, off, len)) // 如果找到2个以上的匹配串
		{
			_OutCode(dest, 1, 1, false);					 // 向dest输出标志位1
			_OutCode(dest, len, 0, true);					 // 向dest输出len的Gamma编码
			_OutCode(dest, off, UpperLog2(nWndSize), false); // 向dest输出offset,位数为UpperLog2(nWndSize)
			_ScrollWindow(len);								 // 向窗口增加len个字符并更新窗口状态
			i += len - 1;									 // 处理完len个字节，将i往后移len因为和else共享i++所以这里i+=len-1
		}
		else // 没有找到2个以上匹配串的情况
		{
			_OutCode(dest, 0, 1, false);								  // 向dest输出标志位0
			_OutCode(dest, (int)(*(unsigned char *)(src + i)), 8, false); // 将src当前字节直接输出到dest
			_ScrollWindow(1);											  // 增加1个字符并更新窗口状态
		}
		i++;		   // 控制变量指向新的待处理的字节
	}				   // 处理完src
	destlen = CurByte; // 压缩到destlen的字节数，如果CurByte指向的字节已经有位被填写了，该字节也算是被压到dest的字节
	if (CurBit > 0)
		destlen++;
	if (destlen >= srclen) // 恰好处理完最后项时destlen超出srclen,不可压缩 返回0
		return 0;
	else // 返回压缩到dest的实际字节数
		return destlen;
}
//--------------------------------------------------------------------------------------------
//  将被压缩后的dest恢复到src中 其中需要传递srclen保证dest可以处理所有的字节并在处理完最后个字
//  节后停止处理，不会处理最后一个字节剩余的无效位而造成越界访问
//  成功解压返回ture,失败返回false
//--------------------------------------------------------------------------------------------
bool Pz_lz77compress::DeCompress(unsigned char *src, int srclen, unsigned char *dest)
{
	int i, q, len, off, bits, j; // 分别为src的指示变量，Gamma编码中的q，压缩格式的len,off,表示off需要的位数,迭代变量
	unsigned char b;			 // 存储当前位的临时变量
	int dw;
	unsigned char *pb;

	CurByte = 0;		// 初始化dest当前字节指示变量
	CurBit = 0;			// 初始化dest当前字节当前位的指示变量
	pWnd = src;			// 记忆窗口初始化
	nWndSize = 0;		// 窗口大小初始化
	if (srclen > 65536) // src大小异常
		return false;
	i = 0;			   // 初始化当前字节src指示变量
	while (i < srclen) // 恢复了所有字节后退出
	{
		b = GetBit((unsigned char)dest[CurByte], CurBit); // 取dest当前位
		MovePos(CurByte, CurBit, 1);					  // 更新当前位置
		if (b == 0)										  // 如果是0标记，则后面8位就是要恢复到src的字节
		{
			CopyBits(src + i, 0, dest + CurByte, CurBit, 8); // 取dest当前位后8位，附加到src当前字节中
			MovePos(CurByte, CurBit, 8);					 // 更新当前位置
			nWndSize++;										 // 窗口大小增加
		}
		else // 如果是1标记，则后面是len,offset格式，len为Gamma编码
		{
			q = 0;	// q初始值为0
			while (1) // 每取到1，q就+1
			{
				b = GetBit((unsigned char)(dest[CurByte]), CurBit); // 取当前位
				MovePos(CurByte, CurBit, 1);						// 更新位置
				if (b == 0)											// 取到0时退出
					break;
				q++;
			}		   // 退出循环时q刚好为1的个数
			dw = 0;	// 存储余项的临时变量
			if (q > 0) // 存在余项的情况
			{
				pb = (unsigned char *)(&dw);										  // 指向第一个字节
				CopyBits(pb + (32 - q) / 8, (32 - q) % 8, dest + CurByte, CurBit, q); // 余项为q位，取q位
				MovePos(CurByte, CurBit, q);										  // 更新位置
				InvertDWord(&dw);													  // 变成小尾端顺序
				len = 1;
				len = len << q;		// len=2^q;
				len = len + dw + 1; // len-1=2^q+r即 len=2^q+r+1;
			}
			else																		   // q为0时
				len = 2;																   // len=2^0+0+1=1+0+1=2
			dw = 0;																		   // 取off之前初始化临时变量dw
			pb = (unsigned char *)(&dw);												   // 取第一个字节
			bits = UpperLog2(nWndSize);													   // off的位数
			CopyBits(pb + (32 - bits) / 8, (32 - bits) % 8, dest + CurByte, CurBit, bits); // 取bits位
			MovePos(CurByte, CurBit, bits);												   // 更新位置
			InvertDWord(&dw);															   // 变成小尾端顺序
			off = dw;																	   // 设置off的值
			for (j = 0; j < len; j++)													   // 根据len,off将记忆窗口off处开始len个字节复制到src的以i位置开始的地方
				src[i + j] = pWnd[off + j];
			nWndSize += len; // 更新记忆窗口状态
			i += len - 1;	// 更新到下一个要处理src位置，因为和标记0共享i++所以这里i+=len-1
		}
		if (nWndSize > 65536) // 本程序没有使用用滑动功能所以窗口大小不会超过65536，这里不会被执行，如果要增加滑动处理的功能，要修改相应的地方
		{
			pWnd += (nWndSize - 65536);
			nWndSize = 65536;
		}
		i++; // 更新到下一个处理的src位置
	}
	return true;
}
//--------------------------------------------------------------------------------------------
//  isGamama编码决定采用编码方案
//  关于Gamma编码原理:
//  对任意的一个大于1正整数n,用Gamma编码表示n-1的值（n-1>=1）
//  对这个n-1可以定位到一个区间[2^q，2^(q+1))
//  所以n-1=2^q+(n-1-2^q)  余项r=n-1-2^q是不超过2^q的 所以用q位2进制数表示 即n-1=2^q+r;
//  用连续q个1表示n-1中的q,q位2进制值表示r
//  因为Gamma是不定长编码，为了解码的唯一性要符合前缀编码的条件，在q个1和r之间加个0就可以满足
//--------------------------------------------------------------------------------------------
void Pz_lz77compress::_OutCode(unsigned char *dest, int code, int bits, bool isGamma)
{
	unsigned char *pb;
	int aout, dw;
	int Gammacode, q, sh;
	if (isGamma) // 采用Gamma非定长编码
	{
		Gammacode = code - 1;	 // 获取待编码数值
		q = LowerLog2(Gammacode); // 获取q
		if (q > 0)				  // q个1编码
		{
			aout = -1;									// aout全为1
			pb = (unsigned char *)(&aout);				// 指向aout的第一个字节
			CopyBits(dest + CurByte, CurBit, pb, 0, q); // 在当前位后面附加q个1到
			MovePos(CurByte, CurBit, q);				// 更新当前处理位的位置
		}
		aout = 0;									// aout全为0
		pb = (unsigned char *)(&aout);				// 指向aout的第一个字节
		CopyBits(dest + CurByte, CurBit, pb, 0, 1); // 在当前位后面附加1个0
		MovePos(CurByte, CurBit, 1);				// 更新当前处理位的位置
		if (q > 0)									// 余项编码
		{
			sh = 1;
			sh = sh << q;														  // 2^q
			dw = Gammacode - sh;												  // 获得余项值 4个字节 小尾端顺序
			pb = (unsigned char *)(&dw);										  // 指向dw第一个字节
			InvertDWord(&dw);													  // 转换成大尾端顺序
			CopyBits(dest + CurByte, CurBit, pb + (32 - q) / 8, (32 - q) % 8, q); // 将32位dw最低q位从高位到低位的顺序附加到当前位后面
			MovePos(CurByte, CurBit, q);										  // 更新当前处理位的位置
		}
	}
	else
	{
		dw = code;																	   // 获取编码数
		pb = (unsigned char *)(&dw);												   // 指向dw第一个字节
		InvertDWord(&dw);															   // 转换成大尾端顺序
		CopyBits(dest + CurByte, CurBit, pb + (32 - bits) / 8, (32 - bits) % 8, bits); // 将32位dw最低q位从高位到低位的顺序附加到当前位后面
		MovePos(CurByte, CurBit, bits);												   // 更新当前处理位的位置
	}
}
//--------------------------------------------------------------------------------------------
//  从src的nSeekStart处开始在之前出现过的字符即记忆窗口中寻找最大匹配串
//  如果没找到返回false
//  如果找到了返回true,并将偏移位置，长度返填到offset,len中
//--------------------------------------------------------------------------------------------
bool Pz_lz77compress::_SeekPhase(unsigned char *src, int srclen, int nSeekStart, int &offset, int &len)
{
	int j, m, n;			// j为通过_GetSameLen()找到的长度，m,n为当前已经找最大的匹配串的长度，偏移
	unsigned char ch1, ch2; // 接受连续两个字节
	int p;
	if (nSeekStart < srclen - 1) // nSeekStart指向的位置是最后一个位置之前的位置，才可能找到
	{
		ch1 = (unsigned char)src[nSeekStart];
		ch2 = (unsigned char)src[nSeekStart + 1];
		p = SortTable[ch1 * 256 + ch2]; // 找到对应链表第一个位置
		if (p != -1)					// 存在的情况
		{
			m = 2; // 更新当前找到的最大长度，偏移
			n = p;
			while (p != -1) // loop对应链表中所有的位置即所有连续2个字符与nSeekStart开始连续2个字符匹配的位置
			{
				j = _GetSameLen(src, srclen, nSeekStart, p); // 从当前位置找到最大匹配
				if (j > m)									 // 如果超出之前找到的最长匹配，更新
				{
					m = j;
					n = p;
				}
				p = SortFAT[p]; // 取下一个位置
			}					// 循环结束后m,n就是最长匹配对应的len,offset
			offset = n;			// 返填offset,len并返回true
			len = m;
			return true;
		}
	}
	return false; // 找不到匹配两个以上的长度，返回false
}
//--------------------------------------------------------------------------------------------
//  从nSeekStart开始寻找在窗口pWnd offset位置开始的最长匹配串
//  offset,nSeekStart开始已经有2个匹配字符了
//--------------------------------------------------------------------------------------------
int Pz_lz77compress::_GetSameLen(unsigned char *src, int srclen, int nSeekStart, int offset)
{
	int i, maxsame;
	i = 2;
	maxsame = Min1(srclen - nSeekStart, nWndSize - offset); // 最大可能匹配大小
	for (i = 2; i < maxsame; i++)							// 有不匹配的情况下退出循环，i就是匹配串的大小，所有项匹配的情况下i刚好是maxsame因此i就是匹配长度
	{
		if (src[nSeekStart + i] != pWnd[offset + i])
			break;
	}
	return i;
}
//--------------------------------------------------------------------------------------------
//  连续加入n个字符后，更新窗口状态
//--------------------------------------------------------------------------------------------
void Pz_lz77compress::_ScrollWindow(int n)
{
	for (int i = 0; i < n; i++)
	{
		nWndSize++;		  // 窗口大小+1
		if (nWndSize > 1) // 加入新串后更新状态
			_InsertIndexItem(nWndSize - 2);
	}
}
//--------------------------------------------------------------------------------------------
//  插入1个字符后，更新窗口相关状态值
//--------------------------------------------------------------------------------------------
void Pz_lz77compress::_InsertIndexItem(int off)
{
	unsigned char ch1, ch2; // 取最新的两个字符
	int offentry;
	ch1 = (unsigned char)pWnd[off];
	ch2 = (unsigned char)pWnd[off + 1];
	if (ch1 != ch2) // 连续两个不一样的字符
	{
		offentry = SortTable[ch1 * 256 + ch2]; // 这两个连续字符在之前的串中是否出现过
		if (offentry == -1)					   // 没有出现过，那么该位置就是其出现的位置
			SortTable[ch1 * 256 + ch2] = off;
		else // 如果出现过,通过SortFAT链表找到最后一个串出现的位置
		{
			while (SortFAT[offentry] != -1)
			{
				offentry = SortFAT[offentry];
			}
			SortFAT[offentry] = off; // off位置是最后项的后面一项，off成为新的末项
		}
	}
	else // 连续两个一样的字符
	{
		if ((lastbyte == ch1) & (off > 0))	 // 两个一样的字符，而且之前的lastbyte也一样(lastbyte)仅当off>0有效
			return;							   // 这样做是为了处理连续出现字符的情况，但是本程序是经过简化的，因为没用上这个功能，如果要提升压缩效果，要改进SortFAT的结构
		offentry = SortTable[ch1 * 256 + ch2]; // 处理方式同ch1!=ch2
		if (offentry == -1)
			SortTable[ch1 * 256 + ch2] = off;
		else
		{
			while (SortFAT[offentry] != -1)
			{
				offentry = SortFAT[offentry];
			}
			SortFAT[offentry] = off;
		}
	}
	lastbyte = ch1; // 更新lastbyte
}
//--------------------------------------------------------------------------------------------
//  记忆窗口的初始化
//--------------------------------------------------------------------------------------------
void Pz_lz77compress::_InitSortTable()
{
	memset(SortTable, -1, sizeof(int) * 65536);
	memset(SortFAT, -1, sizeof(int) * 65536);
	nWndSize = 0;
	lastbyte = 0;
}

// 以LZ77压缩算法压缩数据
int Pz_lz77compress::lz77Compress(unsigned char *dest, int destlen, unsigned char *source, int sourcelen)
{
	int last, act;
	unsigned short flag1, flag2;
	unsigned char *isource = source;
	unsigned char *idest = dest;
	unsigned char destbuf[65536 + 16];
	int iRelen = 0;
	last = sourcelen; // last为文件中还未处理的字节数
	while (last > 0)  // 处理所有字节退出
	{
		act = Min1(65536, last); // act为每一次压缩块的大小
		last -= act;			 // 更新文件剩余字节数
		if (act == 65536)		 // 0表示完整的一块
			flag1 = 0;
		else // 非0表示原始块的实际大小
			flag1 = act;
		iRelen += 2;
		if (destlen < iRelen)
			return -1;
		memcpy(idest, (unsigned char *)(&flag1), 2); // 将标记送往输出文件
		idest += 2;
		int iRet = Compress(isource, act, destbuf); // soubuf压缩到destbuf
		if (iRet == 0)								// 无法压缩的情况  将soubuf直接送往输出文件
		{
			flag2 = flag1; // 标记为压缩处理后送往输出文件的实际字节数
			iRelen += (2 + act);
			if (destlen < iRelen)
				return -1;
			memcpy(idest, (unsigned char *)(&flag2), 2); // 输出标记
			idest += 2;
			memcpy(idest, isource, act); // 输出soubuf
			idest += act;
		}
		else // 可以压缩的情况
		{
			flag2 = iRet; // 获取标记值
			iRelen += (2 + iRet);
			if (destlen < iRelen)
				return -1;
			memcpy(idest, (unsigned char *)(&flag2), 2); // 送输出标记
			idest += 2;
			memcpy(idest, destbuf, iRet); // 输出destbuf
			idest += iRet;
		}
		isource += act;
	}
	return iRelen;
}

// 以LZ77压缩算法压缩数据
int Pz_lz77compress::lz77Expand(unsigned char *dest, int destlen, unsigned char *source, int sourcelen)
{
	int last, act;
	unsigned short flag1, flag2;
	unsigned char *isource = source;
	unsigned char *idest = dest;
	unsigned char destbuf[65536 + 16];
	int iRelen = 0;

	last = sourcelen; // 输入文件剩余位处理的字节数
	while (last > 0)  // 所有字节被处理退出
	{
		memcpy((unsigned char *)(&flag1), isource, 2);
		isource += 2;
		memcpy((unsigned char *)(&flag2), isource, 2);
		isource += 2;
		last -= 2 * sizeof(unsigned short); // 更新剩余字节数
		if (flag1 == 0)						// 原始块是一整块65536字节
			act = 65536;
		else // 原始块小于65536字节
			act = flag1;
		if (flag2 > 0)	 // 压缩处理后 块的大小
			last -= flag2; // 更新剩余处理字节
		else			   // 压缩处理后块的大小是整块  说明原始块也是整块 所以压缩处理后块的大小是原始快的大小即act
			last -= act;
		iRelen += act;
		if (destlen < iRelen)
			return -1;
		if (flag2 == flag1) // 压缩处理后大小没变
		{
			memcpy(idest, isource, act); // 当前处理soubuf就是原始块的实际内容
			idest += act;
			isource += act;
		}
		else // 压缩处理后变小了
		{
			DeCompress(destbuf, act, isource); // 将当前处理块解压到原始块
			memcpy(idest, destbuf, act);
			idest += act;
			isource += flag2;
		}
	}
	return iRelen;
}
