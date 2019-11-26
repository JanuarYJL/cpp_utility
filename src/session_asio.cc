#include "session/tcp/session_asio.h"
#include "session/tcp/session_manager.h"
#include "logger/boost_logger.hpp"

namespace diy
{
namespace utility
{
//////////////////////////////////////////////////////////////////////////
/* SessionTcpImme */
//////////////////////////////////////////////////////////////////////////
SessionTcpImme::SessionTcpImme(socket_type socket,
        manager_ptr_type session_manager_ptr,
        func_request_parser func_request_parser_method,
        func_disconnect_cb func_disconnect_callback,
        func_receive_cb func_receive_callback,
        size_type queue_max_size/* = 0*/)
:SessionImpl(func_request_parser_method,func_disconnect_callback, func_receive_callback),
 session_manager_ptr_(session_manager_ptr),
 socket_(std::move(socket)),
 data_queue_max_size_(queue_max_size)
{
}

void SessionTcpImme::Start()
{
    // 启动接收/发送链
    do_recv();

    await_send();
}

void SessionTcpImme::Stop()
{
    handle_stop();
}

bool SessionTcpImme::Stopped() const
{
    return !socket_.is_open();
}

void SessionTcpImme::Update()
{
    // TODO:
}

int SessionTcpImme::AsyncSend(const char *data, size_type length)
{
    if (!data || length == 0 || length > max_packet_length)
    {
        return error_bad_data;
    }
    if (Stopped())
    {
        return error_session_stopped;
    }
    lock_guard_type guard(send_queue_mutex_);
    if (data_queue_max_size_ == 0 || send_data_queue_.size() < data_queue_max_size_)
    {
        bool active_send = send_data_queue_.empty();
        send_data_queue_.emplace(std::make_shared<DataTCP>(data, length));
        if (active_send)
        {
            await_send();
        }
        return success_code;
    }
    else
    {
        return error_queue_full;
    }
}

const SessionTcpImme::socket_type & SessionTcpImme::socket() const
{
    return socket_;
}

void SessionTcpImme::handle_stop()
{
    if (session_manager_ptr_)
    {
        session_manager_ptr_->DelOneSession(session_id());
    }
    if (func_disconnect_cb_)
    {
        func_disconnect_cb_(std::dynamic_pointer_cast<SessionTcpImme>(shared_from_this()), session_id(), 0);
    }
    lock_guard_type guard(send_queue_mutex_);
    if (socket_.is_open())
    {
        boost::system::error_code ec;
        socket_.close(ec);
    }
}

void SessionTcpImme::do_recv()
{
    if (Stopped())
    {
        return;
    }
    handle_async_recv();
}

void SessionTcpImme::handle_async_recv()
{
    auto self_ = std::dynamic_pointer_cast<SessionTcpImme>(shared_from_this());
    socket_.async_read_some(asio::buffer(recv_data_.data() + recv_data_.data_size(), recv_data_.data_capacity() - recv_data_.data_size()), [this, self_](std::error_code ec, std::size_t bytes_transferred) {
        if (!ec)
        {
            // 更新接收缓存array长度
            recv_data_.set_size(recv_data_.data_size() + bytes_transferred);

            // TODO:后续缓存解包可以进行优化
            //  1.循环取包时不每次都移动到缓存头部 可以完全解析后再移动  2.将缓存做循环队列使用
            while (true)
            {
                // 解析接收缓存数据
                int parse_type = 0;
                int parse_result = func_request_parser_(recv_data_, parse_type);
                if (parse_result == error_packet_bad)
                {
                    // 解包异常 停止
                    handle_stop();
                    break;
                }
                else if (parse_result == error_packet_less)
                {
                    // 不足一包 继续接收
                    if (recv_data_.data_size() == recv_data_.data_capacity())
                    {
                        // 缓存已满时扩容
                        recv_data_.expansion();
                    }
                    do_recv();
                    break;
                }
                else
                {
                    // 将解析出的包回调给业务层
                    on_receive(parse_type, recv_data_.data(), parse_result);
                    // 整理缓存 继续解包
                    recv_data_.move_to_head(parse_result);
                }
            }
        }
        else
        {
            // 接收异常 停止
            handle_stop();
        }
    });
}

void SessionTcpImme::await_send()
{
    if (!Stopped())
    {
        lock_guard_type guard(send_queue_mutex_);
        if (send_data_queue_.empty())
        {
            // 无数据时挂起等待数据
        }
        else
        {
            // 发送数据
            do_send();
        }
    }
}

void SessionTcpImme::do_send()
{
    handle_async_send();
}

void SessionTcpImme::handle_async_send()
{
    auto self_ = std::dynamic_pointer_cast<SessionTcpImme>(shared_from_this());
    asio::async_write(socket_, asio::buffer(send_data_queue_.front()->data(), send_data_queue_.front()->data_size()), [this, self_](std::error_code ec, std::size_t bytes_transferred) {
        if (!ec)
        {
            lock_guard_type guard(send_queue_mutex_);
            // 弹出完成数据
            send_data_queue_.pop();
            // 继续检测发送
            await_send();
        }
        else
        {
            // 发送异常 停止
            handle_stop();
        }
    });
}

void SessionTcpImme::on_receive(int type, char *data, int length)
{
    func_receive_callback_(shared_from_this(), session_id_, type, data, length);
}

//////////////////////////////////////////////////////////////////////////
/* SessionTcpTmot */
//////////////////////////////////////////////////////////////////////////
SessionTcpTmot::SessionTcpTmot(socket_type socket,
        manager_ptr_type session_manager_ptr,
        func_request_parser func_request_parser_method,
        func_disconnect_cb func_disconnect_callback,
        func_receive_cb func_receive_callback,
        size_type queue_max_size/* = 0*/)
:SessionTcpImme(std::move(socket), session_manager_ptr, func_request_parser_method, func_disconnect_callback, func_receive_callback, queue_max_size),
 recv_deadline_(socket_.get_io_service()),
 send_deadline_(socket_.get_io_service()),
 non_empty_send_queue_(socket_.get_io_service()),
 heartbeat_timer_(socket_.get_io_service())
{
    recv_deadline_.expires_at(time_point_type::max());
    send_deadline_.expires_at(time_point_type::max());
    non_empty_send_queue_.expires_at(time_point_type::max());
    heartbeat_timer_.expires_at(time_point_type::max());
}

void SessionTcpTmot::Start()
{
    // 启动接收/发送链
    do_recv();
    recv_deadline_.async_wait(std::bind(&SessionTcpTmot::check_deadline, std::dynamic_pointer_cast<SessionTcpTmot>(shared_from_this()), std::ref(recv_deadline_)));
    await_send();
    send_deadline_.async_wait(std::bind(&SessionTcpTmot::check_deadline, std::dynamic_pointer_cast<SessionTcpTmot>(shared_from_this()), std::ref(send_deadline_)));
}

void SessionTcpTmot::Stop()
{
    handle_stop();
}

bool SessionTcpTmot::Stopped() const
{
    return !socket_.is_open();
}

int SessionTcpTmot::AsyncSend(const char *data, size_type length)
{
    if (!data || length == 0 || length > max_packet_length)
    {
        return error_bad_data;
    }
    if (Stopped())
    {
        return error_session_stopped;
    }
    lock_guard_type guard(send_queue_mutex_);
    if (data_queue_max_size_ == 0 || send_data_queue_.size() < data_queue_max_size_)
    {
        send_data_queue_.emplace(std::make_shared<DataTCP>(data, length));
        non_empty_send_queue_.expires_at(time_point_type::min());

        return success_code;
    }
    else
    {
        return error_queue_full;
    }
}

void SessionTcpTmot::handle_stop()
{
    SessionTcpImme::handle_stop();

    recv_deadline_.cancel();
    send_deadline_.cancel();
    non_empty_send_queue_.cancel();
    heartbeat_timer_.cancel();
}

void SessionTcpTmot::do_recv()
{
    if (Stopped())
    {
        return;
    }
    recv_deadline_.expires_after(asio::chrono::seconds(timeout_sec::recv_timeout));
    handle_async_recv();
}

void SessionTcpTmot::await_send()
{
    if (!Stopped())
    {
        lock_guard_type guard(send_queue_mutex_);
        if (send_data_queue_.empty())
        {
            // 无数据时挂起等待数据
            non_empty_send_queue_.expires_at(time_point_type::max());
            non_empty_send_queue_.async_wait(std::bind(&SessionTcpTmot::await_send, std::dynamic_pointer_cast<SessionTcpTmot>(shared_from_this())));
            // 设置心跳定时器
            heartbeat_timer_.expires_after(asio::chrono::seconds(timeout_sec::heartbeat_sec));
            heartbeat_timer_.async_wait(std::bind(&SessionTcpTmot::check_heartbeat, std::dynamic_pointer_cast<SessionTcpTmot>(shared_from_this())));
        }
        else
        {
            // 发送数据
            do_send();
        }
    }
}

void SessionTcpTmot::do_send()
{
    send_deadline_.expires_after(asio::chrono::seconds(timeout_sec::send_timeout));
    handle_async_send();
}

void SessionTcpTmot::check_deadline(timer_type &deadline)
{
    if (!Stopped())
    {
        if (deadline.expiry() <= timer_type::clock_type::now())
        {
            // 超时
            //handle_stop();
            // 超时时关闭socket不调用handle_stop()由接收发送响应来调用关闭
            if (socket_.is_open())
            {
                boost::system::error_code ec;
                socket_.close(ec);
            }
        }
        else
        {
            // 挂起 继续
            deadline.async_wait(std::bind(&SessionTcpTmot::check_deadline, std::dynamic_pointer_cast<SessionTcpTmot>(shared_from_this()), std::ref(deadline)));
        }
    }
}

void SessionTcpTmot::check_heartbeat()
{
    if (!Stopped())
    {
        if (heartbeat_timer_.expiry() <= timer_type::clock_type::now())
        {
            // 超时 发送心跳
            AsyncSend(heartbeat_data_.c_str(), heartbeat_data_.length());
        }
    }
}

//////////////////////////////////////////////////////////////////////////
/* SessionTcpClient */
//////////////////////////////////////////////////////////////////////////
/**
 *
 * @param io_service
 * @param host
 * @param port
 * @param session_request_handler
 * @param func_receive_callback
 * @param queue_max_size
 */
SessionTcpClient::SessionTcpClient(asio::io_service &io_service,
        func_request_parser func_request_parser_method,
        func_disconnect_cb func_disconnect_callback,
        func_receive_cb func_receive_callback,
        std::string login_query_buf,
        std::string heart_beat_buf,
        size_type queue_max_size)
:SessionTcpTmot(socket_type(io_service), nullptr, func_request_parser_method, func_disconnect_callback, func_receive_callback, queue_max_size),
 reconnect_(false),
 login_query_data_(login_query_buf)
{
    heartbeat_data_ = heart_beat_buf;
}

void SessionTcpClient::Connect(const std::string &host, const std::string &port)
{
    UpdateAddr(host, port);
    reconnect_ = true;
    connect();
}

void SessionTcpClient::Stop()
{
    handle_stop();
}

void SessionTcpClient::Close()
{
    reconnect_ = false;
    Stop();
}

void SessionTcpClient::UpdateAddr(const std::string &host, const std::string &port)
{
    lock_guard_type lg(addr_mutex_);
    remote_host_ = host;
    remote_port_ = port;
}

void SessionTcpClient::handle_stop()
{
    if (reconnect_)
    {
        // client的停止会继续重连 此处只关闭socket并清空队列 并开始重连
        lock_guard_type lkg(send_queue_mutex_);
        queue_type empty_queue;
        send_data_queue_.swap(empty_queue);
        recv_deadline_.expires_at(time_point_type::max());
        send_deadline_.expires_at(time_point_type::max());
        non_empty_send_queue_.expires_at(time_point_type::max());
        heartbeat_timer_.expires_at(time_point_type::max());
        if (socket_.is_open())
        {
            boost::system::error_code ec;
            socket_.close(ec);
        }

        // 延迟5s进行重连
        auto _self = std::dynamic_pointer_cast<SessionTcpClient>(shared_from_this());
        auto delay_timer = std::make_shared<timer_type>(socket_.get_io_service());
        delay_timer->expires_after(asio::chrono::seconds(5));
        delay_timer->async_wait(std::bind([this, delay_timer, _self](){ connect(); }));
    }
    else
    {
        SessionTcpTmot::Stop();
    }
}

void SessionTcpClient::connect()
{
    lock_guard_type lg(addr_mutex_);
    resolver_type rsl(socket_.get_io_service());
    resolver_type::iterator endpoint_iter = rsl.resolve(remote_host_, remote_port_);

    if (endpoint_iter != resolver_type::iterator())
    {
        socket_.async_connect(*endpoint_iter, std::bind(&SessionTcpClient::handle_connect, std::dynamic_pointer_cast<SessionTcpClient>(shared_from_this()), std::placeholders::_1, endpoint_iter));
    }
    else
    {
        UtilityBoostLogger(info) << "connect failed, remote_addr:" << remote_host_ << "/" << remote_port_;
    }
}

void SessionTcpClient::handle_connect(std::error_code ec, resolver_type::iterator endpoint_iter)
{
    try
    {
        // 连接成功
        if (!ec)
        {
            UtilityBoostLogger(info) << "connect success, endpoint:" << endpoint_iter->endpoint();
            set_session_id(session_id() + 1);
            SessionTcpTmot::Start();
            SessionTcpTmot::AsyncSend(login_query_data_.c_str(), login_query_data_.length());
        }
        // 继续连接下一个地址
        else if (++endpoint_iter != resolver_type::iterator())
        {
            boost::system::error_code ec;
            socket_.close(ec);
            socket_.async_connect(*endpoint_iter, std::bind(&SessionTcpClient::handle_connect, std::dynamic_pointer_cast<SessionTcpClient>(shared_from_this()), std::placeholders::_1, endpoint_iter));
        }
        // 连接失败
        else
        {
            UtilityBoostLogger(info) << "connect failed, error_code:" << ec;
            // 进行重连
            handle_stop();
        }
    }
    catch (std::exception &ecp)
    {
        UtilityBoostLogger(error) << __FUNCTION__ << " catch:" << ecp.what();
    }
}

//////////////////////////////////////////////////////////////////////////
/* SessionHTTP */
//////////////////////////////////////////////////////////////////////////
/**
* @param[ in] socket 套接字
* @param[ in] session_request_handler 请求消息解包器
* @param[ in] func_receive_callback 接收回调函数
* @param[ in] queue_max_size 发送队列上限 默认0不限制
*/
SessionHTTP::SessionHTTP(socket_type socket,
        manager_ptr_type session_manager_ptr,
        func_request_parser func_request_parser_method,
        func_disconnect_cb func_disconnect_callback,
        func_receive_cb func_receive_callback,
        size_type queue_max_size/* = 0*/)
        : SessionTcpTmot(std::move(socket), session_manager_ptr, func_request_parser_method, func_disconnect_callback, func_receive_callback, queue_max_size)
{
}

void SessionHTTP::handle_async_recv()
{
    auto self_ = std::dynamic_pointer_cast<SessionHTTP>(shared_from_this());
    socket_.async_read_some(asio::buffer(recv_data_.data() + recv_data_.data_size(), recv_data_.data_capacity() - recv_data_.data_size()), [this, self_](std::error_code ec, std::size_t bytes_transferred) {
        if (!ec)
        {
            HTTPRequest::result_type result;
            size_type parse_size;
            std::tie(result, parse_size) = http_request_.parse(recv_data_.data() + recv_data_.data_size(), recv_data_.data() + recv_data_.data_size() + bytes_transferred);
            // 更新接收缓存array长度
            recv_data_.set_size(recv_data_.data_size() + bytes_transferred);
            // 解析成功
            if (result == HTTPRequest::result_type::good)
            {
                // 回调接收函数

                // TmpTest
                std::string string_buf = HTTPReply::stock_reply(HTTPReply::bad_gateway).to_string();
                http_request_.reset();
                AsyncSend(string_buf.c_str(), string_buf.length());
            }
                // 请求错误
            else if (result == HTTPRequest::bad)
            {
                // 请求错误处理

                // TmpTest
                std::string string_buf = HTTPReply::stock_reply(HTTPReply::bad_request).to_string();
                http_request_.reset();
                AsyncSend(string_buf.c_str(), string_buf.length());
            }
                // 包不完全 继续接收
            else
            {
                // 不足一包 继续接收
                if (recv_data_.data_size() == recv_data_.data_capacity())
                {
                    // 缓存已满时扩容
                    recv_data_.expansion();
                }
                do_recv();
            }
            /*
            // TODO:后续缓存解包可以进行优化
            //  1.循环取包时不每次都移动到缓存头部 可以完全解析后再移动  2.将缓存做循环队列使用
            while (true)
            {
                // 解析接收缓存数据
                int parse_type = 0;
                int parse_result = func_request_parser_(recv_data_, parse_type);
                if (parse_result == error_packet_bad)
                {
                    // 解包异常 停止
                    handle_stop();
                    break;
                }
                else if (parse_result == error_packet_less)
                {
                    // 不足一包 继续接收
                    if (recv_data_.data_size() == recv_data_.data_capacity())
                    {
                        // 缓存已满时扩容
                        recv_data_.expansion();
                    }
                    do_recv();
                    break;
                }
                else
                {
                    // 将解析出的包回调给业务层
                    on_receive(parse_type, recv_data_.data(), parse_result);
                    // 整理缓存 继续解包
                    recv_data_.move_to_head(parse_result);
                }
            }
            */
        }
        else
        {
            // 接收异常 停止
            handle_stop();
        }
    });
}

void SessionHTTP::handle_async_send()
{
    auto self_ = std::dynamic_pointer_cast<SessionHTTP>(shared_from_this());
    asio::async_write(socket_, asio::buffer(send_data_queue_.front()->data(), send_data_queue_.front()->data_size()), [this, self_](std::error_code ec, std::size_t bytes_transferred) {
        if (!ec)
        {
            lock_guard_type guard(send_queue_mutex_);
            // 弹出完成数据
            send_data_queue_.pop();
            // 继续检测发送
            await_send();
        }
        else
        {
            // 发送异常 停止
            handle_stop();
        }
    });
}
}//namespace utility
}//namespace diy