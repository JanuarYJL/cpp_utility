#ifndef utility_include_utility_pool_threadpool_hpp
#define utility_include_utility_pool_threadpool_hpp

#include <cstddef>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <algorithm>
#include <queue>
#include <functional>

#include "common/common.h"

namespace diy
{
namespace utility
{

class ThreadPoolCXX11
{
public:
    using size_type = std::size_t ;
    using task_type = std::function<void()>;
    std::vector<std::thread> thrd_pool_;
    std::queue<task_type> task_que_;
    std::mutex task_mutex_;
    std::condition_variable task_cv_;
    std::atomic<bool> running_;

    ThreadPoolCXX11(size_type thread_num = utility::workthreads_default)
    {
        if (thread_num > utility::workthreads_maxnum)
        {
            thread_num = utility::workthreads_maxnum;
        }
        if (thread_num < utility::workthreads_minnum)
        {
            thread_num = utility::workthreads_minnum;
        }
        thrd_pool_.reserve(thread_num);
        for (size_type ui = 0; ui < thread_num; ++ui)
        {
            thrd_pool_.emplace_back([this](){
                while (running_)
                {
                    task_type temp_task_;
                    {
                        std::unique_lock<std::mutex> lock(task_mutex_);
                        while (running_ && task_que_.empty())
                        {
                            task_cv_.wait(lock);
                        }
                        if (!running_ && task_que_.empty())
                        {
                            return;
                        }
                        temp_task_ = std::move(task_que_.front());
                        task_que_.pop();
                    }
                    temp_task_();
                }
            });
        }
    }

    ~ThreadPoolCXX11()
    {
        running_ = false;
        task_cv_.notify_all();
        for (auto &thrd : thrd_pool_) {
            if(thrd.joinable())
            {
                thrd.join();
            }
        }
    }

    template<class F, class... Args>
    void EnQueue(F&& f, Args&&... args)
    {
        auto temp_task_ = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        {
            std::unique_lock<std::mutex> lock(task_mutex_);
            task_que_.emplace([temp_task_](){
                temp_task_();
            });
        }
        task_cv_.notify_one();
    }

    template<class F, class... Args>
    auto EnQueueFuture(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto temp_task_sptr_ = std::make_shared< std::packaged_task<return_type()> >(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<return_type> res = temp_task_sptr_->get_future();
        {
            std::unique_lock<std::mutex> lock(task_mutex_);
            task_que_.emplace([temp_task_sptr_](){
                (*temp_task_sptr_)();
            });
        }
        task_cv_.notify_one();
        return res;
    }
};

class ThreadPoolASIO
{
public:
    using size_type = std::size_t;
    using thread_type = std::thread;
    using exec_type = asio::io_service::executor_type;
    using work_guard_type = asio::executor_work_guard<exec_type>;

    ThreadPoolASIO(const ThreadPoolASIO &) = delete;
    ThreadPoolASIO &operator=(const ThreadPoolASIO &) = delete;

    explicit ThreadPoolASIO(size_type thread_num = utility::workthreads_default)
            : work_guard_(asio::make_work_guard(io_svc_))
    {
        if (thread_num < utility::workthreads_minnum)
        {
            thread_num = utility::workthreads_minnum;
        }
        if (thread_num > utility::workthreads_maxnum)
        {
            thread_num = utility::workthreads_maxnum;
        }

        workers_.reserve(thread_num);
        for (size_type ui = 0; ui < thread_num; ++ui)
        {
            workers_.emplace_back([this](){
                io_svc_.run();
            });
        }
    }

    ~ThreadPoolASIO()
    {
        if (!io_svc_.stopped())
        {
            io_svc_.stop();
        }
        for (auto &worker : workers_)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }

    template<class F, class... Args>
    void EnQueue(F&& f, Args&&... args)
    {
        io_svc_.post(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    }

    template<class F, class... Args>
    auto EnQueueFuture(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto temp_task_sptr_ = std::make_shared< std::packaged_task<return_type()> >(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<return_type> res = temp_task_sptr_->get_future();
        io_svc_.post([temp_task_sptr_](){
            (*temp_task_sptr_)();
        });
        return res;
    }

    asio::io_service &get_io_service()
    {
        return io_svc_;
    }

private:
    asio::io_service io_svc_;
    std::vector<thread_type> workers_;
    work_guard_type work_guard_;
};
}//namespace utility
}//namespace diy
#endif//!utility_include_utility_pool_threadpool_hpp