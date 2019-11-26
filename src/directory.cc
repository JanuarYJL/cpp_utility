#include "file/directory.h"

#include <stack>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include <stdio.h>

namespace diy
{
namespace utility
{

std::vector<file_info_t> GoThroughDirectory(const std::string &str_directory, bool bRecurse, bool bFile, bool bDirectory)
{
    std::vector<file_info_t> v_return;
    std::stack<std::string> sDirectory;
    sDirectory.push("");

    DIR *dp = NULL;
    struct dirent *dirp = NULL;
    struct stat file_info;
    while (!sDirectory.empty())
    {
        std::string str_current_directory = sDirectory.top();
        sDirectory.pop();
        std::string str_search = str_directory;
        if (str_current_directory.length() > 0)
        {
            str_search = str_directory + "/" + str_current_directory;
        }
        dp = opendir(str_search.c_str());
        if (!dp)
        {
            continue;
        }
        dirp = readdir(dp);
        while (dirp)
        {
            std::string str_current_file;
            if (str_current_directory.length() > 0)
            {
                str_current_file = str_current_directory + "/" + dirp->d_name;
            }
            else
            {
                str_current_file = dirp->d_name;
            }
            if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
            {
            }
            else
            {
                std::string str_full_name = str_search + "/" + dirp->d_name;
                stat(str_full_name.c_str(), &file_info);
                if (S_ISDIR(file_info.st_mode))
                {
                    if (bDirectory)
                    {
                        file_info_t tmpFIT;
                        tmpFIT.modify_time = file_info.st_mtime;
                        tmpFIT.file_size = 0;
                        tmpFIT.str_file_name = str_current_file;
                        tmpFIT.is_directory = true;
                        v_return.push_back(tmpFIT);
                    }
                    if (bRecurse)
                    {
                        sDirectory.push(str_current_file);
                    }
                }
                else
                {
                    if (bFile)
                    {
                        file_info_t tmpFIT;
                        tmpFIT.modify_time = file_info.st_mtime;
                        tmpFIT.file_size = file_info.st_size;
                        tmpFIT.str_file_name = str_current_file;
                        tmpFIT.is_directory = false;
                        v_return.push_back(tmpFIT);
                    }
                }
            }
            dirp = readdir(dp);
        }
        closedir(dp);
    }

    return v_return;
}

} //namespace utility
} //namespace diy