#include "thread_pool.h"
#include <vector>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <memory>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <system_error>

#ifdef __linux__
#include <linux/fs.h>   // 用于 FIBMAP，获取物理块信息（可选）
#include <sys/ioctl.h>
#include <fcntl.h>      // 用于 O_DIRECT
#include <unistd.h>
#endif

class HighPerfFileReader {
private:
    BS::thread_pool m_work_pool;   // 用于处理数据的线程池
    BS::thread_pool m_io_pool;     // 可选的专用I/O线程池（1-2个线程）

    struct FileChunk {
        std::unique_ptr<char, void(*)(void*)> data; // 对齐的内存
        size_t size;
        size_t file_offset;
        std::chrono::high_resolution_clock::time_point read_start;
        bool is_valid;

        // 用于对齐内存的分配器
        static void* allocate_aligned(size_t size, size_t alignment) {
            void* ptr = nullptr;
#ifdef _WIN32
            ptr = _aligned_malloc(size, alignment);
#elif defined(__linux__)
            if (posix_memalign(&ptr, alignment, size) != 0) {
                ptr = nullptr;
            }
#endif
            return ptr;
        }

        FileChunk(size_t chunk_size, size_t offset, size_t alignment = 4096)
            : data(nullptr, [](void* p) {
#ifdef _WIN32
            _aligned_free(p);
#elif defined(__linux__)
            free(p);
#endif
                })
            , size(chunk_size)
                    , file_offset(offset)
                    , is_valid(false)
        {
            void* raw_ptr = allocate_aligned(chunk_size, alignment);
            if (raw_ptr) {
                data.reset(raw_ptr);
            }
            else {
                throw std::bad_alloc();
            }
        }
    };

public:
    // 构造函数，可指定工作线程数和专用I/O线程数
    HighPerfFileReader(size_t work_threads = std::thread::hardware_concurrency(),
        size_t io_threads = 1)
        : m_work_pool(work_threads)
        , m_io_pool(io_threads)
    {
        // 可以在这里初始化性能监控
    }

    // 核心读取函数
    template<typename DataProcessor>
    void read_and_process(const std::string& filepath,
        DataProcessor processor,
        size_t suggested_chunk_size = 4 * 1024 * 1024) // 默认4MB
    {
        // 1. 获取文件大小和最佳分块
        size_t file_size = get_file_size(filepath);
        size_t optimal_chunk = determine_optimal_chunk(file_size, suggested_chunk_size);
        size_t num_chunks = (file_size + optimal_chunk - 1) / optimal_chunk;

        std::cout << "[Info] 文件大小: " << file_size / (1024 * 1024) << " MB, "
            << "分块数: " << num_chunks << ", "
            << "每块: " << optimal_chunk / 1024 << " KB" << std::endl;

        // 2. 创建预读队列（生产者-消费者模型）
        std::queue<std::future<std::unique_ptr<FileChunk>>> prefetch_queue;
        std::mutex queue_mutex;
        std::condition_variable queue_cv;
        const size_t max_prefetch = 4; // 预读深度，可根据IOPS调整

        // 3. 启动预读生产者（在专用I/O线程中）
        std::atomic<size_t> next_chunk_to_read{ 0 };
        std::atomic<bool> reading_done{ false };

        auto prefetch_producer = [&]() {
            while (true) {
                size_t chunk_idx = next_chunk_to_read.fetch_add(1);
                if (chunk_idx >= num_chunks) {
                    reading_done = true;
                    queue_cv.notify_all();
                    break;
                }

                // 计算当前块的偏移和实际大小
                size_t offset = chunk_idx * optimal_chunk;
                size_t actual_size = (chunk_idx == num_chunks - 1)
                    ? (file_size - offset)
                    : optimal_chunk;

                // 提交异步读取任务（仍然同步读取，但由独立线程执行）
                auto read_future = m_io_pool.submit([=, &filepath]()
                    -> std::unique_ptr<FileChunk>
                    {
                        auto chunk = std::make_unique<FileChunk>(actual_size, offset);
                        chunk->read_start = std::chrono::high_resolution_clock::now();

                        // 使用直接I/O读取（Linux）
#ifdef __linux__
                        int fd = open(filepath.c_str(), O_RDONLY | O_DIRECT);
                        if (fd >= 0) {
                            ssize_t bytes_read = pread(fd, chunk->data.get(), actual_size, offset);
                            close(fd);
                            chunk->is_valid = (bytes_read == static_cast<ssize_t>(actual_size));
                        }
                        else {
                            // 回退到标准I/O
                            std::ifstream file(filepath, std::ios::binary);
                            file.seekg(offset);
                            file.read(chunk->data.get(), actual_size);
                            chunk->is_valid = !file.fail();
                        }
#else
                // Windows或其他系统使用标准I/O
                        std::ifstream file(filepath, std::ios::binary);
                        file.seekg(offset);
                        file.read(chunk->data.get(), actual_size);
                        chunk->is_valid = !file.fail();
#endif
                        return chunk;
                    });

                // 将future放入队列
                {
                    std::unique_lock lock(queue_mutex);
                    prefetch_queue.push(std::move(read_future));
                    queue_cv.notify_one(); // 通知消费者
                }

                // 控制预读深度，避免内存占用过大
                {
                    std::unique_lock lock(queue_mutex);
                    queue_cv.wait(lock, [&]() {
                        return prefetch_queue.size() < max_prefetch || reading_done;
                        });
                }
            }
            };

        // 4. 启动处理消费者（在工作线程池中并行处理）
        std::vector<std::future<void>> processing_tasks;
        std::atomic<size_t> chunks_processed{ 0 };

        auto processing_consumer = [&]() {
            while (true) {
                std::future<std::unique_ptr<FileChunk>> chunk_future;

                // 从队列中取出一个已读取完成的块
                {
                    std::unique_lock lock(queue_mutex);
                    queue_cv.wait(lock, [&]() {
                        return !prefetch_queue.empty() ||
                            (reading_done && prefetch_queue.empty());
                        });

                    if (prefetch_queue.empty() && reading_done) {
                        break; // 所有任务完成
                    }

                    chunk_future = std::move(prefetch_queue.front());
                    prefetch_queue.pop();
                    queue_cv.notify_one(); // 通知生产者可以继续读取
                }

                // 获取块数据（这里会等待读取完成）
                auto chunk = chunk_future.get();
                if (!chunk || !chunk->is_valid) {
                    throw std::runtime_error("文件读取失败");
                }

                // 记录读取耗时（用于监控）
                auto read_end = std::chrono::high_resolution_clock::now();
                auto read_duration = std::chrono::duration_cast<std::chrono::microseconds>(
                    read_end - chunk->read_start).count();

                // 提交数据处理任务到工作线程池
                processing_tasks.push_back(
                    m_work_pool.submit([chunk = std::move(chunk),
                        processor,
                        read_duration]()
                        {
                            // 调用用户的数据处理函数
                            processor(chunk->data.get(), chunk->size, chunk->file_offset);

                            // 可在此处收集性能统计信息
                            // std::cout << "块处理完成，读取耗时: " << read_duration << "μs" << std::endl;
                        })
                );

                chunks_processed++;
            }
            };

        // 5. 运行：1个生产者线程 + 多个消费者线程
        auto producer_future = std::async(std::launch::async, prefetch_producer);

        // 启动多个消费者（数量通常等于工作线程数）
        std::vector<std::future<void>> consumer_futures;
        for (size_t i = 0; i < m_work_pool.get_thread_count(); ++i) {
            consumer_futures.push_back(std::async(std::launch::async, processing_consumer));
        }

        // 等待所有任务完成
        producer_future.wait();
        for (auto& f : consumer_futures) f.wait();
        for (auto& f : processing_tasks) f.wait();

        std::cout << "[Info] 处理完成。总共处理 " << chunks_processed.load() << " 个数据块。" << std::endl;
    }

private:
    // 获取文件大小
    size_t get_file_size(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file) {
            throw std::runtime_error("无法打开文件: " + filepath);
        }
        return file.tellg();
    }

    // 动态确定最佳分块大小（可扩展为基于历史性能数据）
    size_t determine_optimal_chunk(size_t file_size, size_t suggested) {
        // 简单启发式规则：
        // 1. 最小为64KB（SSD的典型最佳值）
        // 2. 最大不超过16MB（避免内存压力和缓存不友好）
        // 3. 尝试让块数量接近CPU核心数的2-4倍
        size_t min_chunk = 64 * 1024;      // 64KB
        size_t max_chunk = 16 * 1024 * 1024; // 16MB

        size_t chunk = std::clamp(suggested, min_chunk, max_chunk);

        // 根据文件大小微调
        size_t num_chunks = file_size / chunk;
        size_t desired_chunks = m_work_pool.get_thread_count() * 3; // 每个核心3个块

        if (num_chunks < m_work_pool.get_thread_count()) {
            // 文件很小，减少块大小以增加并行度
            chunk = file_size / m_work_pool.get_thread_count();
            chunk = (chunk + 4095) & ~4095; // 对齐到4K
            return std::max(chunk, min_chunk);
        }

        return chunk;
    }
};