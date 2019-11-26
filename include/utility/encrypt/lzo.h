#ifndef PZ_LZO_DEFLATE
#define PZ_LZO_DEFLATE
#define PZ_HASH(a, b, c) ((unsigned char)a + ((unsigned char)b ^ (unsigned char)c))

// 备注：介绍lzo压缩算法原理和实现步骤等。
// lzo简介：lzo是致力于解压速度的一种数据压缩算法，它将数据块压缩成匹配数据（滑动字典）与非匹配文字的序列。
// 如：要压缩的数据为：ABCDEFGABCD。先计算新字符串的长度，可以知道新字符串长度为7,所以第一个传送的数据为7，之后
// 再加上新字符串，然后继续寻找相同字符串，所以最后压缩后字符串为：7，A,B,C,D,E,F,G,7,4。
// lz77在压缩速度并不令人满意，是因为lz77算法需要在滑动窗口中，做最大匹配字串的搜索，必须一个一个做比对，并找出最大长度的
// 重复长度，因此这种算法非常耗费时间，而且滑动窗口越大，搜索时间就越长。而lzo在速度上的改进，就是放弃使用最大的匹配字串，
// 而使用寻找相对最佳长度的字符串，虽然会使得lzo因此损失很大压缩率，所以lzo必须在速度和压缩率之间有取舍。

// lzo压缩格式(本文件中做了简化)：
// 一、当重复字符串长度在8以下：
//     1、offset<2048时：
//		  len(3)offset(3)reserve(2)offset(8)
//	   2、offset>2048时：
//        00100len(3)offset(6)reserve(2)offset(8)
// 二、当重复字符串长度超过8：
//	   00100000len(8)offset(6)reserve(2)offset(8)
// lzo之所以定义以上的压缩格式有这样几个目的：第一，调整lzo的压缩率，使得lzo可以适合来压缩重复长度较短，但是offset比较长
// 的数据，因为lzo是针对比较小的数据来处理，所以通常这种类型中重复长度都不长，所以lzo把重复长度(len)的位数可以减少，然后
// 增加(偏移)offset的位数。另一方面，lzo减少重复长度(len)的位数也会造成不好的影响，使得重复性很高的数据，压缩效能下降,
// 但lzo只要超过8次就增加，就可以针对压缩数据类型做最合适的格式调整。第二，可以使解压缩时更简单，快速，而且不需要保存额外
// 的压缩信息。如第一种压缩格式中第一个字节，由3个重复长度、3个偏移量和2个保留位组成，又因为重复长度一定有值，而且一定会
// 大于三次以上，因为如果重复长度小于3，不值得压缩，当做新符号不做处理，因此重复长度一定大于3，所以，第一个字节
// (010 000 00)2=64，所以只要判断第一个字节是否大于64，就可以知道是第一种压缩格式。
// lzo压缩格式中还有两个保留位，对压缩率影响很大。lzo利用最前面的位表示新字符的数目，所以当出现3个以下的新字符数据时，就必
// 须增加一个位来表示，对这样的数据，压缩率就会大幅度下降，为了避免这样的情况，lzo在新字符个数3个以下时，把新字符的数目往前
// 填，填到前面的重复长度和偏移量字节里，所以在解码的时候，如果如果这两个保留位中有值，代表处理完重复长度和偏移量后，后面接着的
// 不是新符号数目，而是直接就是新符号，所以直接传送出去，这样可以省去一个字节的数据，减少压缩率的损失。

// lzo最前面的四个字符不做任何处理，主要也是因为这两个保留位的关系。如：要压缩的数据为"AAAAAAA"，一开始先把AAA做hash运算，
// 接着处理的依然是AAA，与之前字符串重复了，要计算新字符个数，之后计算重复个数和偏移量，但在计算新字符个数时，因为新字符只
// 出现一次，新字符个数小于3，所以必须要利用前面重复字符串的重复长度和偏移量来放新字符的个数，但是这个数据是第一个数据，前
// 面没有任何数据可以存放，因此会发生错误。

// 类名：CLzoSlipWindow
// 功能：做为lzo压缩解压缩中的滑动窗口，为了提高速度，放弃使用lz77的最大匹配字符串来压缩，采用计算新字符串的长度。
// 用法：当压缩数据时，新字符要与滑动窗口中字符相匹配，决定编码方式；解压数据时，从滑动窗口中取得字符，并更新滑动窗口。
class CLzoSlipWindow
{
public:
	int index_of_key_start[256];				// 保存PZ_HASH值
	int prev_of_str[4096];						// 保存next_of_str的索引
	int next_of_str[4096];						// 保存index_of_key_start的值
	unsigned char content_of_slip_window[4096]; // 滑动窗口中的值
	int index_of_start;							// 滑动窗口的开始索引
	int index_of_end;							// 滑动窗口的结束索引
	int window_currentlen;						// 当前滑动窗口的长度

	void init_slip_window(); // 初始化滑动窗口

	// 向滑动窗口中加入一个字符
	// @param1 [in]			ch			向滑动窗口中加入的字符
	void Add_char_to_currentwindow(unsigned char ch);
};

// 类名：CLzoCompress
// 功能：以lzo算法对数据压缩解压缩。
// 用法：直接调用函数即可。
class CLzoCompress
{
public:
	// 压缩数据
	// @return 压缩后数据的长度
	// @param1 [out]		dest		保存压缩后的数据
	// @param2 [in]			destlen		保存压缩后数据的长度
	// @param3 [in]			source		要压缩的数据
	// @param4 [in]			sourcelen	要压缩数据的长度
	int compress(unsigned char *dest, int destlen, unsigned char *source, int sourcelen);

	// 解压缩数据
	// @return 解压缩后数据的长度
	// @param1 [out]		dest		保存解压缩后的数据
	// @param2 [in]			destlen		保存解压缩后数据的长度
	// @param3 [in]			source		要解压的数据
	// @param4 [in]			sourcelen	要解压数据的长度
	int uncompress(unsigned char *dest, int destlen, unsigned char *source, int sourcelen);
};
#endif