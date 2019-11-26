#ifndef utility_include_utility_session_acceptor_asio_h
#define utility_include_utility_session_acceptor_asio_h

#include "session/tcp/session_asio.h"

namespace diy
{
namespace utility
{
class AcceptorImpl
{
public:
    AcceptorImpl(asio::io_service &io_service)
            : io_service_(io_service)
    {
    }

    virtual void Start() = 0;

    virtual void Stop() = 0;

protected:
    asio::io_service &io_service_;
};

class SessionMmanagerTCP;

class AcceptorTCP : public AcceptorImpl
{
public:
    typedef asio::ip::tcp::acceptor acceptor_type;
    typedef asio::ip::tcp::socket socket_type;
    typedef asio::ip::tcp::resolver resolver_type;
    typedef asio::ip::tcp::endpoint endpoint_type;
    typedef std::shared_ptr<SessionMmanagerTCP> manager_ptr_type;

    AcceptorTCP(const AcceptorTCP &) = delete;

    AcceptorTCP &operator=(const AcceptorTCP &) = delete;

    AcceptorTCP(asio::io_service &io_service, manager_ptr_type session_manager_ptr, const std::string &address, const std::string &port);

    virtual void Start() override;

    virtual void Stop() override;

private:
    void handle_accept();

    acceptor_type acceptor_;
    manager_ptr_type session_manager_ptr_;
};
}//namespace utility
}//namespace diy

#endif//!utility_include_utility_session_acceptor_asio_h