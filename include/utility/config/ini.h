#ifndef utility_include_utility_config_ini_h
#define utility_include_utility_config_ini_h

#include <string>
#include <map>
#include <vector>

namespace diy
{
namespace utility
{
extern std::vector<std::string> split(const std::string &strStream, char cSeperator);

class CBasicConfig
{
public:
    typedef std::map<std::string, std::vector<std::string>> MAP_KEY_VAL;
    typedef std::map<std::string, MAP_KEY_VAL> MAP_CONFIG_DB;

public:
    CBasicConfig(void);
    virtual ~CBasicConfig(void);

    virtual void ClearConfig(); // add by wangj 20180428

public:
    MAP_CONFIG_DB m_mapConfigDB;
};

class CConfigWriter : virtual public CBasicConfig
{
public:
    CConfigWriter(void);
    ~CConfigWriter(void);

public:
    // 设置文件保存路径
    bool SetFilePath(const std::string &strFile);

    // 保存文件，可以指定文件保存路径，不指定则使用setfilepath指定的值
    void SaveFile(const std::string &strFile = "");

    // 重置当前文件头注释
    void SetFileComment(const std::string &strComment);
    // 增加当问文件头注释
    void AddFileComment(const std::string &strComment);

    //======add value:给当前section:key增加一个值======
    // string::string
    void AddValue(const std::string &strSection, const std::string &strKey, const std::string &strValue);
    void AddValue(const std::string &strSection, const std::string &strKey, const int iValue);
    void AddValue(const std::string &strSection, const std::string &strKey, const float fValue);
    void AddValue(const std::string &strSection, const std::string &strKey, const double dValue);

    // string::int
    void AddValue(const std::string &strSection, const int iKey, const std::string &strValue);
    void AddValue(const std::string &strSection, const int iKey, const int iValue);
    void AddValue(const std::string &strSection, const int iKey, const float fValue);
    void AddValue(const std::string &strSection, const int iKey, const double dValue);

    // int::string
    void AddValue(const int iSection, const std::string &strKey, const std::string &strValue);
    void AddValue(const int iSection, const std::string &strKey, const int iValue);
    void AddValue(const int iSection, const std::string &strKey, const float fValue);
    void AddValue(const int iSection, const std::string &strKey, const double dValue);

    // int::int
    void AddValue(const int iSection, const int iKey, const std::string &strValue);
    void AddValue(const int iSection, const int iKey, const int iValue);
    void AddValue(const int iSection, const int iKey, const float fValue);
    void AddValue(const int iSection, const int iKey, const double dValue);

    //======set value: 无论当前section:key有多少值，重置为一个值======
    // string::string
    void SetValue(const std::string &strSection, const std::string &strKey, const std::string &strValue);
    void SetValue(const std::string &strSection, const std::string &strKey, const int iValue);
    void SetValue(const std::string &strSection, const std::string &strKey, const float fValue);
    void SetValue(const std::string &strSection, const std::string &strKey, const double dValue);

    // string::int
    void SetValue(const std::string &strSection, const int iKey, const std::string &strValue);
    void SetValue(const std::string &strSection, const int iKey, const int iValue);
    void SetValue(const std::string &strSection, const int iKey, const float fValue);
    void SetValue(const std::string &strSection, const int iKey, const double dValue);

    // int::string
    void SetValue(const int iSection, const std::string &strKey, const std::string &strValue);
    void SetValue(const int iSection, const std::string &strKey, const int iValue);
    void SetValue(const int iSection, const std::string &strKey, const float fValue);
    void SetValue(const int iSection, const std::string &strKey, const double dValue);

    // int::int
    void SetValue(const int iSection, const int iKey, const std::string &strValue);
    void SetValue(const int iSection, const int iKey, const int iValue);
    void SetValue(const int iSection, const int iKey, const float fValue);
    void SetValue(const int iSection, const int iKey, const double dValue);

private:
    std::string m_strFilePath; // 文件保存路径

    std::string m_strFileComment; // 文件注释
};

class CConfigLoader : virtual public CBasicConfig
{
public:
    CConfigLoader();
    ~CConfigLoader();

public:
    std::string GetValueString(const std::string &strKey, const std::string &strDefault = "");
    bool GetValueVector(const std::string &strKey, std::vector<std::string> &vOut);

    int GetValueInt(const std::string &strKey, const int iDefault = 0);
    bool GetValueIntVector(const std::string &strKey, std::vector<int> &vOut);

    // 支持section key两个关键字参数
    std::string GetSectionValueString(const std::string &strSection, const std::string &strKey, const std::string &strDefault = "");
    bool GetSectionValueVector(const std::string &strSection, const std::string &strKey, std::vector<std::string> &vOut);

    int GetSectionValueInt(const std::string &strSection, const std::string &strKey, const int iDefault = 0);
    bool GetSectionValueIntVector(const std::string &strSection, const std::string &strKey, std::vector<int> &vOut);

    ////add by wangj 20180123////
    float GetSectionValueFloat(const std::string &strSection, const std::string &strKey, const float fDefault = 0.0f);
    bool GetSectionValueFloatVector(const std::string &strSection, const std::string &strKey, std::vector<float> &vOut);
    ///////////end add///////////

    ////add by wangj 20180201////
    double GetSectionValueDouble(const std::string &strSection, const std::string &strKey, const double dDefault = 0.0f);
    bool GetSectionValueDoubleVector(const std::string &strSection, const std::string &strKey, std::vector<double> &vOut);
    ///////////end add///////////

    ////add by wangj 20171211////
    // 获得所有Section数组用于遍历
    void GetSectionsVector(std::vector<std::string> &vOut);
    // 获得Section下所有Key数组用于遍历
    bool GetKeysVector(const std::string &strSection, std::vector<std::string> &vOut);
    ///////////end add///////////

    void LoadIniConfig(const std::string &strFile, bool bDelNumericSuffix = true);
    ////add by wangj 20180126////
    // 通过指定分隔符的方式加载配置文件
    void LoadIniConfigWithSpecifySeperator(const std::string &strFile, char chSeperator, bool bDelNumericSuffix = true);
    ///////////end add///////////

    ////add by wangj 20180122////
    // 加载整个目录
    void LoadDirectory(const std::string &strDirectory, bool bDelNumericSuffix = true);
    ///////////end add///////////

protected:
    void AddConfig(MAP_CONFIG_DB &inMapDB);
    void LoadConfig(bool bDelNumericSuffix = true);
};

class CConfigManager : public CConfigLoader,
                       public CConfigWriter
{
public:
    CConfigManager(void);
    ~CConfigManager(void);
};

// 加载整个目录的配置
extern void LoadDirectoryConfig(CConfigLoader *pConfig, const char *szDirectoryPath, bool bDelNumericSuffix = true);
} //namespace utility
} //namespace diy
#endif //utility_include_utility_config_ini_h
