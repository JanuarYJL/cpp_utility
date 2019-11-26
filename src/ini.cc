#include "config/ini.h"
#include "file/directory.h"
#include "common/common.h"

#include <stdlib.h>
#include <string>
#include <stack>
#include <fstream>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#ifndef _snprintf
#define _snprintf(_llp, _isz, _fmt, ...) snprintf(_llp, _isz + 1, _fmt, ##__VA_ARGS__)
#endif
#ifndef MAX_PATH
const int MAX_PATH = 260;
#endif

void StringDelComment(char *szData)
{
	if (szData == 0)
	{
		return;
	}
	char *pTmp = strchr(szData, ';');
	if (pTmp)
	{
		*pTmp = 0;
	}
	// 有些配置字段需要包含#符号 del by yangjl 2019-08-28
	// pTmp = strchr(szData, '#');
	// if (pTmp)
	// {
	// 	*pTmp = 0;
	// }
}

void KeyDelNumSuffix(char *szData)
{ //删除数值后缀
	int iEndPos = strlen(szData);
	for (int i = iEndPos - 1; i >= 0; --i)
	{
		if (szData[i] >= '0' && szData[i] <= '9')
		{
			iEndPos = i;
			continue;
		}
		break;
	}
	szData[iEndPos] = '\0';
}

void StringTrim(char *szData)
{ //去掉字符数组前后的空格等字符
	int iStartPos = 0;
	int iEndPos = strlen(szData);
	for (int i = iEndPos - 1; i >= 0; --i)
	{
		if (szData[i] == ' ' || szData[i] == '\t' || szData[i] == '\r' || szData[i] == '\n')
		{
			iEndPos = i;
			continue;
		}
		break;
	}
	for (int i = 0; i < iEndPos; ++i)
	{
		if (szData[i] == ' ' || szData[i] == '\t' || szData[i] == '\r' || szData[i] == '\n')
		{
			// modify by liangsk 2018-12-19
			iStartPos = i + 1;
			// end modify
			continue;
		}
		break;
	}
	memmove(szData, &szData[iStartPos], iEndPos - iStartPos);
	szData[iEndPos - iStartPos] = '\0';
}

void StringQuote(char *szData)
{
	int iEndPos = strlen(szData) - 1;
	if ('\'' == szData[0] && '\'' == szData[iEndPos])
	{
		for (int i = 1; i < iEndPos - 1; ++i)
		{
			szData[i - 1] = szData[i];
		}
	}
	else if ('\"' == szData[0] && '\"' == szData[iEndPos])
	{
		for (int i = 1; i < iEndPos - 1; ++i)
		{
			szData[i - 1] = szData[i];
		}
	}
}

namespace diy
{
namespace utility
{
std::vector<std::string> split(const std::string &strStream, char cSeperator)
{
	std::vector<std::string> strvec;

	std::string::size_type pos1, pos2;
	pos2 = strStream.find(cSeperator);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		strvec.push_back(strStream.substr(pos1, pos2 - pos1));

		pos1 = pos2 + 1;
		pos2 = strStream.find(cSeperator, pos1);
	}
	strvec.push_back(strStream.substr(pos1));
	return strvec;
}

CBasicConfig::CBasicConfig(void)
{
}

CBasicConfig::~CBasicConfig(void)
{
}

void CBasicConfig::ClearConfig()
{
	m_mapConfigDB.clear();
}

CConfigWriter::CConfigWriter(void)
{
}

CConfigWriter::~CConfigWriter(void)
{
}

// 设置文件保存路径
bool CConfigWriter::SetFilePath(const std::string &strFile)
{
	m_strFilePath = strFile;
	return true;
}

// 保存文件，可以指定文件保存路径，不指定则使用setfilepath指定的值
void CConfigWriter::SaveFile(const std::string &strFile)
{
	if (m_mapConfigDB.size() <= 0)
	{
		return;
	}
	std::string strFilePath = m_strFilePath;
	if (strFile != "")
		strFilePath = strFile;
	FILE *fp = fopen(strFilePath.c_str(), "w");
	if (!fp)
	{
		return;
	}
	// 写入文件注释
	std::vector<std::string> vComment = split(m_strFileComment, '\n');
	for (int i = 0, ii = vComment.size(); i < ii; ++i)
	{
		fwrite(";", sizeof(char), strlen(";"), fp);
		fwrite(vComment[i].c_str(), sizeof(char), vComment[i].length(), fp);
		fwrite("\n", sizeof(char), strlen("\n"), fp);
	}

	MAP_CONFIG_DB::iterator itTmp1 = m_mapConfigDB.begin();
	for (; itTmp1 != m_mapConfigDB.end(); ++itTmp1)
	{
		std::string strSection = itTmp1->first;
		fwrite("\n", sizeof(char), strlen("\n"), fp);
		fwrite("[", sizeof(char), strlen("["), fp);
		fwrite(strSection.c_str(), sizeof(char), strSection.length(), fp);
		fwrite("]", sizeof(char), strlen("]"), fp);
		fwrite("\n", sizeof(char), strlen("\n"), fp);
		MAP_KEY_VAL::iterator itTmp2 = itTmp1->second.begin();
		for (; itTmp2 != itTmp1->second.end(); ++itTmp2)
		{
			std::string strKey = itTmp2->first;
			for (int i = 0, ii = itTmp2->second.size(); i < ii; ++i)
			{
				fwrite(strKey.c_str(), sizeof(char), strKey.length(), fp);
				fwrite("=", sizeof(char), strlen("="), fp);
				fwrite(itTmp2->second[i].c_str(), sizeof(char), itTmp2->second[i].length(), fp);
				fwrite("\n", sizeof(char), strlen("\n"), fp);
			}
		}
	}
	fclose(fp);
}

// 增加当问文件头注释
void CConfigWriter::AddFileComment(const std::string &strComment)
{
	m_strFileComment += "\n";
	m_strFileComment += strComment;
}

//======add value:给当前section:key增加一个值======
// string::string
void CConfigWriter::AddValue(const std::string &strSection, const std::string &strKey, const std::string &strValue)
{
	MAP_CONFIG_DB::iterator itTmp1 = m_mapConfigDB.find(strSection);
	if (itTmp1 == m_mapConfigDB.end())
	{
		MAP_KEY_VAL tmpMKV;
		std::vector<std::string> tmpVec;
		tmpVec.push_back(strValue);
		tmpMKV[strKey] = tmpVec;
		m_mapConfigDB[strSection] = tmpMKV;
	}
	else
	{
		MAP_KEY_VAL::iterator itTmp2 = itTmp1->second.find(strKey);
		if (itTmp2 == itTmp1->second.end())
		{
			std::vector<std::string> tmpVec;
			tmpVec.push_back(strValue);
			itTmp1->second[strKey] = tmpVec;
		}
		else
		{
			itTmp2->second.push_back(strValue);
		}
	}
}

void CConfigWriter::AddValue(const std::string &strSection, const std::string &strKey, const int iValue)
{
	char szTmp[64];
	memset(szTmp, 0, 64);
	_snprintf(szTmp, 63, "%d", iValue);
	AddValue(strSection, strKey, szTmp);
}

void CConfigWriter::AddValue(const std::string &strSection, const std::string &strKey, const float fValue)
{
	char szTmp[64];
	memset(szTmp, 0, 64);
	_snprintf(szTmp, 63, "%f", fValue);
	AddValue(strSection, strKey, szTmp);
}

void CConfigWriter::AddValue(const std::string &strSection, const std::string &strKey, const double dValue)
{
	char szTmp[64];
	memset(szTmp, 0, 64);
	_snprintf(szTmp, 63, "%lf", dValue);
	AddValue(strSection, strKey, szTmp);
}

// string::int
void CConfigWriter::AddValue(const std::string &strSection, const int iKey, const std::string &strValue)
{
	char szKey[64];
	memset(szKey, 0, 64);
	_snprintf(szKey, 63, "%d", iKey);
	AddValue(strSection, szKey, strValue);
}

void CConfigWriter::AddValue(const std::string &strSection, const int iKey, const int iValue)
{
	char szKey[64];
	memset(szKey, 0, 64);
	_snprintf(szKey, 63, "%d", iKey);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%d", iValue);

	AddValue(strSection, szKey, szVal);
}

void CConfigWriter::AddValue(const std::string &strSection, const int iKey, const float fValue)
{
	char szKey[64];
	memset(szKey, 0, 64);
	_snprintf(szKey, 63, "%d", iKey);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%f", fValue);

	AddValue(strSection, szKey, szVal);
}
void CConfigWriter::AddValue(const std::string &strSection, const int iKey, const double dValue)
{
	char szKey[64];
	memset(szKey, 0, 64);
	_snprintf(szKey, 63, "%d", iKey);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%lf", dValue);

	AddValue(strSection, szKey, szVal);
}

// int::string
void CConfigWriter::AddValue(const int iSection, const std::string &strKey, const std::string &strValue)
{
	char szSection[64];
	memset(szSection, 0, 64);
	_snprintf(szSection, 63, "%d", iSection);

	AddValue(szSection, strKey, strValue);
}
void CConfigWriter::AddValue(const int iSection, const std::string &strKey, const int iValue)
{
	char szSection[64];
	memset(szSection, 0, 64);
	_snprintf(szSection, 63, "%d", iSection);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%d", iValue);

	AddValue(szSection, strKey, szVal);
}
void CConfigWriter::AddValue(const int iSection, const std::string &strKey, const float fValue)
{
	char szSection[64];
	memset(szSection, 0, 64);
	_snprintf(szSection, 63, "%d", iSection);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%f", fValue);

	AddValue(szSection, strKey, szVal);
}
void CConfigWriter::AddValue(const int iSection, const std::string &strKey, const double dValue)
{
	char szSection[64];
	memset(szSection, 0, 64);
	_snprintf(szSection, 63, "%d", iSection);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%lf", dValue);

	AddValue(szSection, strKey, szVal);
}

// int::int
void CConfigWriter::AddValue(const int iSection, const int iKey, const std::string &strValue)
{
	char szSection[64];
	memset(szSection, 0, 64);
	_snprintf(szSection, 63, "%d", iSection);

	char szKey[64];
	memset(szKey, 0, 64);
	_snprintf(szKey, 63, "%d", iKey);

	AddValue(szSection, szKey, strValue);
}
void CConfigWriter::AddValue(const int iSection, const int iKey, const int iValue)
{
	char szSection[64];
	memset(szSection, 0, 64);
	_snprintf(szSection, 63, "%d", iSection);

	char szKey[64];
	memset(szKey, 0, 64);
	_snprintf(szKey, 63, "%d", iKey);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%d", iValue);

	AddValue(szSection, szKey, szVal);
}
void CConfigWriter::AddValue(const int iSection, const int iKey, const float fValue)
{
	char szSection[64];
	memset(szSection, 0, 64);
	_snprintf(szSection, 63, "%d", iSection);

	char szKey[64];
	memset(szKey, 0, 64);
	_snprintf(szKey, 63, "%d", iKey);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%f", fValue);

	AddValue(szSection, szKey, szVal);
}
void CConfigWriter::AddValue(const int iSection, const int iKey, const double dValue)
{
	char szSection[64];
	memset(szSection, 0, 64);
	_snprintf(szSection, 63, "%d", iSection);

	char szKey[64];
	memset(szKey, 0, 64);
	_snprintf(szKey, 63, "%d", iKey);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%lf", dValue);

	AddValue(szSection, szKey, szVal);
}

//======set value: 无论当前section:key有多少值，重置为一个值======
// string::string
void CConfigWriter::SetValue(const std::string &strSection, const std::string &strKey, const std::string &strValue)
{
	MAP_CONFIG_DB::iterator itTmp1 = m_mapConfigDB.find(strSection);
	if (itTmp1 == m_mapConfigDB.end())
	{
		MAP_KEY_VAL tmpMKV;
		std::vector<std::string> tmpVec;
		tmpVec.push_back(strValue);
		tmpMKV[strKey] = tmpVec;
		m_mapConfigDB[strSection] = tmpMKV;
	}
	else
	{
		MAP_KEY_VAL::iterator itTmp2 = itTmp1->second.find(strKey);
		if (itTmp2 == itTmp1->second.end())
		{
			std::vector<std::string> tmpVec;
			tmpVec.push_back(strValue);
			itTmp1->second[strKey] = tmpVec;
		}
		else
		{
			itTmp2->second.clear(); // 比add value增加一个清空当前数组的过程
			itTmp2->second.push_back(strValue);
		}
	}
}

void CConfigWriter::SetValue(const std::string &strSection, const std::string &strKey, const int iValue)
{
	char szTmp[64];
	memset(szTmp, 0, 64);
	_snprintf(szTmp, 63, "%d", iValue);
	SetValue(strSection, strKey, szTmp);
}

void CConfigWriter::SetValue(const std::string &strSection, const std::string &strKey, const float fValue)
{
	char szTmp[64];
	memset(szTmp, 0, 64);
	_snprintf(szTmp, 63, "%f", fValue);
	SetValue(strSection, strKey, szTmp);
}

void CConfigWriter::SetValue(const std::string &strSection, const std::string &strKey, const double dValue)
{
	char szTmp[64];
	memset(szTmp, 0, 64);
	_snprintf(szTmp, 63, "%lf", dValue);
	SetValue(strSection, strKey, szTmp);
}

// string::int
void CConfigWriter::SetValue(const std::string &strSection, const int iKey, const std::string &strValue)
{
	char szKey[64];
	memset(szKey, 0, 64);
	_snprintf(szKey, 63, "%d", iKey);
	SetValue(strSection, szKey, strValue);
}

void CConfigWriter::SetValue(const std::string &strSection, const int iKey, const int iValue)
{
	char szKey[64];
	memset(szKey, 0, 64);
	_snprintf(szKey, 63, "%d", iKey);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%d", iValue);

	SetValue(strSection, szKey, szVal);
}

void CConfigWriter::SetValue(const std::string &strSection, const int iKey, const float fValue)
{
	char szKey[64];
	memset(szKey, 0, 64);
	_snprintf(szKey, 63, "%d", iKey);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%f", fValue);

	SetValue(strSection, szKey, szVal);
}
void CConfigWriter::SetValue(const std::string &strSection, const int iKey, const double dValue)
{
	char szKey[64];
	memset(szKey, 0, 64);
	_snprintf(szKey, 63, "%d", iKey);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%lf", dValue);

	SetValue(strSection, szKey, szVal);
}

// int::string
void CConfigWriter::SetValue(const int iSection, const std::string &strKey, const std::string &strValue)
{
	char szSection[64];
	memset(szSection, 0, 64);
	_snprintf(szSection, 63, "%d", iSection);

	SetValue(szSection, strKey, strValue);
}
void CConfigWriter::SetValue(const int iSection, const std::string &strKey, const int iValue)
{
	char szSection[64];
	memset(szSection, 0, 64);
	_snprintf(szSection, 63, "%d", iSection);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%d", iValue);

	SetValue(szSection, strKey, szVal);
}
void CConfigWriter::SetValue(const int iSection, const std::string &strKey, const float fValue)
{
	char szSection[64];
	memset(szSection, 0, 64);
	_snprintf(szSection, 63, "%d", iSection);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%f", fValue);

	SetValue(szSection, strKey, szVal);
}
void CConfigWriter::SetValue(const int iSection, const std::string &strKey, const double dValue)
{
	char szSection[64];
	memset(szSection, 0, 64);
	_snprintf(szSection, 63, "%d", iSection);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%lf", dValue);

	SetValue(szSection, strKey, szVal);
}

// int::int
void CConfigWriter::SetValue(const int iSection, const int iKey, const std::string &strValue)
{
	char szSection[64];
	memset(szSection, 0, 64);
	_snprintf(szSection, 63, "%d", iSection);

	char szKey[64];
	memset(szKey, 0, 64);
	_snprintf(szKey, 63, "%d", iKey);

	SetValue(szSection, szKey, strValue);
}
void CConfigWriter::SetValue(const int iSection, const int iKey, const int iValue)
{
	char szSection[64];
	memset(szSection, 0, 64);
	_snprintf(szSection, 63, "%d", iSection);

	char szKey[64];
	memset(szKey, 0, 64);
	_snprintf(szKey, 63, "%d", iKey);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%d", iValue);

	SetValue(szSection, szKey, szVal);
}
void CConfigWriter::SetValue(const int iSection, const int iKey, const float fValue)
{
	char szSection[64];
	memset(szSection, 0, 64);
	_snprintf(szSection, 63, "%d", iSection);

	char szKey[64];
	memset(szKey, 0, 64);
	_snprintf(szKey, 63, "%d", iKey);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%f", fValue);

	SetValue(szSection, szKey, szVal);
}
void CConfigWriter::SetValue(const int iSection, const int iKey, const double dValue)
{
	char szSection[64];
	memset(szSection, 0, 64);
	_snprintf(szSection, 63, "%d", iSection);

	char szKey[64];
	memset(szKey, 0, 64);
	_snprintf(szKey, 63, "%d", iKey);

	char szVal[64];
	memset(szVal, 0, 64);
	_snprintf(szVal, 63, "%lf", dValue);

	SetValue(szSection, szKey, szVal);
}

CConfigLoader::CConfigLoader()
{
	//std::locale::global(std::locale(""));
}

CConfigLoader::~CConfigLoader()
{
}

void CConfigLoader::AddConfig(MAP_CONFIG_DB &inMapDB)
{
	// procedure
	// section->{key-><vector>}
	// insert a section map
	MAP_CONFIG_DB::iterator itAddDB = inMapDB.begin();
	MAP_CONFIG_DB::iterator itOrigin;
	for (; itAddDB != inMapDB.end(); ++itAddDB)
	{
		itOrigin = m_mapConfigDB.find(itAddDB->first);
		if (itOrigin == m_mapConfigDB.end())
		{
			m_mapConfigDB[itAddDB->first] = itAddDB->second;
		}
		else
		{
			// conflict->do nothing
		}
	}
}

void CConfigLoader::LoadConfig(bool bDelNumericSuffix)
{
	char szDirect[MAX_PATH];
	memset(szDirect, 0, MAX_PATH);
#ifdef _MSC_VER
	::GetModuleFileName(0, szDirect, MAX_PATH);
	char *pTmp = strrchr(szDirect, '\\');
	if (pTmp)
	{
		*pTmp = 0;
	}
#else
	//modified by yangjl 20180206//
	//getcwd(szDirect, MAX_PATH - 1);
	readlink("/proc/self/exe", szDirect, MAX_PATH);
	char *pTmp = strrchr(szDirect, '/');
	if (pTmp)
	{
		*pTmp = 0;
	}
	////////////end add////////////
#endif
	std::string strCWD = szDirect;
	std::vector<std::string> vDirs;
	vDirs.push_back(strCWD + "\\config");
	//    vDirs.push_back(strCWD + "\\Config");
	//    vDirs.push_back(strCWD + "\\CONFIG");
	vDirs.push_back(strCWD + "\\conf");
	//    vDirs.push_back(strCWD + "\\Conf");
	//    vDirs.push_back(strCWD + "\\CONF");
	for (int i = 0, ii = (int)vDirs.size(); i < ii; ++i)
	{
		std::vector<file_info_t> vFiles =
			GoThroughDirectory(vDirs[i], true, true, false);
		for (int j = 0, jj = (int)vFiles.size(); j < jj; ++j)
		{
			if (vFiles[j].str_file_name.find_last_of(".INI") == vFiles[j].str_file_name.length() - 1 ||
				vFiles[j].str_file_name.find_last_of(".ini") == vFiles[j].str_file_name.length() - 1)
			{
				std::string strFullPath = vDirs[i] + dir_seperator_ + vFiles[j].str_file_name;
				LoadIniConfig(strFullPath, bDelNumericSuffix);
			}
		}
	}
}

void CConfigLoader::LoadIniConfig(const std::string &strFile, bool bDelNumericSuffix)
{
	std::ifstream inFile;
	inFile.open(strFile.c_str(), std::ios::in);
	if (inFile.is_open())
	{
		MAP_CONFIG_DB tmpMapConfig;
		MAP_KEY_VAL tmpMapKeyVal;
		char szLine[4096];
		std::string strCurrentSection;
		while (!inFile.eof() && inFile.getline(szLine, 4095))
		{							  //读一行
			StringTrim(szLine);		  //去掉字符数组前后的空格等字符
			StringDelComment(szLine); //去注释
			if (strlen(szLine) == 0)
			{
				continue;
			}
			if ('[' == szLine[0])
			{
				char *pSec = strrchr(szLine, ']');
				if (pSec)
				{
					*pSec = 0;
					if (strlen(szLine) <= 1)
					{
						continue;
					}
					if (strCurrentSection.length() > 0)
					{
						tmpMapConfig[strCurrentSection] = tmpMapKeyVal;
						tmpMapKeyVal.clear();
					}
					strCurrentSection = szLine + 1;
				}
			}
			else
			{
				if (strCurrentSection.length() == 0)
				{
					continue;
				}
				char *pTmpEqual = strchr(szLine, '=');
				if (pTmpEqual)
				{
					*pTmpEqual = 0;
					StringTrim(szLine);
					StringTrim(pTmpEqual + 1);
					////add by wangj 20180416////
					// 如果是被 "" 包起来的数据，去掉收尾的 ""
					StringQuote(pTmpEqual + 1);
					///////////end add///////////
					if (bDelNumericSuffix)
						KeyDelNumSuffix(szLine);
					std::string strKey = szLine;		//字段名
					std::string strVal = pTmpEqual + 1; //数值
					tmpMapKeyVal[strKey].push_back(strVal);
				}
			}
		}
		inFile.close();
		if (strCurrentSection.length() > 0)
		{
			tmpMapConfig[strCurrentSection] = tmpMapKeyVal;
		}
		AddConfig(tmpMapConfig);
	}
}

////add by wangj 20180126////
// 通过指定分隔符的方式加载配置文件
void CConfigLoader::LoadIniConfigWithSpecifySeperator(const std::string &strFile, char chSeperator, bool bDelNumericSuffix)
{
	std::ifstream inFile;
	inFile.open(strFile.c_str(), std::ios::in);
	if (inFile.is_open())
	{
		MAP_CONFIG_DB tmpMapConfig;
		MAP_KEY_VAL tmpMapKeyVal;
		char szLine[4096];
		std::string strCurrentSection;
		while (!inFile.eof() && inFile.getline(szLine, 4095))
		{
			StringTrim(szLine);
			StringDelComment(szLine);
			if (strlen(szLine) == 0)
			{
				continue;
			}
			if ('[' == szLine[0])
			{
				char *pSec = strrchr(szLine, ']');
				if (pSec)
				{
					*pSec = 0;
					if (strlen(szLine) <= 1)
					{
						continue;
					}
					if (strCurrentSection.length() > 0)
					{
						tmpMapConfig[strCurrentSection] = tmpMapKeyVal;
						tmpMapKeyVal.clear();
					}
					strCurrentSection = szLine + 1;
				}
			}
			else
			{
				if (strCurrentSection.length() == 0)
				{
					continue;
				}
				char *pTmpEqual = strchr(szLine, chSeperator);
				if (pTmpEqual)
				{
					*pTmpEqual = 0;
					StringTrim(szLine);
					StringTrim(pTmpEqual + 1);
					////add by wagj 20180406////
					StringQuote(pTmpEqual + 1);
					///////////end add//////////
					if (bDelNumericSuffix)
						KeyDelNumSuffix(szLine);
					std::string strKey = szLine;
					std::string strVal = pTmpEqual + 1;
					tmpMapKeyVal[strKey].push_back(strVal);
				}
			}
		}
		inFile.close();
		if (strCurrentSection.length() > 0)
		{
			tmpMapConfig[strCurrentSection] = tmpMapKeyVal;
		}
		AddConfig(tmpMapConfig);
	}
}
///////////end add///////////

////add by wangj 20180122////
// load a whole directory, no recursion
void CConfigLoader::LoadDirectory(const std::string &strDirectory, bool bDelNumericSuffix)
{
	std::vector<file_info_t> vFiles = GoThroughDirectory(strDirectory, false, true, false);
	for (int i = 0, ii = (int)vFiles.size(); i < ii; ++i)
	{
		if (vFiles[i].str_file_name.find_last_of(".ini") == vFiles[i].str_file_name.length() - 1 ||
			vFiles[i].str_file_name.find_last_of(".INI") == vFiles[i].str_file_name.length() - 1)
		{
			std::string strFullPath = strDirectory + dir_seperator_ + vFiles[i].str_file_name;
			LoadIniConfig(strFullPath, bDelNumericSuffix);
		}
	}
}
///////////end add///////////

std::string CConfigLoader::GetValueString(const std::string &strKey, const std::string &strDefault)
{
	int iPos = strKey.find('.');
	if (iPos < 0)
	{
		return strDefault;
	}
	std::string strSection = strKey.substr(0, iPos);
	std::string strKey2 = strKey.substr(iPos + 1);
	return GetSectionValueString(strSection, strKey2, strDefault);
}

bool CConfigLoader::GetValueVector(const std::string &strKey, std::vector<std::string> &vOut)
{
	int iPos = strKey.find('.');
	if (iPos < 0)
	{
		return false;
	}
	std::string strSection = strKey.substr(0, iPos);
	std::string strKey2 = strKey.substr(iPos + 1);
	return GetSectionValueVector(strSection, strKey2, vOut);
}

int CConfigLoader::GetValueInt(const std::string &strKey, const int iDefault)
{
	char szDefault[32];
	memset(szDefault, 0, 32);
	_snprintf(szDefault, 31, "%d", iDefault);
	return atoi(GetValueString(strKey, szDefault).c_str());
}

bool CConfigLoader::GetValueIntVector(const std::string &strKey, std::vector<int> &vOut)
{
	std::vector<std::string> vTmp;
	if (GetValueVector(strKey, vTmp))
	{
		for (int i = 0, ii = vTmp.size(); i < ii; ++i)
		{
			vOut.push_back(atoi(vTmp[i].c_str()));
		}
		return true;
	}
	return false;
}

std::string CConfigLoader::GetSectionValueString(const std::string &strSection, const std::string &strKey, const std::string &strDefault)
{
	MAP_CONFIG_DB::iterator itDB = m_mapConfigDB.find(strSection);
	if (itDB == m_mapConfigDB.end())
	{
		return strDefault;
	}
	MAP_KEY_VAL::iterator itKeyVal = itDB->second.find(strKey);
	if (itKeyVal == itDB->second.end())
	{
		return strDefault;
	}
	if (itKeyVal->second.size() == 0)
	{
		return strDefault;
	}
	std::string strRtn;
	strRtn = itKeyVal->second[0];
	return strRtn;
}

bool CConfigLoader::GetSectionValueVector(const std::string &strSection, const std::string &strKey, std::vector<std::string> &vOut)
{
	MAP_CONFIG_DB::iterator itDB = m_mapConfigDB.find(strSection);
	if (itDB == m_mapConfigDB.end())
	{
		return false;
	}
	MAP_KEY_VAL::iterator itKeyVal = itDB->second.find(strKey);
	if (itKeyVal == itDB->second.end())
	{
		return false;
	}
	if (itKeyVal->second.size() == 0)
	{
		return false;
	}
	vOut = itKeyVal->second;
	return true;
}

int CConfigLoader::GetSectionValueInt(const std::string &strSection, const std::string &strKey, const int iDefault)
{
	char szDefault[32];
	memset(szDefault, 0, 32);
	_snprintf(szDefault, 31, "%d", iDefault);
	return atoi(GetSectionValueString(strSection, strKey, szDefault).c_str());
}

bool CConfigLoader::GetSectionValueIntVector(const std::string &strSection, const std::string &strKey, std::vector<int> &vOut)
{
	std::vector<std::string> vTmp;
	if (GetSectionValueVector(strSection, strKey, vTmp))
	{
		for (int i = 0, ii = vTmp.size(); i < ii; ++i)
		{
			vOut.push_back(atoi(vTmp[i].c_str()));
		}
		return true;
	}
	return false;
}

////add by wangj 20180123////
float CConfigLoader::GetSectionValueFloat(const std::string &strSection, const std::string &strKey, const float fDefault)
{
	char szDefault[32];
	memset(szDefault, 0, 32);
	_snprintf(szDefault, 31, "%f", fDefault);
	return (float)atof(GetSectionValueString(strSection, strKey, szDefault).c_str());
}

bool CConfigLoader::GetSectionValueFloatVector(const std::string &strSection, const std::string &strKey, std::vector<float> &vOut)
{
	std::vector<std::string> vTmp;
	if (GetSectionValueVector(strSection, strKey, vTmp))
	{
		for (int i = 0, ii = vTmp.size(); i < ii; ++i)
		{
			vOut.push_back((float)atof(vTmp[i].c_str()));
		}
		return true;
	}
	return false;
}
///////////end add///////////

////add by wangj 20180201////
double CConfigLoader::GetSectionValueDouble(const std::string &strSection, const std::string &strKey, const double dDefault)
{
	char szDefault[32];
	memset(szDefault, 0, 32);
	_snprintf(szDefault, 31, "%f", dDefault);
	return atof(GetSectionValueString(strSection, strKey, szDefault).c_str());
}

bool CConfigLoader::GetSectionValueDoubleVector(const std::string &strSection, const std::string &strKey, std::vector<double> &vOut)
{
	std::vector<std::string> vTmp;
	if (GetSectionValueVector(strSection, strKey, vTmp))
	{
		for (int i = 0, ii = vTmp.size(); i < ii; ++i)
		{
			vOut.push_back(atof(vTmp[i].c_str()));
		}
		return true;
	}
	return false;
}
///////////end add///////////

////add by wangj 20171211////
// get the sections vector
void CConfigLoader::GetSectionsVector(std::vector<std::string> &vOut)
{
	vOut.clear();
	MAP_CONFIG_DB::iterator itTmp = m_mapConfigDB.begin();
	for (; itTmp != m_mapConfigDB.end(); ++itTmp)
	{
		vOut.push_back(itTmp->first);
	}
}

// get the keys vector of a section
bool CConfigLoader::GetKeysVector(const std::string &strSection, std::vector<std::string> &vOut)
{
	vOut.clear();
	bool bRtn = false;
	MAP_CONFIG_DB::iterator itTmp = m_mapConfigDB.find(strSection);
	if (itTmp != m_mapConfigDB.end())
	{
		bRtn = true;
		MAP_KEY_VAL::iterator itTmp2 = itTmp->second.begin();
		for (; itTmp2 != itTmp->second.end(); ++itTmp2)
		{
			vOut.push_back(itTmp2->first);
		}
	}
	return bRtn;
}
///////////end add///////////

CConfigManager::CConfigManager(void)
{
}

CConfigManager::~CConfigManager(void)
{
}

void LoadDirectoryConfig(CConfigLoader *pConfig, const char *szDirectoryPath, bool bDelNumericSuffix)
{
	if (!pConfig || !szDirectoryPath || strlen(szDirectoryPath) == 0)
	{
		return;
	}
	std::string strDirectoryPath = szDirectoryPath;
	std::vector<file_info_t> vFiles = GoThroughDirectory(strDirectoryPath, false, true, false);
	for (int i = 0, ii = (int)vFiles.size(); i < ii; ++i)
	{
		if (vFiles[i].str_file_name.find_last_of(".ini") == vFiles[i].str_file_name.length() - 1 ||
			vFiles[i].str_file_name.find_last_of(".INI") == vFiles[i].str_file_name.length() - 1)
		{
			std::string strFullPath = strDirectoryPath + dir_seperator_ + vFiles[i].str_file_name;
			pConfig->LoadIniConfig(strFullPath, bDelNumericSuffix);
		}
	}
}

} //namespace utility
} //namespace diy