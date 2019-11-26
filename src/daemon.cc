#include "daemon/daemon.h"

#include <fcntl.h>
#include <unistd.h>

namespace diy
{
namespace utility
{

bool Daemon::DaemonModeRun()
{
    if (daemon(1, 0) == -1)
    {
        return false;
    }
    else
    {
        return true;
    }
}

int lockfile(int fd)
{
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return (fcntl(fd, F_SETLK, &fl));
}

bool Daemon::AlreadyRunning(const std::string &lock_file)
{
    int fd = -1;
    std::string str_pid;
    fd = open(lock_file.c_str(), O_RDWR | O_CREAT, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
    if (fd < 0)
    {
        return true;
    }
    if (lockfile(fd) == -1)
    {
        // 尝试对文件进行加锁
        close(fd);
        return true;
    }
    else
    {
        // 写入运行实例的pid
        ftruncate(fd, 0);
        str_pid = std::to_string(getpid());
        write(fd, str_pid.c_str(), str_pid.length());
        return false;
    }
}

} //namespace utility
} //namespace diy
