#ifndef utility_include_utility_database_otl_otl_tools_hpp
#define utility_include_utility_database_otl_otl_tools_hpp

#include "otlv4.h"
#include "encoding/code_convert.h"

namespace diy
{
namespace utility
{

class OTLTools
{
public:
    /**
     *
     * @param _otl_stream
     * @param _field_str
     * @param _f_type 0：int,1:double,2:字符串,3：时间类型格式字符串"yyyy-mm-dd hh:mm:ss"
     * @return -1:error  >=0:otl_var_enum
     */
    static int GetOTLField(otl_stream &_otl_stream, std::string &_field_str, int &_f_type)
    {
        if (_otl_stream.eof())
        {
            return -1;
        }

        try
        {
            otl_var_desc *tmp_var_desc_ptr = _otl_stream.describe_next_out_var();

            int tmp_ftype = -1;
            if (tmp_var_desc_ptr!=NULL)
            {
                tmp_ftype = tmp_var_desc_ptr->ftype;
            }

            switch(tmp_ftype)
            {
            case otl_var_char:
            {
                _f_type = 2;
#ifdef OTL_ODBC_MSSQL_2008
                std::string _tmp_field_str;
                _otl_stream >> _tmp_field_str;
                if (!_tmp_field_str.empty())
                {
                    _field_str.resize(_tmp_field_str.length());
                    int convert_ret = iconv_convert("utf-8", "gbk", &_field_str[0], _field_str.length(), _tmp_field_str.c_str(), _tmp_field_str.length());
                    if (convert_ret < 0)
                    {
                        _field_str.resize(0);
                        _field_str = _tmp_field_str;
                    }
                    else
                    {
                        _field_str.resize(convert_ret);
                    }
                }
#else
                // oracle时设置环境变量ZHS16GBK即可不需要转换
                // mysql时使用libmyodbc8a.so ANSI Driver
                _otl_stream >> _field_str;
                if (_field_str == " ")
                {
                    _field_str = "";
                }
#endif
                break;
            }
            // ----------------------------modified by zhangt 20190807--------------------------
            case otl_var_varchar_long:
            case otl_var_raw_long:
            {
                _f_type = 2;
                otl_long_string _long_str;
                otl_lob_stream _lob;
                _otl_stream >> _lob;    
#ifdef OTL_ODBC_MSSQL_2008
                while(!_lob.eof())
                {
                    std::string _buff_str;
                    _lob >> _long_str;
                    if (_long_str.len() > 0)
                    {
                        _buff_str.resize(_long_str.len());
                        int convert_ret = iconv_convert("utf-8", "gbk", &_buff_str[0], _buff_str.length(), 
                            reinterpret_cast<const char*>(_long_str.v), _long_str.len());
                        if (convert_ret < 0)
                        {
                            _buff_str.resize(0);
                            _buff_str = std::string(reinterpret_cast<const char*>(_long_str.v), _long_str.len());
                        }
                        else
                        {
                            _buff_str.resize(convert_ret);
                        }
                    }
                    _field_str += _buff_str;
                }
#else
                    // oracle时设置环境变量ZHS16GBK即可不需要转换
                    // mysql时使用libmyodbc8a.so ANSI Driver
                while(!_lob.eof())
                {
                    _lob >> _long_str;
                    std::string buff_str(reinterpret_cast<const char*>(_long_str.v), _long_str.len());
                    _field_str += buff_str;
                }
                if (_field_str == " ")
                {
                    _field_str = "";
                }
#endif
                _lob.close();
                break;
            }
            // ---------------------------------------end modify--------------------------------------
            case otl_var_float:
            case otl_var_double:
            {
                _f_type = 1;
                double db_field = 0.0;
                _otl_stream >> db_field;
                _field_str = std::to_string(db_field);
                break;
            }
            case otl_var_int:
            case otl_var_unsigned_int:
            case otl_var_short:
            case otl_var_long_int:
            {
                _f_type = 0;
                int i_field = 0;
                _otl_stream >> i_field;
                _field_str = std::to_string(i_field);
                break;
            }
            case otl_var_timestamp:
            case otl_var_db2time:
            case otl_var_db2date:
            case otl_var_tz_timestamp:
            case otl_var_ltz_timestamp:
            {
                _f_type = 3;
                otl_datetime otl_dt;
                _otl_stream >> otl_dt;
                char sz_datatime[20] = {0};
                snprintf(sz_datatime, 20, "%04d-%02d-%02d %02d:%02d:%02d", otl_dt.year, otl_dt.month, otl_dt.day, otl_dt.hour, otl_dt.minute, otl_dt.second);
                _field_str = sz_datatime;
                break;
            }
            case otl_var_bigint:
            {
                _f_type = 0;
                int64_t i64_field = 0;
                _otl_stream >> i64_field;
                _field_str = std::to_string(i64_field);
                break;
            }
            default:
            {
                break;
            }
            }
            if (_otl_stream.is_null())
            {
                _field_str = "";
            }
            return tmp_ftype;
        }
        catch(const otl_exception& e)
        {
            _otl_stream.close();
            return -1;
        }
    }

    static void ResolveResultSet(otl_stream &_otl_stream, std::vector<std::vector<std::string>> &vv_records)
    {
        while (!_otl_stream.eof())
        {
            int f_type = 0;
            int desc_num = 0;
            /*otl_column_desc *column_desc_ = */_otl_stream.describe_select(desc_num);
            std::vector<std::string> v_tmp_record;
            v_tmp_record.resize(desc_num);
            for (int j = 0; j < desc_num; ++j)
            {
                GetOTLField(_otl_stream, v_tmp_record[j], f_type);
            }
            vv_records.emplace_back(v_tmp_record);
        }
    }
};

}
}

#endif//!utility_include_utility_database_otl_otl_tools_hpp