#include <cstring>
#include "encrypt/lzo.h"

// 初始化滑动窗口
void CLzoSlipWindow::init_slip_window()
{
	memset(content_of_slip_window, 0, sizeof(unsigned char) * 4096);
	int i = 0;
	for (i = 0; i < 256; i++)
	{
		index_of_key_start[i] = -1;
	}
	for (i = 0; i < 4096; i++)
	{
		next_of_str[i] = -1;
		prev_of_str[i] = -1;
	}
	index_of_start = -1;
	index_of_end = -1;
	window_currentlen = 0;
}

// 把一个字符加入到滑动窗口中
void CLzoSlipWindow::Add_char_to_currentwindow(unsigned char ch)
{
	if (window_currentlen == 0)
	{
		index_of_start = 0;
		index_of_end = 0;
		content_of_slip_window[index_of_end] = ch;
		window_currentlen++;
	}
	else if (window_currentlen == 1)
	{
		index_of_end++;
		content_of_slip_window[index_of_end] = ch;
		window_currentlen++;
	}
	else if (window_currentlen < 4096)
	{
		index_of_end++;
		content_of_slip_window[index_of_end] = ch;
		window_currentlen++;
		unsigned char a, b, c;
		a = content_of_slip_window[(index_of_end)-2];
		b = content_of_slip_window[(index_of_end)-1];
		c = content_of_slip_window[index_of_end];
		unsigned char key = PZ_HASH(a, b, c); // 将最后三个字符hash运算
		int index = index_of_key_start[key];  // 以hash运算结果作为下标
		if (index == -1)
			index_of_key_start[key] = (index_of_end)-2; // 给以hash运算结果为下标的数组赋值
		else
		{
			next_of_str[(index_of_end)-2] = index;
			prev_of_str[index] = (index_of_end)-2;
			index_of_key_start[key] = (index_of_end)-2;
		}
		//Todo:
	}
	else
	{
		unsigned char a, b, c;
		a = content_of_slip_window[index_of_start]; // 当滑动窗口写满时，开始新的hash运算
		b = content_of_slip_window[((index_of_start) + 1) % 4096];
		c = content_of_slip_window[((index_of_start) + 2) % 4096];
		unsigned char key = PZ_HASH(a, b, c);
		int index = index_of_key_start[key];
		if (index == index_of_start)
		{
			index_of_key_start[key] = next_of_str[index_of_start];
			if (index_of_key_start[key] != -1)
			{
				prev_of_str[index_of_key_start[key]] = -1;
			}
			next_of_str[index_of_start] = -1;
			prev_of_str[index_of_start] = -1;
		}
		else
		{
			next_of_str[prev_of_str[index_of_start]] = next_of_str[index_of_start];
			if (next_of_str[index_of_start] != -1)
				prev_of_str[next_of_str[index_of_start]] = prev_of_str[index_of_start];
			next_of_str[index_of_start] = -1;
			prev_of_str[index_of_start] = -1;
		}
		//Todo:delete something

		index_of_start = ((index_of_start) + 1) % 4096; // 添加新的数据
		index_of_end = ((index_of_end) + 1) % 4096;
		content_of_slip_window[index_of_end] = ch;

		a = content_of_slip_window[((index_of_end) + 4094) % 4096];
		b = content_of_slip_window[((index_of_end) + 4095) % 4096];
		c = content_of_slip_window[index_of_end];
		key = PZ_HASH(a, b, c);
		index = index_of_key_start[key];
		if (index == -1)
			index_of_key_start[key] = ((index_of_end) + 4094) % 4096;
		else
		{
			index_of_key_start[key] = ((index_of_end) + 4094) % 4096;
			next_of_str[index_of_key_start[key]] = index;
			prev_of_str[index] = index_of_key_start[key];
		}
		//Todo:add something
	}
}

// 压缩数据，压缩过程：
/* 1、把前4个字符加入到滑动窗口中；
   2、读取下面的字符，进行hash运算，到滑动窗口中匹配字符串，如果是新字符串，进行步骤3，否则进行步骤4；
   3、如果新字符的个数小于3，把新字符个数填到前面的重复长度和偏移量字节中的保留字节中，后面是新字符串；
	  如果新字符的个数大于3小于18，前面的重复长度和偏移量字节中的保留字节为空，后面是新字符个数和新字符串；
	  如果新字符的个数大于18小于255，前面的重复长度和偏移量字节中的保留字节为空，后面是0x00、新字符个数和新字符串；
	  如果新字符的个数大于255，不压缩，退出。
   4、如果匹配长度小于3，对新符号不压缩；
      如果匹配长度大于3小于8，并且offset小于2048，编码格式为len(3)offset(3)reserve(2)offset(8)；
	  如果匹配长度大于3小于8，并且offset大于2048，编码格式为00100len(3) offset(6)reserve(2) offset(8)；
	  如果匹配长度大于8小于255，编码格式为00100000 len(8) offset(6)reserve(2) offset(8)。
   5、更新滑动窗口，继续步骤2，直到完成压缩所有数据。  
*/
int CLzoCompress::compress(unsigned char *dest, int destlen, unsigned char *source, int sourcelen)
{
	if (dest == 0 || destlen <= 0 || source == 0 || sourcelen <= 3) // 参数检查
		return -1;

	CLzoSlipWindow window;					 // 滑动窗口
	window.init_slip_window();				 // 滑动窗口初始化
	unsigned char *iterator_dest = dest;	 // 目的BUF迭代
	unsigned char *iterator_source = source; // 源BUF迭代
	int real_in_dest = 0;					 // 压缩到dest的实际字节数
	unsigned char *newbyte_start = source;   // 第一个新字符的位置
	unsigned char *dest_modify_pos = 0;		 // 指示待修正的位置

	if (sourcelen <= 6) // 当长度不超过6时编码格式为 新字符个数+所有新字符的内存表示 加结束标记0x0000
	{
		real_in_dest = 1 + sourcelen + 2;			 // 写入到dest的字节数
		if (destlen < real_in_dest + 1)				 // 检查dest是否有足够的buf来容纳写入的字节
			return -1;								 // 不够的情况下返回失败标识-1
		int newbyte = sourcelen - 3;				 // 个数在4到18之间用一个字节表示,对应一个8位二进制值+3,该二进制值区间[1，15]
		*(iterator_dest++) = (unsigned char)newbyte; // 将这个二进制内存表示放在dest里
		int i = 0;
		for (i = 0; i < sourcelen; i++) // 将所有新字符加入到dest
			*(iterator_dest + i) = *(iterator_source + i);
		*(short *)(iterator_dest + i) = 0; // 加入结束标记0x0000 两个字节
		return real_in_dest;			   // 返回压入dest实际字节数
	}

	//---------------------以下是字节个数达到7以上的情况--------------------------------------
	//
	//    7个以上的时候要寻找与滑动窗口匹配的字符串，因为第一个元组必然是表示新字节的，而1到3
	//    范围内的个数是存在之前的len+offset元组中，此时还没有这样的元组，所以将前4个都看成新
	//    字节（即使从3个或第4个能找到匹配项），从第5个开始寻找匹配项
	//----------------------------------------------------------------------------------------
	int i = 0;
	for (i = 0; i < 4; i++) // 将前4个字符加入到滑动窗口里
		window.Add_char_to_currentwindow(*(iterator_source++));

	for (i = 4; i < sourcelen - 2; i++) // 考虑所有可能匹配的连续3个字符
	{
		iterator_source = source + i; // 设置迭代器的位置
		unsigned char a, b, c;		  // 从迭代器位置开始连续3个字符存放在a,b,c处
		a = *iterator_source;
		b = *(iterator_source + 1);
		c = *(iterator_source + 2);
		unsigned char key = PZ_HASH(a, b, c);		// key=f(a,b,c)的值，将a,b,c按映射值key划分到key的空间里，缩小匹配范围
		int index = window.index_of_key_start[key]; // key空间的第一个key=f(a,b,c)位置

		if (index != -1) // 表示当前key空间为非空，a,b,c所属空间可能存在与a,b,c匹配的串，开始寻找
		{
			//Todo:
			unsigned short offset = 0;							   // 匹配到的串的下标
			unsigned short len = 0;								   // 匹配串的长度
			for (; index != -1; index = window.next_of_str[index]) // 遍历所有f(x,y,z)=key的xyz
			{
				int count = 0; // 与当前xyz匹配的长度
				for (;;)
				{
					if (sourcelen == (i + count)) // 匹配完了source的最后一项，退出
						break;
					if (*(iterator_source + count) != window.content_of_slip_window[(index + count) % 4096])
						break; // 当前字符匹配，退出
					count++;
					if (count == 255) // 最大匹配长度255，不再往后匹配
						break;
					if (window.index_of_end == (index + count - 1) % 4096) // 滑动窗口的最后项匹配后不再匹配
						break;
				}
				if (count > len) // 如果找到更大的就更新 len offset
				{
					len = count;
					offset = index;
				}
				if (len == 255) // len被更新到255，停止遍历
					break;
			}
			if (len < 3) // 匹配的串长度小于3的编码方式
				window.Add_char_to_currentwindow(*iterator_source);
			else //if(len<=8)										// 长度为3到8之间的编码方式
			{
				int local = (int)(iterator_source - newbyte_start); // 新字符的个数
				if (local > 255)
					return -1;													   // 出现连续255以上新字符的情况不压缩
				if (local == 0)													   // 0个新字符的处理方式
					*dest_modify_pos = (*(unsigned char *)dest_modify_pos) & 0xFC; // 待修正的dest处
				else if (local > 0 && local < 4)								   // 1到3个字符的处理方式
				{
					real_in_dest += local;
					if (destlen < real_in_dest + 1)
						return -1;
					*dest_modify_pos = ((*(unsigned char *)dest_modify_pos) & 0xFC) | ((unsigned char)local & 0x03);
					for (int i = 0; i < local; i++)
						*(iterator_dest++) = *(newbyte_start++);
				}
				else if (local >= 4 && local <= 18) // 4到18个字符的处理方式
				{
					real_in_dest += (local + 1);
					if (destlen < real_in_dest + 1)
						return -1;
					if (dest_modify_pos != 0)
						*dest_modify_pos = (*(unsigned char *)dest_modify_pos) & 0xFC;
					unsigned char bit = (unsigned char)(local - 3);
					*(iterator_dest++) = bit;
					for (int i = 0; i < local; i++)
						*(iterator_dest++) = *(newbyte_start++);
				}
				else // 19到255个字符的处理方式
				{
					real_in_dest += (2 + local);
					if (destlen < real_in_dest + 1)
						return -1;
					if (dest_modify_pos != 0)
						*dest_modify_pos = (*(unsigned char *)dest_modify_pos) & 0xFC;
					unsigned char bit1 = 0x00;
					unsigned char bit2 = (unsigned char)local;
					*(iterator_dest++) = bit1;
					*(iterator_dest++) = bit2;
					for (int i = 0; i < local; i++)
						*(iterator_dest++) = *(newbyte_start++);
				}			  // end if...else
				if (len <= 8) // 长度为3到8之间的编码方式
				{
					if (offset < 2048) // 低于2048时，格式为len(3)offset(3)reserve(2) offset(8)
					{
						real_in_dest += 2;
						if (destlen < real_in_dest + 1)
							return -1;
						unsigned char bitlen = (((unsigned char)(len - 1)) << 5) & 0xE0;  // len放在高3位
						unsigned char bitoffset1 = (((unsigned char)offset) << 2) & 0x1C; // offset的低3位放在3，4，5位
						unsigned char bitoffset2 = (unsigned char)(offset >> 3);		  // offset的4到11位
						dest_modify_pos = iterator_dest;
						*(iterator_dest++) = (bitlen | bitoffset1) & 0xFC;
						*(iterator_dest++) = bitoffset2;
					}
					else // 超过了2048时 格式为00100len(3) offset(6)reserve(2) offset(8)
					{
						real_in_dest += 3;
						if (destlen < real_in_dest + 1)
							return -1;
						unsigned char bit1 = ((unsigned char)(len - 1)) | 0x20;
						unsigned char bit2 = (((unsigned char)offset) << 2) & 0xFC;
						unsigned char bit3 = (unsigned char)(offset >> 6);
						*(iterator_dest++) = bit1;
						dest_modify_pos = iterator_dest;
						*(iterator_dest++) = bit2;
						*(iterator_dest++) = bit3;
					}
				}
				else // 长度为9到255之间的编码方式 格式00100000 len(8) offset(6)reserve(2) offset(8)
				{
					real_in_dest += 4;
					if (destlen < real_in_dest + 1)
						return -1;
					unsigned char bit1 = 0x20;
					unsigned char bit2 = (unsigned char)len;
					unsigned char bit3 = (((unsigned char)offset) << 2) & 0xFC;
					unsigned char bit4 = (unsigned char)(offset >> 6);
					*(iterator_dest++) = bit1;
					*(iterator_dest++) = bit2;
					dest_modify_pos = iterator_dest;
					*(iterator_dest++) = bit3;
					*(iterator_dest++) = bit4;
				}					  // end if...else
				newbyte_start += len; // 下一次新字符出现的位置 更新滑动窗口
				i = i + len - 1;
				for (int i = 0; i < len; i++)
					window.Add_char_to_currentwindow(*(iterator_source + i));
			} // end if...else
		}	 // end if
		else  // 当前key空间为空，所以滑动窗口中不存在f(x,y,z)=f(a,b,c)也就找不到匹配串了
			window.Add_char_to_currentwindow(*iterator_source);
	} // end for

	iterator_source = source + i;
	for (; i < sourcelen; i++) // 余项处理
		iterator_source++;
	int local = (int)(iterator_source - newbyte_start);
	if (local > 255)
		return -1;													   // 出现连续255以上新字符的情况不压缩
	if (local == 0)													   // 0个新字符的处理方式
		*dest_modify_pos = (*(unsigned char *)dest_modify_pos) & 0xFC; // 待修正的dest处
	else if (local > 0 && local < 4)								   // 1到3个字符的处理方式
	{
		real_in_dest += local;
		if (destlen < real_in_dest + 1)
			return -1;
		*dest_modify_pos = ((*(unsigned char *)dest_modify_pos) & 0xFC) | ((unsigned char)local & 0x03);
		for (int i = 0; i < local; i++)
			*(iterator_dest++) = *(newbyte_start++);
	}
	else if (local >= 4 && local <= 18) // 4到18个字符的处理方式
	{
		real_in_dest += (local + 1);
		if (destlen < real_in_dest + 1)
			return -1;
		if (dest_modify_pos != 0)
			*dest_modify_pos = (*(unsigned char *)dest_modify_pos) & 0xFC;
		unsigned char bit = (unsigned char)(local - 3);
		*(iterator_dest++) = bit;
		for (int i = 0; i < local; i++)
			*(iterator_dest++) = *(newbyte_start++);
	}
	else // 19到255个字符的处理方式
	{
		real_in_dest += (2 + local);
		if (destlen < real_in_dest + 1)
			return -1;
		if (dest_modify_pos != 0)
			*dest_modify_pos = (*(unsigned char *)dest_modify_pos) & 0xFC;
		unsigned char bit1 = 0x00;
		unsigned char bit2 = (unsigned char)local;
		*(iterator_dest++) = bit1;
		*(iterator_dest++) = bit2;
		for (int i = 0; i < local; i++)
			*(iterator_dest++) = *(newbyte_start++);
	}

	// 加入结束标记
	real_in_dest += 2;
	if (destlen < real_in_dest + 1)
		return -1;
	*(unsigned short *)iterator_dest = 0x0000;
	iterator_dest += 2;
	*iterator_dest = 0; // 以空结尾
	return real_in_dest;
}

// 解压缩数据，
// 解压缩的过程比较简单，判断头标记来判断是哪种格式，根据压缩时的格式逐步分析数据，就可使数据还原。
int CLzoCompress::uncompress(unsigned char *dest, int destlen, unsigned char *source, int sourcelen)
{
	if (dest == 0 || destlen <= 0 || source == 0 || sourcelen <= 0)
		return -1;
	CLzoSlipWindow window;					 // 滑动窗口
	window.init_slip_window();				 // 滑动窗口初始化
	unsigned char *iterator_dest = dest;	 // 目的BUF迭代
	unsigned char *iterator_source = source; // 源BUF迭代
	int real_in_dest = 0;					 // 压缩到dest的实际字节数

	unsigned char head = 0; // 编码的头标记  根据head值来分辨是哪种格式的编码
	int i = 0;
	for (i = 0; i < sourcelen; i++) // 遍历source解码
	{
		iterator_source = source + i;
		head = *(unsigned char *)iterator_source;
		unsigned short len = 0;
		unsigned short offset = 0;
		unsigned char temp = 0;
		if (head >= 0x40) // 格式为 len(3)offset(3)reserve(2) offset(8)
		{
			temp = *(unsigned char *)iterator_source;		  // 取第一个字节
			len = (unsigned short)(temp >> 5) + 1;			  // 取len(3)
			offset += (unsigned short)((temp >> 2) & 0x07);   // 取offset(3)
			unsigned int count = (unsigned int)(temp & 0x03); // 取reserve(2)
			temp = *(unsigned char *)(iterator_source + 1);   // 取第二个字节
			offset += temp * 8;								  // 取offset(8)*8

			real_in_dest += len;
			if (destlen < real_in_dest + 1) // 检查dest的长度
				return -1;
			int j = 0;
			for (j = 0; j < len; j++)
				*(iterator_dest + j) = window.content_of_slip_window[(offset + j) % 4096]; // 从滑动窗口取出值
			for (j = 0; j < len; j++)
				window.Add_char_to_currentwindow(*(iterator_dest++)); // 更新滑动窗口
			if (count > 0)											  // 如果reserve非0 说明后面是连续count个字符
			{
				real_in_dest += count;
				if (destlen < real_in_dest + 1) // 检查dest长度
					return -1;
				iterator_source += 2; // iterator_source向后移动两个字节
				for (int j = 0; j < (int)count; j++)
				{
					window.Add_char_to_currentwindow(*iterator_source); //  更新滑动窗口
					*(iterator_dest++) = *(iterator_source++);			//  处理新字符部分
				}
			}
			i += (2 + count - 1); // 控制变量对应到下一个处理单元
		}
		else if (head >= 0x22 && head <= 0x27) // 00100len(3) offset(6)reserve(2) offset(8)
		{
			temp = *(unsigned char *)iterator_source;		  // 取第一个字节
			len = (unsigned short)(temp & 0x07) + 1;		  // 取len(3)
			temp = *(unsigned char *)(iterator_source + 1);   // 取第二个字节
			offset += (unsigned short)(temp >> 2);			  // 取offset(6)
			unsigned int count = (unsigned int)(temp & 0x03); // 取reserve(2)
			temp = *(unsigned char *)(iterator_source + 2);   // 取第三个字节
			offset += temp * 64;							  // 取offset(8)*8

			real_in_dest += len;
			if (destlen < real_in_dest + 1) // 检查dest的长度
				return -1;
			int j = 0;
			for (j = 0; j < len; j++)
				*(iterator_dest + j) = window.content_of_slip_window[(offset + j) % 4096]; // 从滑动窗口取出值
			for (j = 0; j < len; j++)
				window.Add_char_to_currentwindow(*(iterator_dest++)); // 更新滑动窗口
			if (count > 0)											  // 如果reserve非0 说明后面是连续count个字符
			{
				real_in_dest += count;
				if (destlen < real_in_dest + 1) // 检查dest长度
					return -1;
				iterator_source += 3; // iterator_source向后移动三个字节
				for (int j = 0; j < (int)count; j++)
				{
					window.Add_char_to_currentwindow(*iterator_source); // 更新滑动窗口
					*(iterator_dest++) = *(iterator_source++);			// 处理新字符部分
				}
			}
			i += (3 + count - 1); // 控制变量对应到下一个处理单元
		}
		else if (head == 0x20) // 00100000 len(8) offset(6)reserve(2) offset(8)
		{
			temp = *(unsigned char *)(iterator_source + 1);   // 取第二个字节
			len = (unsigned short)temp;						  // 取len(8)
			temp = *(unsigned char *)(iterator_source + 2);   // 取第三个字节
			offset += (unsigned short)(temp >> 2);			  // 取offset(6)
			unsigned int count = (unsigned int)(temp & 0x03); // 取reserve(2)
			temp = *(unsigned char *)(iterator_source + 3);   // 取第四个字节
			offset += temp * 64;							  // 取offset(8)

			real_in_dest += len;
			if (destlen < real_in_dest + 1) // 检查dest的长度
				return -1;
			int j = 0;
			for (j = 0; j < len; j++)
				*(iterator_dest + j) = window.content_of_slip_window[(offset + j) % 4096]; // 从滑动窗口取出值
			for (j = 0; j < len; j++)
				window.Add_char_to_currentwindow(*(iterator_dest++)); // 更新滑动窗口
			if (count > 0)											  // 如果reserve非0 说明后面是连续count个字符
			{
				real_in_dest += count;
				if (destlen < real_in_dest + 1) // 检查dest长度
					return -1;
				iterator_source += 4; // iterator_source向后移动四个字节
				for (int j = 0; j < (int)count; j++)
				{
					window.Add_char_to_currentwindow(*iterator_source); // 更新滑动窗口
					*(iterator_dest++) = *(iterator_source++);			// 处理新字符部分
				}
			}
			i += (4 + count - 1); // 控制变量对应到下一个处理单元
		}
		else if (head >= 0x01 && head <= 0x0F) // count unsigned char...unsigned char count为字符个数 区间[4，18]
		{
			temp = *(unsigned char *)iterator_source;	  // 取第一个字节
			unsigned int count = (unsigned int)(temp + 3); // 取count
			real_in_dest += count;
			if (destlen < real_in_dest + 1) // 检查dest长度
				return -1;
			iterator_source += 1; // iterator_source向后移动一个字节
			for (int j = 0; j < (int)count; j++)
			{
				window.Add_char_to_currentwindow(*iterator_source); // 更新滑动窗口
				*(iterator_dest++) = *(iterator_source++);			// 处理新字符部分
			}
			i += (1 + count - 1); // 控制变量指向下一个处理单元
		}
		else if (head == 0x00) // 0x00 count(8) unsigned char...unsigned char count为字符个数 区间[9，255]
		{
			temp = *(unsigned char *)(iterator_source + 1); // 取第二个字节
			if (temp != 0x00)								// 非0不是结束标记
			{
				unsigned int count = (unsigned int)temp; // 取新字节个数
				real_in_dest += count;
				if (destlen < real_in_dest + 1) // 检查dest长度
					return -1;
				iterator_source += 2; // iterator_source向后移动两个字节
				for (int j = 0; j < (int)count; j++)
				{
					window.Add_char_to_currentwindow(*iterator_source); // 更新滑动窗口
					*(iterator_dest++) = *(iterator_source++);			// 处理新字符部分
				}
				i += (2 + count - 1); // 控制变量指向下一个处理单元
			}
			else // 0x00结束标记
			{
				iterator_source += 2;
				*iterator_dest = 0; // 空结尾
				i += 2;
				break; // 退出for循环
			}
		}
		else // 其他格式的编码 由于这里的滑动窗口是4096所以不会有另外两种格式出现，因此在此处没有解码
		{
			//  Todo:translation for new form will be added future
		}				// end if...else
	}					// end for
	if (i == sourcelen) // source所有都被处理
		return real_in_dest;
	else // source有没有被处理的项
		return -1;
}