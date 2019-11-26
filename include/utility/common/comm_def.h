#ifndef utility_include_utility_common_def_h
#define utility_include_utility_common_def_h

namespace diy
{
namespace utility
{
enum constant
{
    workthreads_minnum = 1,
    workthreads_maxnum = 16,
    workthreads_default = 4,

    max_quote_packet_length = 1024*32,
    max_quote_packet_length_8k = 1024*8,
    max_packet_length = 1*1024*1024, // 1M
};
static const unsigned int check_bit_ = 0x02673602; // "'STX'diy'ETX'"
static const unsigned short int full_space_half_ = 0xA1;
static const unsigned short int full_space_ = 41377;
static const char dir_seperator_ = '/';

// DISALLOW_INIT_COPY_AND_ASSIGN def
#ifndef DISALLOW_INIT_COPY_ASSIGN
#define DISALLOW_INIT_COPY_ASSIGN(T) \
    T() = delete;                    \
    T(const T &) = delete;           \
    T &operator=(const T &) = delete;
#endif

// DISALLOW_COPY_ASSIGN def
#ifndef DISALLOW_COPY_ASSIGN
#define DISALLOW_COPY_ASSIGN(T) \
    T(const T &) = delete;      \
    T &operator=(const T &) = delete;
#endif

} //namespace utility
} //namespace diy

#endif //!utility_include_utility_common_def_h