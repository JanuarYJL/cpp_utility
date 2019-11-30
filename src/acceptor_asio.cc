#include "session/tcp/acceptor_asio.h"
#include "session/tcp/session_manager.h"

namespace diy
{
namespace utility
{
AcceptorTCP::AcceptorTCP(asio::io_service &io_service, manager_ptr_type session_manager_ptr, const std::string &address, const std::string &port)
    : AcceptorImpl(io_service), acceptor_(io_service), session_manager_ptr_(session_manager_ptr)
{
    resolver_type resolver(io_service_);
    endpoint_type endpoint = *resolver.resolve(address, port).begin();
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
}

void AcceptorTCP::Start()
{
    handle_accept();
}

void AcceptorTCP::Stop()
{
    if (acceptor_.is_open())
    {
        acceptor_.close();
    }
}

void AcceptorTCP::handle_accept()
{
    acceptor_.async_accept([this](std::error_code ec, socket_type socket) {
        if (!acceptor_.is_open())
        {
            return;
        }

        if (!ec)
        {
            if (session_manager_ptr_)
            {
                session_manager_ptr_->AddOneSession(std::move(socket));
            }
            else
            {
                // TODO:
            }
        }
        else
        {
            // TODO:
        }

        // 继续接收
        handle_accept();
    });
}
} //namespace utility
} //namespace diy