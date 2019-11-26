#include "session/tcp/session_manager.h"
#include "logger/boost_logger.hpp"

namespace diy
{
namespace utility
{
//////////////////////////////////////////////////////////////////////////
/* SessionMmanagerTCP */
//////////////////////////////////////////////////////////////////////////
SessionMmanagerTCP::SessionMmanagerTCP(const accept_conf_type &accept_conf_vec,
        func_request_parser func_request_parser_method,
        func_disconnect_cb func_disconnect_callback,
        func_receive_cb func_receive_callback)
        : session_id_alloter_(0),
          session_max_num_(0),
          session_queue_max_size_(0),
          accept_conf_(accept_conf_vec),
          func_disconnect_cb_(func_disconnect_callback),
          func_receive_callback_(func_receive_callback),
          func_request_parser_(func_request_parser_method)
{
}

void SessionMmanagerTCP::Start()
{
    lock_guard_type guard(session_mutex_);

    // 获取配置的工作线程数
    int threads_num = std::get<0>(accept_conf_);
    if (threads_num < workthreads_minnum)
    {
        threads_num = workthreads_minnum;
    }
    else if (threads_num > workthreads_maxnum)
    {
        threads_num = workthreads_maxnum;
    }

    UtilityBoostLogger(info) << __FUNCTION__ << " work threads num " << threads_num;

    // TODO:最初想法有问题 此处不应该将acceptor的start和工作线程绑定 只需要创建并启动即可 后续进行修改
    // 根据配置创建acceptor
    for (const auto &conf_tuple : std::get<1>(accept_conf_))
    {
        try
        {
            auto self_ = std::dynamic_pointer_cast<SessionMmanagerTCP>(shared_from_this());
            std::string string_address, string_port;
            std::tie(string_address, string_port) = conf_tuple;
            acceptor_ptr_type acceptor_ptr(std::make_shared<acceptor_type>(io_service_, self_, string_address, string_port));
            acceptor_ptr->Start();
            acceptor_vec_.emplace_back(acceptor_ptr);
            UtilityBoostLogger(info) << __FUNCTION__ << " accept.add " << string_address << ":" << string_port;
        }
        catch (const std::exception &excp)
        {
            // TODO:
            UtilityBoostLogger(error) << "sessionmanager accept.start catch excp:" << excp.what();
        }
    }

    // 补足配置的线程数量 刨除主线程1个
    for (int i = 0; i < threads_num - 1; ++i)
    {
        thread_ptr_vec_.emplace_back(std::make_shared<thread_type>([this]() {
            io_service_.run();
            UtilityBoostLogger(info) << "sessionmanager worker thread:" << std::this_thread::get_id() << " is over";
        }));
    }
}

void SessionMmanagerTCP::Stop()
{
    lock_guard_type guard(session_mutex_);

    // 停止所有acceptor
    for (auto &acceptor_ptr : acceptor_vec_)
    {
        acceptor_ptr->Stop();
    }

    // 停止所有session
    for (auto &session_pair : session_map_)
    {
        session_pair.second->Stop();
    }

    // 等待线程退出
    for (auto &thread_ptr : thread_ptr_vec_)
    {
        if (thread_ptr->joinable())
        {
            thread_ptr->join();
        }
    }
}

void SessionMmanagerTCP::AddOneSession(socket_type socket)
{
    lock_guard_type guard(session_mutex_);
    if (session_max_num_ == 0 || session_map_.size() < session_max_num_)
    {
        int save_sessio_id = 0;
        try
        {
            auto self_ = std::dynamic_pointer_cast<SessionMmanagerTCP>(shared_from_this());
            socket.set_option(asio::ip::tcp::no_delay(true));
            session_ptr_type session_ptr(std::make_shared<session_type>(std::move(socket), self_, func_request_parser_, func_disconnect_cb_, func_receive_callback_, session_queue_max_size_));
            save_sessio_id = ++session_id_alloter_;
            session_ptr->set_session_id(save_sessio_id);
            session_map_[session_ptr->session_id()] = session_ptr;
            UtilityBoostLogger(info) << __FUNCTION__
                              << " sessionid=" << session_ptr->session_id()
                              << " remote_ep:" << session_ptr->socket().remote_endpoint().address().to_string()
                              << "/" << session_ptr->socket().remote_endpoint().port()
                              << " local_ep:" << session_ptr->socket().local_endpoint().address().to_string()
                              << "/" << session_ptr->socket().local_endpoint().port();
            session_ptr->Start();
        }
        catch (const std::exception &excp)
        {
            DelOneSession(save_sessio_id);
            UtilityBoostLogger(error) << __FUNCTION__ << " excp:" << excp.what();
        }
    }
    else
    {
        // error::error_session_full
        UtilityBoostLogger(info) << __FUNCTION__ << "session full"
                          << " remote_ep:" << socket.remote_endpoint().address().to_string()
                          << "/" << socket.remote_endpoint().port()
                          << " local_ep:" << socket.local_endpoint().address().to_string()
                          << "/" << socket.local_endpoint().port();
    }
}

void SessionMmanagerTCP::DelOneSession(int session_id)
{
    lock_guard_type guard(session_mutex_);
    session_map_.erase(session_id);
    UtilityBoostLogger(info) << __FUNCTION__ << " sessionid=" << session_id;
}

int SessionMmanagerTCP::CloseOneSession(int session_id)
{
    lock_guard_type guard(session_mutex_);
    auto iter = session_map_.find(session_id);
    if (iter != session_map_.end())
    {
        iter->second->Stop();
        return 0;
    }
    return -1;
}

int SessionMmanagerTCP::AsyncSend(int sessionid, const char *data, size_type length)
{
    lock_guard_type guard(session_mutex_);
    if (sessionid == 0)
    {
        for (auto &_session : session_map_)
        {
            _session.second->AsyncSend(data, length);
        }
    }
    else
    {
        auto iter = session_map_.find(sessionid);
        if (iter != session_map_.end())
        {
            return iter->second->AsyncSend(data, length);
        }
        else
        {
            return error_session_not_exist;
        }
    }
    return success_code;
}
//////////////////////////////////////////////////////////////////////////
/* SessionMmanagerHTTP */
//////////////////////////////////////////////////////////////////////////
SessionMmanagerHTTP::SessionMmanagerHTTP(const accept_conf_type &accept_conf_vec,
                                           func_request_parser func_request_parser_method,
                                           func_disconnect_cb func_disconnect_callback,
                                           func_receive_cb func_receive_callback)
        : SessionMmanagerTCP(accept_conf_vec, func_request_parser_method, func_disconnect_callback, func_receive_callback)
{
}

void SessionMmanagerHTTP::AddOneSession(socket_type socket)
{
    lock_guard_type guard(session_mutex_);
    if (session_max_num_ == 0 || session_map_.size() < session_max_num_)
    {
        try
        {
            auto self_ = std::dynamic_pointer_cast<SessionMmanagerHTTP>(shared_from_this());
            socket.set_option(asio::ip::tcp::no_delay(true));
            session_ptr_type session_ptr(std::make_shared<session_type>(std::move(socket), self_, func_request_parser_, func_disconnect_cb_, func_receive_callback_, session_queue_max_size_));
            session_ptr->set_session_id(++session_id_alloter_);
            session_map_[session_ptr->session_id()] = session_ptr;
            session_ptr->Start();
        }
        catch (const std::exception &excp)
        {
            UtilityBoostLogger(error) << __FUNCTION__ << " excp:" << excp.what();
        }
    }
    else
    {
        // TODO:
        // error::error_session_full
    }
}

void SessionMmanagerHTTP::DelOneSession(int session_id)
{
    lock_guard_type guard(session_mutex_);
    session_map_.erase(session_id);
}
}//namespace utility
}//namespace diy