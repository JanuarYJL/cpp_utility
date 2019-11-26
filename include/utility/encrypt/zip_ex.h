// ZipEx.h: interface for the CZipEx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZIPEX_H__3B608037_627D_4AEE_B4BB_E24D2F802D71__INCLUDED_)
#define AFX_ZIPEX_H__3B608037_627D_4AEE_B4BB_E24D2F802D71__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

///////////////////////////////
// 扩展Zip压缩类
//
// add by pcq 20091214
///////////////////////////////
// 类名:CZipEx
// 功能:对数据文件进行压缩或解压缩，提供了三种压缩解压缩算法，分别为LZ77压缩算法、LZO压缩算法、霍夫曼编码压缩算法，
//      压缩算法和过程在各压缩算法类中有详细说明。
// 用法:服务器中调用压缩算法压缩数据发送到客户端，客户端调用解压算法解压收到的数据
#include "node.h"
#include "lz77compress.h"
#include "lzo.h"

class CZipEx
{
public:
	CZipEx();
	virtual ~CZipEx();

public:
	// 用lz77和霍夫曼算法压缩数据
	// @return 压缩后数据的长度
	// @param1 [out]			保存压缩后的数据
	// @param2 [in]				保存压缩数据的长度
	// @param3 [in]				被压缩的数据
	// @param4 [in]				被压缩数据的长度
	int Zip(unsigned char *dest, int destlen, unsigned char *sour, int sourlen);

	// 用霍夫曼和lz77算法解压缩数据
	// @return 解压后数据的长度
	// @param1 [out]			保存解压后的数据
	// @param2 [in]				保存解压后数据的长度
	// @param3 [in]				被解压的数据
	// @param4 [in]				被解压数据的长度
	int UnZip(unsigned char *dest, int destlen, unsigned char *sour, int sourlen);

	// 用lz77算法压缩数据
	// @return 压缩后数据的长度
	// @param1 [out]			保存压缩后的数据
	// @param2 [in]				保存压缩数据的长度
	// @param3 [in]				被压缩的数据
	// @param4 [in]				被压缩数据的长度
	int lz77Zip(unsigned char *dest, int destlen, unsigned char *sour, int sourlen);

	// 用lz77算法解压数据
	// @return 解压后数据的长度
	// @param1 [out]			保存解压后的数据
	// @param2 [in]				保存解压数据的长度
	// @param3 [in]				被解压的数据
	// @param4 [in]				被解压数据的长度
	int lz77UnZip(unsigned char *dest, int destlen, unsigned char *sour, int sourlen);

	// 用lzo算法压缩数据
	// @return 压缩后数据的长度
	// @param1 [out]			保存压缩后的数据
	// @param2 [in]				保存压缩数据的长度
	// @param3 [in]				被压缩的数据
	// @param4 [in]				被压缩数据的长度
	int lzoZip(unsigned char *dest, int destlen, unsigned char *sour, int sourlen);

	// 用lzo算法解压数据
	// @return 解压后数据的长度
	// @param1 [out]			保存解压后的数据
	// @param2 [in]				保存解压数据的长度
	// @param3 [in]				被解压的数据
	// @param4 [in]				被解压数据的长度
	int lzoUnZip(unsigned char *dest, int destlen, unsigned char *sour, int sourlen);

public:
	//
	// 以下函数为观察压缩效果使用
	//
	float GetZipRate(void);			// 取得压缩比
	float GetZipSuccByteRate(void); // 取得成功压缩字节比
	float GetZipCountRate(void);	// 取得总压缩比

	long GetZipInByte(void);	// 取得压缩数据的总大小
	long GetZipOutByte(void);   // 取得压缩后数据的总大小
	long GetZipFalseByte(void); // 取得压缩失败数据的总大小
	long GetZipSuccByte(void);  // 取得压缩成功数据的总大小
	void ResetZipByte(void);	// 重置压缩数据

	long GetUnZipInByte(void);	// 取得解压缩数据的总大小
	long GetUnZipOutByte(void);   // 取得解压缩后数据的总大小
	long GetUnZipSuccByte(void);  // 取得解压缩成功数据的总大小
	long GetUnZipFalseByte(void); // 取得解压缩失败数据的总大小
	void ResetUnZipByte(void);	// 重置解压缩数据

private:
	long m_lZipInByte;	// 压缩数据的总大小
	long m_lZipOutByte;   // 压缩后数据的总大小
	long m_lZipSuccByte;  // 压缩成功数据的总大小
	long m_lZipFalseByte; // 压缩失败数据的总大小

	long m_lUnZipInByte;	// 解压缩数据的总大小
	long m_lUnZipOutByte;   // 解压缩后数据的总大小
	long m_lUnZipSuccByte;  // 压缩成功数据的总大小
	long m_lUnZipFalseByte; // 解压缩失败数据的总大小

private:
	Pz_lz77compress lz77;	  // lz77压缩算法类
	CLzoCompress lzo;		   // lzo压缩算法类
	PZ_HandleCompress hoffman; // 霍夫曼编码压缩算法类
};

#endif // !defined(AFX_ZIPEX_H__3B608037_627D_4AEE_B4BB_E24D2F802D71__INCLUDED_)
