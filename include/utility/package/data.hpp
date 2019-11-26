#ifndef utility_include_utility_package_data_hpp
#define utility_include_utility_package_data_hpp

#include <cstddef>
#include <algorithm>
#include <queue>
#include <functional>

#include "common/common.h"

namespace diy
{
namespace utility
{
class DataImpl
{
public:
    enum
    {
        default_size = 32 * 1024, per_alloc_size = 32 * 1024
    };
    typedef std::string data_type;
    typedef std::size_t size_type;

    DataImpl()
    {
        data_.reserve(default_size);
        data_size_ = 0;
    }

    DataImpl(const char *data, size_type length)
    {
        data_.reserve((std::max)((size_type) default_size, length));
        memcpy(const_cast<char *>(data_.data()), data, length);
        data_size_ = length;
    }

    // 扩容
    void expansion()
    {
        data_.reserve(data_.capacity() + (size_type) per_alloc_size);
    }

    // 获取data_引用
    data_type &data_ref()
    {
        return data_;
    }

    const data_type &data_ref() const
    {
        return data_;
    }

    // data数据
    char *data()
    {
        return const_cast<char *>(data_.data());
    }

    const char *data() const
    {
        return data_.data();
    }

    // 获取容量
    const size_type data_capacity() const
    {
        return data_.capacity();
    }

    // 获取数据size
    const size_type data_size() const
    {
        return data_size_;
    }

    // 设置数据size
    void set_size(size_type size)
    {
        data_size_ = size;
    }

    // 移动数据到头部 从0计
    void move_to_head(size_type off_pos)
    {
        if (off_pos >= data_size_) // 移动位置在有效数据之外
        {
            data_size_ = 0;
        } else if (off_pos > 0) // 移动位置为0时不需要处理
        {
            data_size_ = data_size_ - off_pos;
            memmove(const_cast<char *>(data_.data()), data_.data() + off_pos, data_size_);
        }
    }

protected:
    data_type data_;
    size_type data_size_;
};

class DataTCP : public DataImpl
{
public:
    DataTCP() : DataImpl()
    {
    }

    DataTCP(const char *data, size_type length) : DataImpl(data, length)
    {
    }

protected:
private:
};

class DataUDP : public DataImpl
{
public:
    DataUDP() : DataImpl()
    {
    }

    DataUDP(const char *data, size_type length) : DataImpl(data, length)
    {
    }

protected:
private:
};

class DataHTTP : public DataImpl
{
public:
    struct header
    {
        std::string name;
        std::string value;
    };

    DataHTTP() : DataImpl()
    {
    }

    DataHTTP(const char *data, size_type length) : DataImpl(data, length)
    {
    }

    std::vector<header> &headers()
    {
        return headers_;
    }

    const std::vector<header> &headers() const
    {
        return headers_;
    }

    std::string &content()
    {
        return content_;
    }

    const std::string &content() const
    {
        return content_;
    }

protected:
private:
    std::vector<header> headers_;
    std::string content_;
};

class RequestParserImpl
{
public:
    typedef std::size_t size_type;

    /**
    * @param[ in] data
    * @param[out] type 解析类型
    * @return(int) 成功:解析长度 不足一个完整包:error_packet_less 解析数据异常:error_packet_bad
    */
    virtual int HandleParse(const DataImpl &data, int &type) = 0;
};

class RequestHandler
{
public:
    typedef std::size_t size_type;

    RequestHandler(const RequestHandler &) = delete;

    RequestHandler &operator=(const RequestHandler &) = delete;

    explicit RequestHandler(RequestParserImpl &parser)
            : parser_ref_(parser)
    {
    }

    /**
    * @param[ in] data
    * @param[out] type 解析类型
    * @return(int) 成功:解析长度 不足一个完整包:error_packet_less 解析数据异常:error_packet_bad
    */
    int HandleRequest(const DataImpl &data, int &type)
    {
        return parser_ref_.HandleParse(data, type);
    }

private:
    RequestParserImpl &parser_ref_;
};
}//namespace utility
}//namespace diy

#endif//!utility_include_utility_package_data_hpp