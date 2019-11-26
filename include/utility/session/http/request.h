#pragma once
#ifndef utility_include_utility_session_http_request_h
#define utility_include_utility_session_http_request_h

#include "package/data.hpp"

namespace diy
{
namespace utility
{
/*
* HTTP头结构
*/
class HTTPHeader
{
public:
    std::string name;
    std::string value;
};

/**
* HTTP请求结构
*/
class HTTPRequest
{
public:
    typedef std::size_t size_type;

    enum result_type
    {
        good, bad, indeterminate
    };

    HTTPRequest()
            : http_version_major_(0)
            , http_version_minor_(0)
            , state_(method_start)
            , parse_bytes_(0)
    {
    }

    ~HTTPRequest()
    {
    }

    /**
    * 重置解析状态
    */
    void reset()
    {
        state_ = method_start;
        parse_bytes_ = 0;
    }

    /**
    * 获取已解析数据长度
    */
    size_type parse_bytes()
    {
        return parse_bytes_;
    }

    /**
    * @param begin iterator 起始迭代器
    * @param end iterator 终止迭代器
    * @return tuple<解析结果, 解析长度>
    */
    template<typename InputIterator>
    std::tuple<result_type, size_type> parse(InputIterator begin, InputIterator end)
    {
        size_type parse_count = 0;
        while (begin != end)
        {
            ++parse_count;
            result_type result = consume(*begin++);
            if (result == good || result == bad)
            {
                parse_bytes_ += parse_count;
                return std::make_tuple(result, parse_count);
            }
        }
        parse_bytes_ += parse_count;
        return std::make_tuple(indeterminate, parse_count);
    }

    std::string method_;
    std::string uri_;
    int http_version_major_;
    int http_version_minor_;
    std::vector<HTTPHeader> headers_;

private:
    // 处理输入
    result_type consume(char input);

    // 检测是否是字符
    static bool is_char(int c);

    // 检测是否是HTTP控制字符
    static bool is_ctl(int c);

    // 检测是否是HTTP特殊字符
    static bool is_tspecial(int c);

    // 检测是否是数值
    static bool is_digit(int c);

    // 当前解析状态
    enum state
    {
        method_start,
        method,
        uri,
        http_version_h,
        http_version_t_1,
        http_version_t_2,
        http_version_p,
        http_version_slash,
        http_version_major_start,
        http_version_major,
        http_version_minor_start,
        http_version_minor,
        expecting_newline_1,
        header_line_start,
        header_lws,
        header_name,
        space_before_header_value,
        header_value,
        expecting_newline_2,
        expecting_newline_3
    } state_;

    size_type parse_bytes_;
};

/**
* HTTP应答结构
*/
class HTTPReply
{
public:
    /**
    * http应答状态
    */
    enum status_type
    {
        ok = 200,
        created = 201,
        accepted = 202,
        no_content = 204,
        multiple_choices = 300,
        moved_permanently = 301,
        moved_temporarily = 302,
        not_modified = 304,
        bad_request = 400,
        unauthorized = 401,
        forbidden = 403,
        not_found = 404,
        internal_server_error = 500,
        not_implemented = 501,
        bad_gateway = 502,
        service_unavailable = 503
    } status_;

    HTTPReply() : status_(ok)
    {
    }

    ~HTTPReply()
    {
    }

    /**
    * reply -> buffers/string
    */
    std::vector<asio::const_buffer> to_buffers();

    std::string to_string();

    /**
    * 返回指定状态的预存应答
    */
    static HTTPReply stock_reply(status_type status);

    std::vector<HTTPHeader> headers_;
    std::string content_;
};
}//namespace utility
}//namespace diy
#endif//!utility_include_utility_session_http_request_h