#ifndef utility_include_utility_encoding_code_convert_h
#define utility_include_utility_encoding_code_convert_h

#include <iconv.h>
#include <cstring>
#include <algorithm>

namespace diy
{
namespace utility
{
/*
 * input :
 *   from_code   :"gb2312","utf-8",...
 *   to_code     :"gb2312","utf-8",...
 *   max_dst_len :dst buffer max length
 *   src         :src buffer pointer
 *   src_len     :src buffer actual length except '\0'
 * output:
 *   dst         :dst buffer pointer
 * return:
 *   fail        :-1
 *   success     :actual dst length
 */
static int iconv_convert(const char *from_code,
                         const char *to_code,
                         char *dst,
                         const int max_dst_len,
                         const char *src,
                         const int src_len)
{
    // fault-tolerant
    if (from_code == nullptr || to_code == nullptr || dst == nullptr || max_dst_len < 1 || src == nullptr || src_len < 1)
    {
        return -1;
    }
    if (strcmp(from_code, to_code) == 0)
    {
        int ndstlen = (std::min)(src_len, max_dst_len - 1);
        memcpy(dst, src, ndstlen);
        dst[ndstlen] = 0;
        return ndstlen;
    }
    // open iconv
    size_t uileftlen = max_dst_len;
    iconv_t cd = iconv_open(to_code, from_code);
    if (cd == (iconv_t)-1)
    {
        return -1;
    }
    // conver by iconv
    size_t uisrclen = src_len;
    if (iconv(cd, (char **)(&src), &uisrclen, &dst, &uileftlen) == (size_t)-1)
    {
        iconv_close(cd);
        return -1;
    }
    // close iconv
    iconv_close(cd);
    // return dst actul length
    return (max_dst_len - uileftlen);
}

} //namespace utility
} //namespace diy
#endif //utility_include_utility_encoding_code_convert_h
