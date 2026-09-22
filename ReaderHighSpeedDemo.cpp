#include "ThreadPool.h"
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstring>
#include <iostream>
#include <chrono>

namespace fs = std::filesystem;

class HighPerformanceFileReader {
private:
    ThreadPool& m_threadPool;
    const size_t m_blockSize;  
public:
    HighPerformanceFileReader(ThreadPool& threadPool, size_t blockSize = 8 * 1024 * 1024)
        : m_threadPool(threadPool), m_blockSize(blockSize) {}

    
    std::vector<char> readFile(const std::string& filePath) {
    
        const auto fileSize = fs::file_size(filePath);
        if (fileSize == 0) {
            return {};
        }

    
        const size_t blockCount = (fileSize + m_blockSize - 1) / m_blockSize;
        std::vector<char> result(fileSize);
        std::vector<std::future<void>> futures;
        futures.reserve(blockCount);

        
        for (size_t i = 0; i < blockCount; ++i) {
            const size_t offset = i * m_blockSize;
            const size_t currentBlockSize = std::min(m_blockSize, fileSize - offset);

        
            futures.emplace_back(m_threadPool.submit(
                &HighPerformanceFileReader::readBlock,
                filePath,
                offset,
                currentBlockSize,
                &result[offset]
            ));
        }

        
        for (auto& future : futures) {
            future.get();  
        }

        return result;
    }

private:
    
    static void readBlock(const std::string& filePath, size_t offset, size_t size, char* dest) {
    
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("aaa" + filePath);
        }

    
        const size_t bufferSize = 1 * 1024 * 1024;  
        std::vector<char> buffer(bufferSize);
        file.rdbuf()->pubsetbuf(buffer.data(), bufferSize);


        file.seekg(offset);


        file.read(dest, size);


        if (!file) {
            throw std::runtime_error("�ļ���ȡʧ��: " + filePath +
                " (offset: " + std::to_string(offset) + ", size: " + std::to_string(size) + ")");
        }
    }
};


int man___() {
    try {
        const size_t threadcount = std::thread::hardware_concurrency();
        ThreadPool threadpool(threadcount);
        threadpool.init();

        HighPerformanceFileReader reader(threadpool, 16 * 1024 * 1024);  

        const std::string filepath = "large_file.bin";  

        auto start = std::chrono::high_resolution_clock::now();

        auto data = reader.readFile(filepath);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        
        std::cout << "文件大小 " << data.size() / (1024 * 1024) << " mb\n";
        std::cout << "耗时: " << elapsed.count() << " ��\n";
        std::cout << "速度: " << (data.size() / (1024 * 1024)) / elapsed.count() << " mb/s\n";

        threadpool.shutdown();
    }
    catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}