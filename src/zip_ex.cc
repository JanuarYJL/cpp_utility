// ZipEx.cpp: implementation of the CZipEx class.
//
//////////////////////////////////////////////////////////////////////
#include <cstring>
#include "encrypt/zip_ex.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CZipEx::CZipEx()
{
	ResetZipByte();
	ResetUnZipByte();
}

CZipEx::~CZipEx()
{
}

//
// 压缩
//
int CZipEx::Zip(unsigned char *dest, int destlen, unsigned char *sour, int sourlen)
{
	m_lZipInByte += sourlen;

	// 进行lz77压缩
	int iRet = lz77.lz77Compress(dest, destlen, sour, sourlen);
	if (iRet <= 0)
	{
		m_lZipFalseByte += sourlen;
		return -1;
	}

	// 进行霍夫曼编码
	unsigned char *pTmp = NULL;

	pTmp = new unsigned char[iRet];
	memcpy(pTmp, dest, sizeof(unsigned char) * iRet);
	iRet = hoffman.Compress(dest, destlen, pTmp, iRet);
	delete[] pTmp;
	if (iRet <= 0 || iRet >= sourlen)
	{
		// 如果压缩失败或者压缩结果大于未压缩长度，则认为压缩失败
		m_lZipFalseByte += sourlen;
		return -1;
	}

	//
	// for test
	//
	/*
	// 进行lz77压缩
	int iRet = lz77.lz77Compress(dest, destlen, sour, sourlen);
	if(iRet <= 0 || iRet >= sourlen)
	{
		m_lZipFalseByte += sourlen;
		return -1;
	}
	int iRet = hoffman.Compress(dest, destlen, sour, sourlen);
	if(iRet <= 0 || iRet >= sourlen)
	{
		m_lZipFalseByte += sourlen;
		return -1;
	}
	*/

	m_lZipSuccByte += sourlen;
	m_lZipOutByte += iRet;
	return iRet;
}

//
// 解压缩
//
int CZipEx::UnZip(unsigned char *dest, int destlen, unsigned char *sour, int sourlen)
{
	m_lUnZipInByte += sourlen;

	// 进行霍夫曼编码解码
	int iRet = hoffman.UnCompress(dest, destlen, sour, sourlen);
	if (iRet <= 0)
	{
		m_lUnZipFalseByte += sourlen;
		return -1;
	}

	// 进行lz77解压缩

	unsigned char *pTmp = new unsigned char[iRet];
	memcpy(pTmp, dest, sizeof(unsigned char) * iRet);
	iRet = lz77.lz77Expand(dest, destlen, pTmp, iRet);
	delete[] pTmp;
	if (iRet <= 0)
	{
		m_lUnZipFalseByte += sourlen;
		return -1;
	}

	m_lUnZipSuccByte += sourlen;
	m_lUnZipOutByte += iRet;
	return iRet;
}

//
// lz77压缩
//
int CZipEx::lz77Zip(unsigned char *dest, int destlen, unsigned char *sour, int sourlen)
{
	m_lZipInByte += sourlen;

	// 进行lz77压缩
	int iRet = lz77.lz77Compress(dest, destlen, sour, sourlen);
	if (iRet <= 0 || iRet >= sourlen)
	{
		m_lZipFalseByte += sourlen;
		return -1;
	}

	m_lZipSuccByte += sourlen;
	m_lZipOutByte += iRet;
	return iRet;
}

//
// lz77解压缩
//
int CZipEx::lz77UnZip(unsigned char *dest, int destlen, unsigned char *sour, int sourlen)
{
	m_lUnZipInByte += sourlen;

	// 进行lz77解压缩
	int iRet = lz77.lz77Expand(dest, destlen, sour, sourlen);
	if (iRet <= 0)
	{
		m_lUnZipFalseByte += sourlen;
		return -1;
	}

	m_lUnZipSuccByte += sourlen;
	m_lUnZipOutByte += iRet;
	return iRet;
}

//
// lzo压缩
//
int CZipEx::lzoZip(unsigned char *dest, int destlen, unsigned char *sour, int sourlen)
{
	m_lZipInByte += sourlen;

	// 进行lzo压缩
	int iRet = lzo.compress(dest, destlen, sour, sourlen);
	if (iRet <= 0 || iRet >= sourlen)
	{
		m_lZipFalseByte += sourlen;
		return -1;
	}

	m_lZipSuccByte += sourlen;
	m_lZipOutByte += iRet;
	return iRet;
}

//
// lzo解压缩
//
int CZipEx::lzoUnZip(unsigned char *dest, int destlen, unsigned char *sour, int sourlen)
{
	m_lUnZipInByte += sourlen;

	// 进行lzo解压缩
	int iRet = lzo.uncompress(dest, destlen, sour, sourlen);
	if (iRet <= 0)
	{
		m_lUnZipFalseByte += sourlen;
		return -1;
	}

	m_lUnZipSuccByte += sourlen;
	m_lUnZipOutByte += iRet;
	return iRet;
}

//
// 取得压缩比
//
float CZipEx::GetZipRate(void)
{
	if (m_lZipSuccByte - m_lZipOutByte == 0)
		return 0.0f;
	else
		return (float)((float)(m_lZipSuccByte - m_lZipOutByte) / (float)m_lZipSuccByte);
}

//
// 取得成功压缩字节比
//
float CZipEx::GetZipSuccByteRate(void)
{
	if (m_lZipSuccByte == 0)
		return 0.0f;
	else
		return (float)((float)m_lZipSuccByte / (float)m_lZipInByte);
}

//
// 取得总压缩比
//
float CZipEx::GetZipCountRate(void)
{
	if (m_lZipSuccByte - m_lZipOutByte == 0)
		return 0.0f;
	else
		return (float)((float)(m_lZipSuccByte - m_lZipOutByte) / (float)m_lZipInByte);
}

//
// 取得压缩数据的总大小
//
long CZipEx::GetZipInByte(void)
{
	return m_lZipInByte;
}

//
// 取得压缩后数据的总大小
//
long CZipEx::GetZipOutByte(void)
{
	return m_lZipOutByte;
}

//
// 取得压缩成功数据的总大小
//
long CZipEx::GetZipSuccByte(void)
{
	return m_lZipSuccByte;
}

//
// 取得压缩失败数据的总大小
//
long CZipEx::GetZipFalseByte(void)
{
	return m_lZipFalseByte;
}

//
// 重置压缩数据
//
void CZipEx::ResetZipByte(void)
{
	m_lZipInByte = 0l;
	m_lZipOutByte = 0l;
	m_lZipSuccByte = 0l;
	m_lZipFalseByte = 0l;
}

//
// 取得解压缩数据的总大小
//
long CZipEx::GetUnZipInByte(void)
{
	return m_lUnZipInByte;
}

//
// 取得解压缩后数据的总大小
//
long CZipEx::GetUnZipOutByte(void)
{
	return m_lUnZipOutByte;
}

//
// 取得解压缩成功数据的总大小
//
long CZipEx::GetUnZipSuccByte(void)
{
	return m_lUnZipSuccByte;
}

//
// 取得解压缩失败数据的总大小
//
long CZipEx::GetUnZipFalseByte(void)
{
	return m_lUnZipFalseByte;
}

//
// 重置解压缩数据
//
void CZipEx::ResetUnZipByte(void)
{
	m_lUnZipInByte = 0l;
	m_lUnZipOutByte = 0l;
	m_lUnZipSuccByte = 0l;
	m_lUnZipFalseByte = 0l;
}
