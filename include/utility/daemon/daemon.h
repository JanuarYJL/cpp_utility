#ifndef utility_include_utility_daemon_daemon_h
#define utility_include_utility_daemon_daemon_h

#include "common/common.h"

namespace diy
{
namespace utility
{
class Daemon
{
public:
    DISALLOW_INIT_COPY_ASSIGN(Daemon);

    static bool DaemonModeRun();
    /**
     * @rief: 检测是否已经运行
     * @param: 锁文件
     * @return: 检验结果
     */
    static bool AlreadyRunning(const std::string &lock_file);

private:
    /* data */
};

} //namespace utility
} //namespace diy

#endif //utility_include_utility_daemon_daemon_h