#ifndef utility_include_utility_session_tcp_session_asio_h
#define utility_include_utility_session_tcp_session_asio_h

#include "package/data.hpp"
#include "pattern/observer.h"
#include "session/http/request.h"

namespace diy
{
namespace utility
{
class SessionImpl : public ObserverImpl, public std::enable_shared_from_this<SessionImpl>
{
public:
    typedef std::size_t size_type;
    typedef std::recursive_mutex mutex_type;
    typedef std::condition_variable condv_type;
    typedef std::lock_guard<mutex_type> lock_guard_type;
    typedef asio::steady_timer timer_type;
    typedef asio::steady_timer::time_point time_point_type;
    typedef std::function<void(std::shared_ptr<SessionImpl> /*session_ptr*/, int /*sessionid*/, int /*reason*/)> func_disconnect_cb;
    typedef std::function<void(std::shared_ptr<SessionImpl> /*session_ptr*/, int /*sessionid*/, int /*type*/, char * /*data*/, int /*length*/)> func_receive_cb;
    typedef std::function<int(const DataImpl & /*data*/, int & /*type[out]*/)> func_request_parser;

    explicit SessionImpl(func_request_parser func_request_parser_method,
                         func_disconnect_cb func_disconnect_callback,
                         func_receive_cb func_receive_callback)
        : session_id_(0),
          func_disconnect_cb_(func_disconnect_callback),
          func_receive_callback_(func_receive_callback),
          func_request_parser_(func_request_parser_method)
    {
    }
    virtual ~SessionImpl() {}

    virtual void Start() = 0;

    virtual void Stop() = 0;

    virtual bool Stopped() const = 0;

    int session_id()
    {
        return session_id_;
    }

    void set_session_id(int session_id)
    {
        session_id_ = session_id;
    }

protected:
    int session_id_;

    func_disconnect_cb func_disconnect_cb_;
    func_receive_cb func_receive_callback_;
    func_request_parser func_request_parser_;
};

class SessionMmanagerTCP;

class SessionTcpImme : public SessionImpl
{
public:
    typedef asio::ip::tcp::socket socket_type;
    typedef std::shared_ptr<DataTCP> data_ptr_type;
    typedef std::queue<data_ptr_type> queue_type;
    typedef std::shared_ptr<SessionMmanagerTCP> manager_ptr_type;

    SessionTcpImme(const SessionTcpImme &) = delete;

    SessionTcpImme &operator=(const SessionTcpImme &) = delete;

    /**
     * @param[ in] socket 套接字
     * @param[ in] session_manager_ptr
     * @param[ in] func_request_parser_method 请求消息解包方法
     * @param[ in] func_disconnect_callback 断开回调函数
     * @param[ in] func_receive_callback 接收回调函数
     * @param[ in] queue_max_size 发送队列上限 默认0不限制
     */
    explicit SessionTcpImme(socket_type socket,
                            manager_ptr_type session_manager_ptr,
                            func_request_parser func_request_parser_method,
                            func_disconnect_cb func_disconnect_callback,
                            func_receive_cb func_receive_callback,
                            size_type queue_max_size = 0);

    /**
    * 启动
    */
    void Start() override;

    /**
    * 停止
    */
    void Stop() override;
    /**
    * 已停
    */
    bool Stopped() const override;

    /**
    * 更新
    */
    void Update() override;

    /**
    * @param[ in] data 发送数据
    * @param[ in] length 发送长度
    * @return(int) 压包结果success_code/error_queue_full
    */
    virtual int AsyncSend(const char *data, size_type length);

    /*
     */
    const socket_type &socket() const;

protected:
    virtual void handle_stop();

    virtual void do_recv();

    virtual void handle_async_recv();

    virtual void await_send();

    virtual void do_send();

    virtual void handle_async_send();

    virtual void on_receive(int type, char *data, int length);

    manager_ptr_type session_manager_ptr_;
    socket_type socket_;
    DataTCP recv_data_;

    size_type data_queue_max_size_;
    mutex_type send_queue_mutex_;
    queue_type send_data_queue_;
};

class SessionTcpTmot : public SessionTcpImme
{
public:
    enum timeout_sec
    {
        recv_timeout = 30,
        send_timeout = 30,
        heartbeat_sec = 10
    };

    SessionTcpTmot(const SessionTcpTmot &) = delete;

    SessionTcpTmot &operator=(const SessionTcpTmot &) = delete;

    /**
    * @param[ in] socket 套接字
    * @param[ in] session_request_handler 请求消息解包器
    * @param[ in] func_receive_callback 接收回调函数
    * @param[ in] queue_max_size 发送队列上限 默认0不限制
    */
    explicit SessionTcpTmot(socket_type socket,
                            manager_ptr_type session_manager_ptr,
                            func_request_parser func_request_parser_method,
                            func_disconnect_cb func_disconnect_callback,
                            func_receive_cb func_receive_callback,
                            size_type queue_max_size = 0);

    /**
    * 启动
    */
    void Start() override;

    /**
    * 停止
    */
    void Stop() override;

    /**
    * 已停
    */
    bool Stopped() const override;

    /**
    * @param[ in] data 发送数据
    * @param[ in] length 发送长度
    * @return(int) 压包结果success_code/error_queue_full
    */
    int AsyncSend(const char *data, size_type length) override;

protected:
    void handle_stop() override;

    void do_recv() override;

    void await_send() override;

    void do_send() override;

    void check_deadline(timer_type &deadline);

    void check_heartbeat();

    timer_type recv_deadline_;
    timer_type send_deadline_;
    timer_type non_empty_send_queue_;
    std::string heartbeat_data_;
    timer_type heartbeat_timer_;
};

class SessionTcpClient : public SessionTcpTmot
{
public:
    using tcp_client_ptr = std::shared_ptr<SessionTcpClient>;
    using resolver_type = asio::ip::tcp::resolver;

    /**
     *
     * @param io_service
     * @param host
     * @param port
     * @param session_request_handler
     * @param func_receive_callback
     * @param queue_max_size
     */
    explicit SessionTcpClient(asio::io_service &io_service,
                              func_request_parser func_request_parser_method,
                              func_disconnect_cb func_disconnect_callback,
                              func_receive_cb func_receive_callback,
                              std::string login_query_buf,
                              std::string heart_beat_buf,
                              size_type queue_max_size = 0);

    /**
    * 连接
    */
    void Connect(const std::string &host, const std::string &port);

    /**
    * 停止
    */
    void Stop() override;

    /**
     *
     */
    void Close();

    /**
     *
     * @param host
     * @param port
     */
    void UpdateAddr(const std::string &host, const std::string &port);

protected:
    virtual void handle_stop() override;

private:
    void connect();
    void handle_connect(std::error_code ec, resolver_type::iterator endpoint_iter);

protected:
    mutex_type addr_mutex_;
    //resolver_type::results_type endpoints_;
    std::string remote_host_;
    std::string remote_port_;
    std::atomic<bool> reconnect_;
    std::string login_query_data_;
};

class SessionMmanagerHTTP;

class SessionHTTP : public SessionTcpTmot
{
public:
    SessionHTTP(const SessionHTTP &) = delete;

    SessionHTTP &operator=(const SessionHTTP &) = delete;

    /**
    * @param[ in] socket 套接字
    * @param[ in] session_request_handler 请求消息解包器
    * @param[ in] func_receive_callback 接收回调函数
    * @param[ in] queue_max_size 发送队列上限 默认0不限制
    */
    explicit SessionHTTP(socket_type socket,
                         manager_ptr_type session_manager_ptr,
                         func_request_parser func_request_parser_method,
                         func_disconnect_cb func_disconnect_callback,
                         func_receive_cb func_receive_callback,
                         size_type queue_max_size = 0);

protected:
    virtual void handle_async_recv() override;

    virtual void handle_async_send() override;

private:
    HTTPRequest http_request_;
};
} //namespace utility
} //namespace diy

#endif //!utility_include_utility_session_tcp_session_asio_h