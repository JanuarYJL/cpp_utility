#include "session/http/request.h"

namespace status_strings
{

// 状态描述
const std::string ok = "HTTP/1.0 200 OK\r\n";
const std::string created = "HTTP/1.0 201 Created\r\n";
const std::string accepted = "HTTP/1.0 202 Accepted\r\n";
const std::string no_content = "HTTP/1.0 204 No Content\r\n";
const std::string multiple_choices = "HTTP/1.0 300 Multiple Choices\r\n";
const std::string moved_permanently = "HTTP/1.0 301 Moved Permanently\r\n";
const std::string moved_temporarily = "HTTP/1.0 302 Moved Temporarily\r\n";
const std::string not_modified = "HTTP/1.0 304 Not Modified\r\n";
const std::string bad_request = "HTTP/1.0 400 Bad Request\r\n";
const std::string unauthorized = "HTTP/1.0 401 Unauthorized\r\n";
const std::string forbidden = "HTTP/1.0 403 Forbidden\r\n";
const std::string not_found = "HTTP/1.0 404 Not Found\r\n";
const std::string internal_server_error = "HTTP/1.0 500 Internal Server Error\r\n";
const std::string not_implemented = "HTTP/1.0 501 Not Implemented\r\n";
const std::string bad_gateway = "HTTP/1.0 502 Bad Gateway\r\n";
const std::string service_unavailable = "HTTP/1.0 503 Service Unavailable\r\n";

// status description to std::vector<asio::const_buffer>
asio::const_buffer to_buffer(diy::utility::HTTPReply::status_type status)
{
    switch (status)
    {
        case diy::utility::HTTPReply::ok:
            return asio::buffer(ok);
        case diy::utility::HTTPReply::created:
            return asio::buffer(created);
        case diy::utility::HTTPReply::accepted:
            return asio::buffer(accepted);
        case diy::utility::HTTPReply::no_content:
            return asio::buffer(no_content);
        case diy::utility::HTTPReply::multiple_choices:
            return asio::buffer(multiple_choices);
        case diy::utility::HTTPReply::moved_permanently:
            return asio::buffer(moved_permanently);
        case diy::utility::HTTPReply::moved_temporarily:
            return asio::buffer(moved_temporarily);
        case diy::utility::HTTPReply::not_modified:
            return asio::buffer(not_modified);
        case diy::utility::HTTPReply::bad_request:
            return asio::buffer(bad_request);
        case diy::utility::HTTPReply::unauthorized:
            return asio::buffer(unauthorized);
        case diy::utility::HTTPReply::forbidden:
            return asio::buffer(forbidden);
        case diy::utility::HTTPReply::not_found:
            return asio::buffer(not_found);
        case diy::utility::HTTPReply::internal_server_error:
            return asio::buffer(internal_server_error);
        case diy::utility::HTTPReply::not_implemented:
            return asio::buffer(not_implemented);
        case diy::utility::HTTPReply::bad_gateway:
            return asio::buffer(bad_gateway);
        case diy::utility::HTTPReply::service_unavailable:
            return asio::buffer(service_unavailable);
        default:
            return asio::buffer(internal_server_error);
    }
}

// status description to std::string
std::string to_string(diy::utility::HTTPReply::status_type status)
{
    switch (status)
    {
        case diy::utility::HTTPReply::ok:
            return ok;
        case diy::utility::HTTPReply::created:
            return created;
        case diy::utility::HTTPReply::accepted:
            return accepted;
        case diy::utility::HTTPReply::no_content:
            return no_content;
        case diy::utility::HTTPReply::multiple_choices:
            return multiple_choices;
        case diy::utility::HTTPReply::moved_permanently:
            return moved_permanently;
        case diy::utility::HTTPReply::moved_temporarily:
            return moved_temporarily;
        case diy::utility::HTTPReply::not_modified:
            return not_modified;
        case diy::utility::HTTPReply::bad_request:
            return bad_request;
        case diy::utility::HTTPReply::unauthorized:
            return unauthorized;
        case diy::utility::HTTPReply::forbidden:
            return forbidden;
        case diy::utility::HTTPReply::not_found:
            return not_found;
        case diy::utility::HTTPReply::internal_server_error:
            return internal_server_error;
        case diy::utility::HTTPReply::not_implemented:
            return not_implemented;
        case diy::utility::HTTPReply::bad_gateway:
            return bad_gateway;
        case diy::utility::HTTPReply::service_unavailable:
            return service_unavailable;
        default:
            return internal_server_error;
    }
}

}//namespace status_strings

namespace misc_strings
{

// 字段取值分隔符
const char name_value_separator[] = {':', ' '};
// 回车换行符
const char crlf[] = {'\r', '\n'};

}//namespace misc_strings

namespace stock_replies
{

// 预备应答
const char ok[] = "";
const char created[] = "<html>"
        "<head><title>Created</title></head>"
        "<body><h1>201 Created</h1></body>"
        "</html>";
const char accepted[] = "<html>"
        "<head><title>Accepted</title></head>"
        "<body><h1>202 Accepted</h1></body>"
        "</html>";
const char no_content[] = "<html>"
        "<head><title>No Content</title></head>"
        "<body><h1>204 Content</h1></body>"
        "</html>";
const char multiple_choices[] = "<html>"
        "<head><title>Multiple Choices</title></head>"
        "<body><h1>300 Multiple Choices</h1></body>"
        "</html>";
const char moved_permanently[] = "<html>"
        "<head><title>Moved Permanently</title></head>"
        "<body><h1>301 Moved Permanently</h1></body>"
        "</html>";
const char moved_temporarily[] = "<html>"
        "<head><title>Moved Temporarily</title></head>"
        "<body><h1>302 Moved Temporarily</h1></body>"
        "</html>";
const char not_modified[] = "<html>"
        "<head><title>Not Modified</title></head>"
        "<body><h1>304 Not Modified</h1></body>"
        "</html>";
const char bad_request[] = "<html>"
        "<head><title>Bad Request</title></head>"
        "<body><h1>400 Bad Request</h1></body>"
        "</html>";
const char unauthorized[] = "<html>"
        "<head><title>Unauthorized</title></head>"
        "<body><h1>401 Unauthorized</h1></body>"
        "</html>";
const char forbidden[] = "<html>"
        "<head><title>Forbidden</title></head>"
        "<body><h1>403 Forbidden</h1></body>"
        "</html>";
const char not_found[] = "<html>"
        "<head><title>Not Found</title></head>"
        "<body><h1>404 Not Found</h1></body>"
        "</html>";
const char internal_server_error[] = "<html>"
        "<head><title>Internal Server Error</title></head>"
        "<body><h1>500 Internal Server Error</h1></body>"
        "</html>";
const char not_implemented[] = "<html>"
        "<head><title>Not Implemented</title></head>"
        "<body><h1>501 Not Implemented</h1></body>"
        "</html>";
const char bad_gateway[] = "<html>"
        "<head><title>Bad Gateway</title></head>"
        "<body><h1>502 Bad Gateway</h1></body>"
        "</html>";
const char service_unavailable[] = "<html>"
        "<head><title>Service Unavailable</title></head>"
        "<body><h1>503 Service Unavailable</h1></body>"
        "</html>";

// stock reply to std::string
std::string to_string(diy::utility::HTTPReply::status_type status)
{
    switch (status)
    {
        case diy::utility::HTTPReply::ok:
            return ok;
        case diy::utility::HTTPReply::created:
            return created;
        case diy::utility::HTTPReply::accepted:
            return accepted;
        case diy::utility::HTTPReply::no_content:
            return no_content;
        case diy::utility::HTTPReply::multiple_choices:
            return multiple_choices;
        case diy::utility::HTTPReply::moved_permanently:
            return moved_permanently;
        case diy::utility::HTTPReply::moved_temporarily:
            return moved_temporarily;
        case diy::utility::HTTPReply::not_modified:
            return not_modified;
        case diy::utility::HTTPReply::bad_request:
            return bad_request;
        case diy::utility::HTTPReply::unauthorized:
            return unauthorized;
        case diy::utility::HTTPReply::forbidden:
            return forbidden;
        case diy::utility::HTTPReply::not_found:
            return not_found;
        case diy::utility::HTTPReply::internal_server_error:
            return internal_server_error;
        case diy::utility::HTTPReply::not_implemented:
            return not_implemented;
        case diy::utility::HTTPReply::bad_gateway:
            return bad_gateway;
        case diy::utility::HTTPReply::service_unavailable:
            return service_unavailable;
        default:
            return internal_server_error;
    }
}

}//namespace stock_replies


namespace diy
{
namespace utility
{
//////////////////////////////////////////////////////////////////////////
/* HTTPRequest */
//////////////////////////////////////////////////////////////////////////

HTTPRequest::result_type HTTPRequest::consume(char input)
{
    switch (state_)
    {
        case method_start:
            if (!is_char(input) || is_ctl(input) || is_tspecial(input))
            {
                return bad;
            }
            else
            {
                state_ = method;
                method_.push_back(input);
                return indeterminate;
            }
        case method:
            if (input == ' ')
            {
                state_ = uri;
                return indeterminate;
            }
            else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
            {
                return bad;
            }
            else
            {
                method_.push_back(input);
                return indeterminate;
            }
        case uri:
            if (input == ' ')
            {
                state_ = http_version_h;
                return indeterminate;
            }
            else if (is_ctl(input))
            {
                return bad;
            }
            else
            {
                uri_.push_back(input);
                return indeterminate;
            }
        case http_version_h:
            if (input == 'H')
            {
                state_ = http_version_t_1;
                return indeterminate;
            }
            else
            {
                return bad;
            }
        case http_version_t_1:
            if (input == 'T')
            {
                state_ = http_version_t_2;
                return indeterminate;
            }
            else
            {
                return bad;
            }
        case http_version_t_2:
            if (input == 'T')
            {
                state_ = http_version_p;
                return indeterminate;
            }
            else
            {
                return bad;
            }
        case http_version_p:
            if (input == 'P')
            {
                state_ = http_version_slash;
                return indeterminate;
            }
            else
            {
                return bad;
            }
        case http_version_slash:
            if (input == '/')
            {
                http_version_major_ = 0;
                http_version_minor_ = 0;
                state_ = http_version_major_start;
                return indeterminate;
            }
            else
            {
                return bad;
            }
        case http_version_major_start:
            if (is_digit(input))
            {
                http_version_major_ = http_version_major_ * 10 + input - '0';
                state_ = http_version_major;
                return indeterminate;
            }
            else
            {
                return bad;
            }
        case http_version_major:
            if (input == '.')
            {
                state_ = http_version_minor_start;
                return indeterminate;
            }
            else if (is_digit(input))
            {
                http_version_major_ = http_version_major_ * 10 + input - '0';
                return indeterminate;
            }
            else
            {
                return bad;
            }
        case http_version_minor_start:
            if (is_digit(input))
            {
                http_version_minor_ = http_version_minor_ * 10 + input - '0';
                state_ = http_version_minor;
                return indeterminate;
            }
            else
            {
                return bad;
            }
        case http_version_minor:
            if (input == '\r')
            {
                state_ = expecting_newline_1;
                return indeterminate;
            }
            else if (is_digit(input))
            {
                http_version_minor_ = http_version_minor_ * 10 + input - '0';
                return indeterminate;
            }
            else
            {
                return bad;
            }
        case expecting_newline_1:
            if (input == '\n')
            {
                state_ = header_line_start;
                return indeterminate;
            }
            else
            {
                return bad;
            }
        case header_line_start:
            if (input == '\r')
            {
                state_ = expecting_newline_3;
                return indeterminate;
            }
            else if (!headers_.empty() && (input == ' ' || input == '\t'))
            {
                state_ = header_lws;
                return indeterminate;
            }
            else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
            {
                return bad;
            }
            else
            {
                headers_.push_back(HTTPHeader());
                headers_.back().name.push_back(input);
                state_ = header_name;
                return indeterminate;
            }
        case header_lws:
            if (input == '\r')
            {
                state_ = expecting_newline_2;
                return indeterminate;
            }
            else if (input == ' ' || input == '\t')
            {
                return indeterminate;
            }
            else if (is_ctl(input))
            {
                return bad;
            }
            else
            {
                state_ = header_value;
                headers_.back().value.push_back(input);
                return indeterminate;
            }
        case header_name:
            if (input == ':')
            {
                state_ = space_before_header_value;
                return indeterminate;
            }
            else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
            {
                return bad;
            }
            else
            {
                headers_.back().name.push_back(input);
                return indeterminate;
            }
        case space_before_header_value:
            if (input == ' ')
            {
                state_ = header_value;
                return indeterminate;
            }
            else
            {
                return bad;
            }
        case header_value:
            if (input == '\r')
            {
                state_ = expecting_newline_2;
                return indeterminate;
            }
            else if (is_ctl(input))
            {
                return bad;
            }
            else
            {
                headers_.back().value.push_back(input);
                return indeterminate;
            }
        case expecting_newline_2:
            if (input == '\n')
            {
                state_ = header_line_start;
                return indeterminate;
            }
            else
            {
                return bad;
            }
        case expecting_newline_3:
            return (input == '\n') ? good : bad;
        default:
            return bad;
    }
}

bool HTTPRequest::is_char(int c)
{
    return c >= 0 && c <= 127;
}

bool HTTPRequest::is_ctl(int c)
{
    return (c >= 0 && c <= 31) || (c == 127);
}

bool HTTPRequest::is_tspecial(int c)
{
    switch (c)
    {
        case '(':
        case ')':
        case '<':
        case '>':
        case '@':
        case ',':
        case ';':
        case ':':
        case '\\':
        case '"':
        case '/':
        case '[':
        case ']':
        case '?':
        case '=':
        case '{':
        case '}':
        case ' ':
        case '\t':
            return true;
        default:
            return false;
    }
}

bool HTTPRequest::is_digit(int c)
{
    return c >= '0' && c <= '9';
}

//////////////////////////////////////////////////////////////////////////
/* HTTPReply */
//////////////////////////////////////////////////////////////////////////
std::vector<asio::const_buffer> HTTPReply::to_buffers()
{
    std::vector<asio::const_buffer> buffers;
    buffers.push_back(status_strings::to_buffer(status_));
    for (std::size_t i = 0; i < headers_.size(); ++i)
    {
        HTTPHeader &header_ref = headers_[i];
        buffers.push_back(asio::buffer(header_ref.name));
        buffers.push_back(asio::buffer(misc_strings::name_value_separator));
        buffers.push_back(asio::buffer(header_ref.value));
        buffers.push_back(asio::buffer(misc_strings::crlf));
    }
    buffers.push_back(asio::buffer(misc_strings::crlf));
    buffers.push_back(asio::buffer(content_));
    return buffers;
}

std::string HTTPReply::to_string()
{
    std::string string_buf;
    string_buf.append(status_strings::to_string(status_));
    for (const auto &header_ref : headers_)
    {
        string_buf.append(header_ref.name);
        string_buf.append(misc_strings::name_value_separator);
        string_buf.append(header_ref.value);
        string_buf.append(misc_strings::crlf);
    }
    string_buf.append(misc_strings::crlf);
    string_buf.append(content_);
    return string_buf;
}

HTTPReply HTTPReply::stock_reply(status_type status)
{
    HTTPReply rep;
    rep.status_ = status;
    rep.content_ = stock_replies::to_string(status);
    rep.headers_.resize(2);
    rep.headers_[0].name = "Content-Length";
    rep.headers_[0].value = std::to_string(rep.content_.size());
    rep.headers_[1].name = "Content-Type";
    rep.headers_[1].value = "text/html";
    return rep;
}
}//namespace util
}//namespace diy