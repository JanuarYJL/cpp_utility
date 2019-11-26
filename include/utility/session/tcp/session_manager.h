#pragma once
#ifndef utility_include_utility_session_tcp_session_manager_h
#define utility_include_utility_session_tcp_session_manager_h

#include "session/tcp/session_asio.h"
#include "session/tcp/acceptor_asio.h"

#include <map>
#include <tuple>
#include <thread>

namespace diy
{
namespace utility
{
class SessionMmanagerImpl : public std::enable_shared_from_this<SessionMmanagerImpl>
{
public:
    typedef std::size_t size_type;
    typedef std::recursive_mutex mutex_type;
    typedef std::condition_variable condv_type;
    typedef std::lock_guard<mutex_type> lock_guard_type;
    typedef std::thread thread_type;
    typedef std::shared_ptr<thread_type> thread_ptr_type;
    typedef std::vector<thread_ptr_type> thread_ptr_vec_type;
    typedef std::tuple<int/*threads num*/, std::vector<std::tuple<std::string/*address*/, std::string/*port*/>>> accept_conf_type;

    typedef SessionImpl::func_disconnect_cb func_disconnect_cb;
    typedef SessionImpl::func_receive_cb func_receive_cb;
    typedef SessionImpl::func_request_parser func_request_parser;

    virtual void Start() = 0;

    virtual void Stop() = 0;
};

class SessionMmanagerTCP : public SessionMmanagerImpl
{
public:
    typedef asio::ip::tcp::socket socket_type;
    typedef SessionTcpTmot session_type;
    //typedef SessionTcpImme session_type;
    typedef std::shared_ptr<session_type> session_ptr_type;
    typedef std::map<int, session_ptr_type> session_map_type;
    typedef AcceptorTCP acceptor_type;
    typedef std::shared_ptr<acceptor_type> acceptor_ptr_type;
    typedef std::vector<acceptor_ptr_type> acceptor_vec_type;

    SessionMmanagerTCP(const SessionMmanagerTCP &) = delete;

    SessionMmanagerTCP &operator=(const SessionMmanagerTCP &) = delete;

    explicit SessionMmanagerTCP(const accept_conf_type &accept_conf_vec,
            func_request_parser func_request_parser_method,
            func_disconnect_cb func_disconnect_callback,
            func_receive_cb func_receive_callback);

    void Start() override;

    void Stop() override;

    asio::io_service &get_io_service()
    {
        return io_service_;
    }

    /**
    * @param[ in] socket
    */
    virtual void AddOneSession(socket_type socket);

    /**
    * @param[ in] session_id要删除的session_id
    */
    virtual void DelOneSession(int session_id);

    /**
     * @brief 关闭指定sessionid的连接
     * @param session_id
     * @return
     */
    int CloseOneSession(int session_id);

    /**
     * @param[ in] sessionid 要发送的sessionid 为0时发送给所有session
     * @param[ in] data
     * @param[ in] length
     * @return
     */
    int AsyncSend(int sessionid, const char *data, size_type length);

protected:
    asio::io_service io_service_;
    mutex_type session_mutex_;
    int session_id_alloter_; // session id 分配器
    size_type session_max_num_; // 最大session数量
    size_type session_queue_max_size_; // session消息队列上限
    accept_conf_type accept_conf_; // accept配置信息
    acceptor_vec_type acceptor_vec_; // acceptor队列
    thread_ptr_vec_type thread_ptr_vec_; // 线程队列

    func_disconnect_cb  func_disconnect_cb_;
    func_receive_cb     func_receive_callback_;
    func_request_parser func_request_parser_;

private:
    session_map_type session_map_; // session_tcp存储map
};

class SessionMmanagerHTTP : public SessionMmanagerTCP
{
public:
    typedef SessionHTTP session_type;
    typedef std::shared_ptr<session_type> session_ptr_type;
    typedef std::map<int, session_ptr_type> session_map_type;

    SessionMmanagerHTTP(const SessionMmanagerHTTP &) = delete;

    SessionMmanagerHTTP &operator=(const SessionMmanagerHTTP &) = delete;

    explicit SessionMmanagerHTTP(const accept_conf_type &accept_conf_vec,
                                  func_request_parser func_request_parser_method,
                                  func_disconnect_cb func_disconnect_callback,
                                  func_receive_cb func_receive_callback);

    /**
    * @param[ in] socket
    */
    virtual void AddOneSession(socket_type socket) override;

    /**
    * @param[ in] session_id要删除的session_id
    */
    virtual void DelOneSession(int session_id) override;

protected:

private:
    session_map_type session_map_; // session_http存储map
};
}//namespace utility
}//namespace diy

#endif//!utility_include_utility_session_tcp_session_manager_h