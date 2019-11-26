#ifndef PZ_LZ77_COMPRESS
#define PZ_LZ77_COMPRESS

// 备注:LZ77的原理比较复杂，下面对压缩原理和方法做简单介绍。

// LZ77算法简介:
// LZ77算法在某种意义上又可称为“滑动窗口压缩”，该算法将一个虚拟的，可以跟随压缩继承滑动的窗口作为
// 术语字典，要压缩的字符串如果在该窗口中出现，则输出其出现位置和长度。使用固定大小窗口进行术语匹配，而不是在所有
// 已经编码的信息中匹配，是因为匹配算法的时间消耗往往较多，必须限制字典的大小才能保证算法的效率；随着压缩的进程，
// 滑动字典窗口，使其中总包含最近编码过的信息，因为对于大多数信息而言，要编码的字符串往往在最近的上下文中更容易
// 找到匹配串。

// 压缩基本流程:
// 1、从当前位置开始，考察位编码的数据，并试图在滑动窗口中找出最长的匹配字符串，如果找到，则进行步骤2，否则进行步骤3.
// 2、输出三元符号组 ( off, len, c )。其中 off 为窗口中匹配字符串相对窗口边界的偏移，len 为可匹配的长度，c 为下一个字符。
//    然后将窗口向后滑动 len + 1 个字符，继续步骤 1。
// 3、输出三元符号组 ( 0, 0, c )。其中 c 为下一个字符。然后将窗口向后滑动 len + 1 个字符，继续步骤 1。

// 举例说明:
// 假设窗口的大小为 10 个字符，我们刚编码过的 10 个字符是：abcdbbccaa，即将编码的字符为：abaeaaabaee,
// 首先发现，可以和要编码字符匹配的最长串为 ab ( off = 0, len = 2 ), ab 的下一个字符为 a，输出三元组：( 0, 2, a )
// 现在窗口向后滑动 3 个字符，窗口中的内容为：dbbccaaaba，下一个字符 e 在窗口中没有匹配，我们输出三元组：( 0, 0, e )
// 窗口向后滑动 1 个字符，其中内容变为：bbccaaabae，要编码的 aaabae 在窗口中存在( off = 4, len = 6 )，其后的字符为 e，
// 可以输出：( 4, 6, e )。将可以匹配的字符串都变成了指向窗口内的指针，并由此完成了对上述数据的压缩。
// 在解压缩的过程中，只要向压缩时那样维护好滑动的窗口，随着三元组的不断输入，我们在窗口中找到相应的匹配串，
// 缀上后继字符 c 输出（如果 off 和 len 都为 0 则只输出后继字符 c )即可还原出原始数据。

// 编码方法:
// 我们必须精心设计三元组中每个分量的表示方法，才能达到较好的压缩效果。一般来讲，编码的设计要根据待编码的数值的分布情况而定。
// 对于三元组的第一个分量——窗口内的偏移，通常的经验是，偏移接近窗口尾部的情况要多于接近窗口头部的情况，这是因为字符串在与
// 其接近的位置较容易找到匹配串，但对于普通的窗口大小（例如 4096 字节）来说，偏移值基本还是均匀分布的，我们完全可以用固定的
// 位数来表示它。编码 off 需要的位数 bitnum = upper_bound( log2( MAX_WND_SIZE ))。
// 由此，如果窗口大小为 4096，用 12 位就可以对偏移编码。如果窗口大小为 2048，用 11 位就可以了。复杂一点的程序考虑到在压缩
// 开始时，窗口大小并没有达到 MAX_WND_SIZE，而是随着压缩的进行增长，因此可以根据窗口的当前大小动态计算所需要的位数，这样
// 可以略微节省一点空间。
// 对于第二个分量——字符串长度，我们必须考虑到，它在大多数时候不会太大，少数情况下才会发生大字符串的匹配。显然可以使用一种
// 变长的编码方式来表示该长度值。在前面我们已经知道，要输出变长的编码，该编码必须满足前缀编码的条件。
// 该类中用的是Gamma编码，即γ 编码：
// 它分作前后两个部分，假设对 x 编码，令 q = int( log2x )，则编码的前一部分是 q 个 1 加一个 0，后一部分是 q 位长的二进制数，
// 其值等于 x - 2q 。γ编码表如下：
// 值 x    γ编码
// ---------------------
//	  1       0
//    2      10 0
//    3      10 1
//    4     110 00
//    5     110 01
//    6     110 10
//    7     110 11
//    8    1110 000
//    9    1110 001
// 对三元组的最后一个分量——字符 c，因为其分布并无规律可循，我们只能老老实实地用 8 个二进制位对其编码。

// 输出方法的改进，即本类用到的方法:
// LZ77 的原始算法采用三元组输出每一个匹配串及其后续字符，即使没有匹配，我们仍然需要输出一个 len = 0 的三元组来表示单个字符。
// 试验表明，这种方式对于某些特殊情况（例如同一字符不断重复的情形）有着较好的适应能力。但对于一般数据，我们还可以设计出另外
// 一种更为有效的输出方式：将匹配串和不能匹配的单个字符分别编码、分别输出，输出匹配串时不同时输出后续字符。
// 将每一个输出分成匹配串和单个字符两种类型，并首先输出一个二进制位对其加以区分。例如，输出 1 表示下面是一个匹配串，输出 0
// 表示下面是一个单个字符。
// 之后，如果要输出的是单个字符，我们直接输出该字符的字节值，这要用 8 个二进制位。也就是说，我们输出一个单个的字符共需要 9
// 个二进制位。如果要输出的是匹配串，我们按照前面的方法依次输出 off 和 len。对 off，我们可以输出定长编码，也可以输出变长
// 前缀码，对 len 我们输出变长前缀码。有时候我们可以对匹配长度加以限制，例如，我们可以限制最少匹配 3 个字符。因为，对于 2
// 个字符的匹配串，我们使用匹配串的方式输出并不一定比我们直接输出 2 个单个字符（需要 18 位）节省空间（是否节省取决于我们采
// 用何种编码输出 off 和 len）。

// 查找匹配串:
// 在滑动窗口中查找最长的匹配串，大概是 LZ77 算法中的核心问题。容易知道，LZ77 算法中空间和时间的消耗集中于对匹配串的查找算法。
// 该类中用到的匹配算法为:将窗口中每个长度为 2 的字符串建立索引，先在此索引中匹配，之后对得出的每个可匹配位置进行顺序查找，
// 直到找到最长匹配字符串。

// 完整描述见文件顶部
class Pz_lz77compress
{
private:
	unsigned char *pWnd;	// 窗口指针，窗口最大为64K，并且不做滑动，每次最多压缩64K数据
	int nWndSize;			// 当前窗口大小
	int SortTable[65536];   // 查找两个连续字符串在窗口中出现位置在SortFAT中下标
	int SortFAT[65536];		// 对滑动窗口中每一个2字节串排序，排序是为了进行快速术语匹配，保存该2字节串的每一个出现位置
	int CurByte;			// 当前输出位置的字节偏移
	int CurBit;				// 当前输出位置的位偏移
	unsigned char lastbyte; // 窗口中的最后一个字符
protected:
	int cquestion(int a, int b, int c); // (a>0)?b:c

	// 在一个字节范围内复制位流
	// @param1 [in]		memDest		目标数据区
	// @param2 [in]		nDestPos	目标数据区第一个字节中的起始位
	// @param3 [in]		memSrc		源数据区
	// @param4 [in]		nSrcPos		源数据区第一个字节中的起始位
	// @param5 [in]		nBits		要复制的位数
	void CopyBitsInAByte(unsigned char *memDest, int nDestPos, unsigned char *memSrc, int nSrcPos, int nBits);

	// 复制内存中的位流
	// @param1 [in]		memDest		目标数据区
	// @param2 [in]		nDestPos	目标数据区第一个字节中的起始位
	// @param3 [in]		memSrc		源数据区
	// @param4 [in]		nSrcPos		源数据区第一个字节中的起始位
	// @param5 [in]		nBits		要复制的位数
	void CopyBits(unsigned char *memDest, int nDestPos, unsigned char *memSrc, int nSrcPos, int nBits);

	// 将字节排列顺序翻转，X86为小尾端，TCP/IP为大尾端，要进行大小尾端转化
	void InvertDWord(int *pDW);

	// 设置一个字节指定位的值
	// @param1 [in][out]Abyte		一个字节
	// @param2 [in]		iBit		指定的字节中位置，高位起从0计数(左起)
	// @param3 [in]		aBit		要设置的值
	void SetBit(unsigned char &Abyte, int iBit, unsigned char aBit);

	// 得到一个字节指定位的值
	// @return 字节中指定位的值
	// @param1 [in]		Abyte		一个字节
	// @param2 [int]	iBit		指定的字节中位置，高位起从0计数(左起)
	unsigned char GetBit(unsigned char Abyte, int iBit);

	// 移动字节和字节内num位
	// @param1 [in]		piByte		第几个字节
	// @param2 [in]		iBit		该字节第几个位
	// @param3 [in]		num			移动的个数
	void MovePos(int &piByte, int &iBit, int num);

	// 取得Log2(n)上的整数
	// @return 不小于以2为底n的对数的最小整数
	// @param1 [int]	n			参与计算的真数
	int UpperLog2(int n);

	// 取得log2(n)下的整数
	// @return 不大于以2为底n的对数的最大整数
	// @param1 [int]	n			参与计算的真数
	int LowerLog2(int n);

	int Min1(int a, int b); // 取两个数据的最小值
public:
	// 压缩数据，每次压缩65536个字符
	// @return 压缩后数据长度
	// @param1 [out]	dest		保存压缩后的数据
	// @param2 [in]		destlen		要压缩数据帧的总长度(包括头帧)
	// @param3 [in]		source		要压缩的数据
	// @param4 [in]		sourcelen	要压缩数据的长度(不包括头帧)
	int lz77Compress(unsigned char *dest, int destlen, unsigned char *source, int sourcelen);

	// 解压数据，每次解压65536个字符
	// @return 解压后数据长度
	// @param1 [out]	dest		保存解压后的数据
	// @param2 [in]		destlen		保存解压数据的总长度
	// @param3 [in]		source		要解压的数据
	// @param4 [in]		sourcelen	要解压数据的长度
	int lz77Expand(unsigned char *dest, int destlen, unsigned char *source, int sourcelen);

protected:
	// 压缩数据
	// @return 压缩后数据长度
	// @param1 [in]		src			要压缩的数据
	// @param2 [in]		srclen		要压缩数据的长度，不能超过65536
	// @param3 [out]	dest		压缩后的数据
	int Compress(unsigned char *src, int srclen, unsigned char *dest);

	// 解压数据
	// @return 解压是否成功
	// @param1 [out]	src			解压后的数据
	// @param2 [in]		srclen		保存解压数据的长度
	// @param3 [in]		dest		要解压的数据
	bool DeCompress(unsigned char *src, int srclen, unsigned char *dest);

	// 向目标区输出压缩码
	// @param1 [out]	dest		保存压缩码的目标区
	// @param2 [in]		code		要编码的数据
	// @param3 [in]		bits		要输出的位数，当isGamma==false时有效
	// @param4 [in]		isGamma		是否采用Gamma编码
	void _OutCode(unsigned char *dest, int code, int bits, bool isGamma);

	// 从src的nSeekStart中查找与窗口匹配的最大字符串
	// @return 是否找到2个以上的匹配串
	// @param1 [in]		src			要压缩的数据
	// @param2 [in]		srclen		要压缩数据的长度
	// @param3 [in]		nSeekStart	在src中开始匹配的位置
	// @param4 [out]	offset		找到最大匹配字符串在窗口的偏移量
	// @param5 [out]	len			找到最大匹配字符串的长度
	bool _SeekPhase(unsigned char *src, int srclen, int nSeekStart, int &offset, int &len);

	// 从窗口offset开始与原数据nSeekStart开始查找到相同字符的个数
	// @return 找到的最长匹配串长度
	// @param1 [in]		src			要压缩的数据
	// @param2 [in]		srclen		要压缩数据的长度
	// @param3 [in]		nSeekStart	在src中开始匹配的位置
	// @param4 [in]		offset		在窗口中开始匹配的位置
	int _GetSameLen(unsigned char *src, int srclen, int nSeekStart, int offset);

	// 向窗口中增加n个字符，并更新窗口状态
	// @param1 [in]		n			向窗口中添加字符的个数
	void _ScrollWindow(int n);

	// 插入一个字符后，更新窗口相关状态
	// @param1 [in]		off			窗口中位置
	void _InsertIndexItem(int off);

	void _InitSortTable(); // 初始化窗口
};
#endif