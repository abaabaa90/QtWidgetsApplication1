#pragma once
#include <functional>
#include <future>
#include <mutex>
#include <deque>
#include <thread>
#include <utility>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <iostream>
#define DEBUG
// 基于 std::deque 实现的线程安全队列
template <typename T>
class SafeQueue {
private:
    std::deque<T> m_queue;  // 存储任务的双端队列
    std::mutex m_mutex;     // 保护队列访问的互斥锁

public:
    SafeQueue() = default;
    SafeQueue(const SafeQueue&) = delete;
    SafeQueue& operator=(const SafeQueue&) = delete;

    // 检查队列是否为空
    bool empty() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    // 获取队列大小
    int size() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    // 将任务添加到队列末尾（用于任务提交）
    void enqueue(T&& t) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push_back(std::move(t));  // 使用移动语义优化性能
    }

    // 从队列前端取出任务（用于本地线程获取自己的任务）
    bool dequeue(T& t) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        t = std::move(m_queue.front());  // 使用移动语义优化性能
        m_queue.pop_front();
        return true;
    }

    // 从队列后端窃取任务（用于工作窃取，避免与本地任务竞争）
    bool steal(T& t) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        t = std::move(m_queue.back());  // 从后端窃取，避免与dequeue操作冲突
        m_queue.pop_back();
        return true;
    }
};

class ThreadPool {
private:
    // 工作线程类，每个线程运行此对象的operator()
    class ThreadWorker {
    private:
        int m_id;          // 线程ID，用于索引其专属队列
        ThreadPool* m_pool; // 指向线程池的指针，用于访问共享资源

    public:
        ThreadWorker(ThreadPool* pool, const int id) : m_pool(pool), m_id(id) { }

        void operator()() {
            std::function<void()> func;  // 用于存储待执行的任务
            bool dequeued;               // 标记任务是否成功获取

            while (!m_pool->m_shutdown) {  // 主循环，直到线程池关闭
                // 第一步：尝试从自己的队列中获取任务（优先本地任务）
                if (m_pool->m_queues[m_id].dequeue(func)) {
                    dequeued = true;
                }
                // 第二步：如果本地队列为空，则尝试窃取其他线程的任务
                else {
                    dequeued = false;
                    for (int i = 0; i < m_pool->m_queues.size(); ++i) {
                        if (i == m_id) continue;  // 跳过自己的队列
                        if (m_pool->m_queues[i].steal(func)) {  // 从其他队列后端窃取
                            dequeued = true;
                            break;  // 成功窃取后立即退出循环
                        }
                    }
                }

                if (dequeued) {
                    func();  // 执行获取到的任务
                }
                else {
                    // 如果本地和窃取都失败，进入等待状态
                    std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);
                    m_pool->m_conditional_lock.wait(lock);  // 阻塞等待新任务通知
                }
            }
        }
    };

    bool m_shutdown;                                    // 线程池关闭标志
    std::vector<SafeQueue<std::function<void()>>> m_queues;  // 每个线程一个任务队列，实现工作窃取
    std::vector<std::thread> m_threads;                 // 工作线程池
    std::mutex m_conditional_mutex;                     // 保护条件变量的互斥锁
    std::condition_variable m_conditional_lock;         // 用于线程等待和唤醒的条件变量
    std::atomic<int> m_submit_index{ 0 };                // 原子计数器，用于轮询分配任务到不同队列

public:
    // 构造函数：初始化线程池，创建指定数量的队列和线程
    ThreadPool(const int n_threads)
        : m_shutdown(false),
        m_queues(n_threads),    // 为每个线程创建一个独立的队列
        m_threads(n_threads)    // 预分配线程容器空间
    {
#ifdef DEBUG
        std::cout << "[ThreadPool] 创建线程池，线程数量: " << n_threads << std::endl;
#endif
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    // 初始化线程池：启动所有工作线程
    void init() {
        for (int i = 0; i < m_threads.size(); ++i) {
            m_threads[i] = std::thread(ThreadWorker(this, i));  // 为每个线程创建ThreadWorker对象
        }
#ifdef DEBUG
        std::cout << "[ThreadPool] 所有线程已启动" << std::endl;
#endif
    }

    // 关闭线程池：设置关闭标志并等待所有线程结束
    void shutdown() {
        m_shutdown = true;  // 设置关闭标志，使所有工作线程退出主循环
        m_conditional_lock.notify_all();  // 唤醒所有等待的线程，避免它们永久阻塞
        for (int i = 0; i < m_threads.size(); ++i) {
            if (m_threads[i].joinable()) {
                m_threads[i].join();  // 等待每个线程完成并回收资源
            }
        }
#ifdef DEBUG
        std::cout << "[ThreadPool] 线程池已关闭，所有线程已结束" << std::endl;
#endif
    }

    // 提交任务到线程池
    // F: 任务函数类型, Args: 参数类型
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        // 推导任务的返回类型
        using return_type = decltype(f(args...));

        // 创建一个打包任务对象，将函数和参数绑定
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)  // 使用完美转发绑定函数和参数
        );

        // 将任务包装成void()函数，然后提交到队列
        // 使用原子计数器实现轮询分配，将任务均匀分发到不同队列
        int idx = m_submit_index.fetch_add(1) % m_queues.size();
        m_queues[idx].enqueue([task]() { (*task)(); });  // 捕获shared_ptr并执行任务

#ifdef DEBUG
        std::cout << "[ThreadPool] 任务已提交到队列 " << idx << " (提交线程ID: " << std::this_thread::get_id() << ")" << std::endl;
#endif

        m_conditional_lock.notify_one();  // 通知一个等待的线程有新任务到达
        return task->get_future();  // 返回future，调用者可通过它获取任务执行结果
    }
};