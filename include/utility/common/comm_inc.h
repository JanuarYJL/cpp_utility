#ifndef utility_include_utility_common_inc_h
#define utility_include_utility_common_inc_h

#ifdef ASIO_STANDALONE
#include "asio.hpp"
#else
#include "boost/asio.hpp"
namespace asio = boost::asio;
#endif//ASIO_STANDALONE


#endif//!utility_include_utility_common_inc_h