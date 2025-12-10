#include "ThreadPool.h"
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstring>
#include <iostream>
#include <chrono>

namespace fs = std::filesystem;

// 高性能文件读取器
class HighPerformanceFileReader {
private:
    ThreadPool& m_threadPool;
    const size_t m_blockSize;  // 每个线程处理的块大小

public:
    // 构造函数：线程池引用 + 块大小（默认8MB，可根据系统调整）
    HighPerformanceFileReader(ThreadPool& threadPool, size_t blockSize = 8 * 1024 * 1024)
        : m_threadPool(threadPool), m_blockSize(blockSize) {}

    // 读取文件到内存，返回完整数据
    std::vector<char> readFile(const std::string& filePath) {
        // 获取文件大小
        const auto fileSize = fs::file_size(filePath);
        if (fileSize == 0) {
            return {};
        }

        // 计算需要的块数
        const size_t blockCount = (fileSize + m_blockSize - 1) / m_blockSize;
        std::vector<char> result(fileSize);  // 预分配足够内存
        std::vector<std::future<void>> futures;
        futures.reserve(blockCount);

        // 提交所有读取任务
        for (size_t i = 0; i < blockCount; ++i) {
            const size_t offset = i * m_blockSize;
            const size_t currentBlockSize = std::min(m_blockSize, fileSize - offset);

            // 提交任务到线程池
            futures.emplace_back(m_threadPool.submit(
                &HighPerformanceFileReader::readBlock,
                filePath,
                offset,
                currentBlockSize,
                &result[offset]
            ));
        }

        // 等待所有任务完成
        for (auto& future : futures) {
            future.get();  // 会抛出异常，如果读取失败
        }

        return result;
    }

private:
    // 静态方法：读取文件的一块数据
    static void readBlock(const std::string& filePath, size_t offset, size_t size, char* dest) {
        // 以二进制模式打开文件，禁用流同步以提高性能
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("无法打开文件: " + filePath);
        }

        // 设置文件缓冲区大小（使用较大的缓冲区减少IO操作）
        const size_t bufferSize = 1 * 1024 * 1024;  // 1MB缓冲区
        std::vector<char> buffer(bufferSize);
        file.rdbuf()->pubsetbuf(buffer.data(), bufferSize);

        // 定位到要读取的位置
        file.seekg(offset);

        // 读取数据
        file.read(dest, size);

        // 检查读取是否成功
        if (!file) {
            throw std::runtime_error("文件读取失败: " + filePath +
                " (offset: " + std::to_string(offset) + ", size: " + std::to_string(size) + ")");
        }
    }
};

//// 使用示例
//int man___() {
//    try {
//        // 根据CPU核心数创建线程池（通常设置为核心数或核心数*2）
//        const size_t threadCount = std::thread::hardware_concurrency();
//        ThreadPool threadPool(threadCount);
//        threadPool.init();
//
//        HighPerformanceFileReader reader(threadPool, 16 * 1024 * 1024);  // 16MB块大小
//
//        const std::string filePath = "large_file.bin";  // 替换为你的大文件路径
//
//        // 计时
//        auto start = std::chrono::high_resolution_clock::now();
//
//        // 读取文件
//        auto data = reader.readFile(filePath);
//
//        auto end = std::chrono::high_resolution_clock::now();
//        std::chrono::duration<double> elapsed = end - start;
//
//        // 输出结果
//        std::cout << "文件大小: " << data.size() / (1024 * 1024) << " MB\n";
//        std::cout << "读取时间: " << elapsed.count() << " 秒\n";
//        std::cout << "读取速度: " << (data.size() / (1024 * 1024)) / elapsed.count() << " MB/s\n";
//
//        threadPool.shutdown();
//    }
//    catch (const std::exception& e) {
//        std::cerr << "错误: " << e.what() << std::endl;
//        return 1;
//    }
//
//    return 0;
//}