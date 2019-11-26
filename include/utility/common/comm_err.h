#ifndef utility_include_utility_common_err_h
#define utility_include_utility_common_err_h

namespace diy
{
namespace utility
{
enum error_code
{
    error_queue_full = -65535,  // 队列已满
    error_packet_less,          // 包不全
    error_packet_bad,           // 包损坏
    error_session_full,         // 会话已满
    error_session_stopped,      // 连接已关闭
    error_session_not_exist,    // 连接不存在
    error_bad_data,             // 数据错误

    success_code = 0,           // 成功
};
}//namespace utility
}//namespace diy

#endif//!utility_include_utility_common_err_h