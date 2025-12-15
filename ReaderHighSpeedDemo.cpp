#include "ThreadPool.h"
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstring>
#include <iostream>
#include <chrono>

namespace fs = std::filesystem;

// �������ļ���ȡ��
class HighPerformanceFileReader {
private:
    ThreadPool& m_threadPool;
    const size_t m_blockSize;  // ÿ���̴߳���Ŀ��С

public:
    // ���캯�����̳߳����� + ���С��Ĭ��8MB���ɸ���ϵͳ������
    HighPerformanceFileReader(ThreadPool& threadPool, size_t blockSize = 8 * 1024 * 1024)
        : m_threadPool(threadPool), m_blockSize(blockSize) {}

    // ��ȡ�ļ����ڴ棬������������
    std::vector<char> readFile(const std::string& filePath) {
        // ��ȡ�ļ���С
        const auto fileSize = fs::file_size(filePath);
        if (fileSize == 0) {
            return {};
        }

        // ������Ҫ�Ŀ���
        const size_t blockCount = (fileSize + m_blockSize - 1) / m_blockSize;
        std::vector<char> result(fileSize);  // Ԥ�����㹻�ڴ�
        std::vector<std::future<void>> futures;
        futures.reserve(blockCount);

        // �ύ���ж�ȡ����
        for (size_t i = 0; i < blockCount; ++i) {
            const size_t offset = i * m_blockSize;
            const size_t currentBlockSize = std::min(m_blockSize, fileSize - offset);

            // �ύ�����̳߳�
            futures.emplace_back(m_threadPool.submit(
                &HighPerformanceFileReader::readBlock,
                filePath,
                offset,
                currentBlockSize,
                &result[offset]
            ));
        }

        // �ȴ������������
        for (auto& future : futures) {
            future.get();  // ���׳��쳣�������ȡʧ��
        }

        return result;
    }

private:
    // ��̬��������ȡ�ļ���һ������
    static void readBlock(const std::string& filePath, size_t offset, size_t size, char* dest) {
        // �Զ�����ģʽ���ļ���������ͬ�����������
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("�޷����ļ�: " + filePath);
        }

        // �����ļ���������С��ʹ�ýϴ�Ļ���������IO������
        const size_t bufferSize = 1 * 1024 * 1024;  // 1MB������
        std::vector<char> buffer(bufferSize);
        file.rdbuf()->pubsetbuf(buffer.data(), bufferSize);

        // ��λ��Ҫ��ȡ��λ��
        file.seekg(offset);

        // ��ȡ����
        file.read(dest, size);

        // ����ȡ�Ƿ�ɹ�
        if (!file) {
            throw std::runtime_error("�ļ���ȡʧ��: " + filePath +
                " (offset: " + std::to_string(offset) + ", size: " + std::to_string(size) + ")");
        }
    }
};

//// ʹ��ʾ��
//int man___() {
//    try {
//        // ����CPU�����������̳߳أ�ͨ������Ϊ�������������*2��
//        const size_t threadCount = std::thread::hardware_concurrency();
//        ThreadPool threadPool(threadCount);
//        threadPool.init();
//
//        HighPerformanceFileReader reader(threadPool, 16 * 1024 * 1024);  // 16MB���С
//
//        const std::string filePath = "large_file.bin";  // �滻Ϊ��Ĵ��ļ�·��
//
//        // ��ʱ
//        auto start = std::chrono::high_resolution_clock::now();
//
//        // ��ȡ�ļ�
//        auto data = reader.readFile(filePath);
//
//        auto end = std::chrono::high_resolution_clock::now();
//        std::chrono::duration<double> elapsed = end - start;
//
//        // ������
//        std::cout << "�ļ���С: " << data.size() / (1024 * 1024) << " MB\n";
//        std::cout << "��ȡʱ��: " << elapsed.count() << " ��\n";
//        std::cout << "��ȡ�ٶ�: " << (data.size() / (1024 * 1024)) / elapsed.count() << " MB/s\n";
//
//        threadPool.shutdown();
//    }
//    catch (const std::exception& e) {
//        std::cerr << "����: " << e.what() << std::endl;
//        return 1;
//    }
//
//    return 0;
//}