#ifndef utility_include_utility_file_directory_h
#define utility_include_utility_file_directory_h

#include <vector>
#include <string>

namespace diy
{
namespace utility
{

struct file_info_t
{
    std::string str_file_name; // 文件相对路径，遍历文件夹时返回相对传入文件夹的路径
    time_t modify_time;
    long long file_size;
    bool is_directory;
};

extern std::vector<file_info_t> GoThroughDirectory(const std::string &str_directory, bool bRecurse = false, bool bFile = true, bool bDirectory = true);

} //namespace utility
} //namespace diy

#endif //utility_include_utility_file_directory_h
