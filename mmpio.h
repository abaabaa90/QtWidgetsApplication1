#include <cstddef>
#include <cstdint>
#include <optional>
#include <system_error>
#include <string>
#include <filesystem>
#include <cstddef>
#include <fstream>      
#include <iostream>     
#include <cstring>      

#define PLATFORM_WINDOWS 1
#include <windows.h>

namespace lightweight_mmap {

    class MMapIO {
    public:
        enum class AccessMode : uint8_t {
            ReadOnly,
            ReadWrite
        };

        MMapIO(const MMapIO&) = delete;
        MMapIO& operator=(const MMapIO&) = delete;

        MMapIO(MMapIO&& other) noexcept
            : m_data(other.m_data),
            m_size(other.m_size),
            m_mode(other.m_mode),
            m_hFile(other.m_hFile),
            m_hMap(other.m_hMap),
            m_valid(other.m_valid) {
            other.m_data = nullptr;
            other.m_size = 0;
            other.m_valid = false;
            other.m_hFile = INVALID_HANDLE_VALUE;
            other.m_hMap = nullptr;
        }

        MMapIO& operator=(MMapIO&& other) noexcept {
            if (this != &other) {
                release();

                m_data = other.m_data;
                m_size = other.m_size;
                m_mode = other.m_mode;
                m_hFile = other.m_hFile;
                m_hMap = other.m_hMap;
                m_valid = other.m_valid;

                other.m_data = nullptr;
                other.m_size = 0;
                other.m_valid = false;
                other.m_hFile = INVALID_HANDLE_VALUE;
                other.m_hMap = nullptr;
            }
            return *this;
        }

        ~MMapIO() {
            release();
        }

        std::byte* data() noexcept { return m_data; }
        const std::byte* data() const noexcept { return m_data; }

        size_t size() const noexcept { return m_size; }

        bool is_valid() const noexcept { return m_valid; }

        std::error_code sync() noexcept {
            if (!m_valid) {
                return std::error_code(EINVAL, std::system_category());
            }

            if (FlushViewOfFile(m_data, m_size)) {
                return {};
            }
            else {
                return std::error_code(GetLastError(), std::system_category());
            }
        }

        static std::optional<MMapIO> open(
            const std::filesystem::path& file_path,
            AccessMode mode = AccessMode::ReadOnly,
            std::error_code ec = std::error_code{}
        ) {
            ec.clear();
            MMapIO mmap_io;

            DWORD desired_access = (mode == AccessMode::ReadOnly) ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE);
            DWORD share_mode = FILE_SHARE_READ;
            DWORD creation_disposition = OPEN_EXISTING;
            DWORD flags = FILE_ATTRIBUTE_NORMAL;

            mmap_io.m_hFile = CreateFileW(
                file_path.c_str(),
                desired_access,
                share_mode,
                nullptr,
                creation_disposition,
                flags,
                nullptr
            );
            if (mmap_io.m_hFile == INVALID_HANDLE_VALUE) {
                ec = std::error_code(GetLastError(), std::system_category());
                return std::nullopt;
            }

            LARGE_INTEGER file_size;
            if (!GetFileSizeEx(mmap_io.m_hFile, &file_size)) {
                ec = std::error_code(GetLastError(), std::system_category());
                CloseHandle(mmap_io.m_hFile);
                mmap_io.m_hFile = INVALID_HANDLE_VALUE;
                return std::nullopt;
            }
            mmap_io.m_size = static_cast<size_t>(file_size.QuadPart);
            if (mmap_io.m_size == 0) {
                // 核心修复：用数值 158 替代 ERROR_EMPTY_FILE 宏
                ec = std::error_code(158, std::system_category());
                CloseHandle(mmap_io.m_hFile);
                mmap_io.m_hFile = INVALID_HANDLE_VALUE;
                return std::nullopt;
            }

            DWORD protect = (mode == AccessMode::ReadOnly) ? PAGE_READONLY : PAGE_READWRITE;
            mmap_io.m_hMap = CreateFileMappingW(
                mmap_io.m_hFile,
                nullptr,
                protect,
                0, 0,
                nullptr
            );
            if (mmap_io.m_hMap == nullptr) {
                ec = std::error_code(GetLastError(), std::system_category());
                CloseHandle(mmap_io.m_hFile);
                mmap_io.m_hFile = INVALID_HANDLE_VALUE;
                return std::nullopt;
            }

            DWORD map_access = (mode == AccessMode::ReadOnly) ? FILE_MAP_READ : FILE_MAP_WRITE;
            mmap_io.m_data = reinterpret_cast<std::byte*>(MapViewOfFile(
                mmap_io.m_hMap,
                map_access,
                0, 0,
                0
            ));
            if (mmap_io.m_data == nullptr) {
                ec = std::error_code(GetLastError(), std::system_category());
                CloseHandle(mmap_io.m_hMap);
                CloseHandle(mmap_io.m_hFile);
                mmap_io.m_hMap = nullptr;
                mmap_io.m_hFile = INVALID_HANDLE_VALUE;
                return std::nullopt;
            }

            mmap_io.m_mode = mode;
            mmap_io.m_valid = true;
            return std::make_optional(std::move(mmap_io));
        }

    private:
        MMapIO() noexcept
            : m_data(nullptr),
            m_size(0),
            m_mode(AccessMode::ReadOnly),
            m_hFile(INVALID_HANDLE_VALUE),
            m_hMap(nullptr),
            m_valid(false) {}

        void release() noexcept {
            if (!m_valid) return;

            if (m_data != nullptr) {
                UnmapViewOfFile(m_data);
                m_data = nullptr;
            }

            if (m_hMap != nullptr) {
                CloseHandle(m_hMap);
                m_hMap = nullptr;
            }
            if (m_hFile != INVALID_HANDLE_VALUE) {
                CloseHandle(m_hFile);
                m_hFile = INVALID_HANDLE_VALUE;
            }

            m_size = 0;
            m_valid = false;
        }

        std::byte* m_data;
        size_t m_size;
        AccessMode m_mode;
        HANDLE m_hFile;
        HANDLE m_hMap;
        bool m_valid;
    };

} 
// ========== 使用示例 ==========
//int qweasd() {
//    const std::filesystem::path test_file = L"test_mmap.dat";
//
//    {
//        std::ofstream fout(test_file, std::ios::binary | std::ios::trunc);
//        if (!fout.is_open()) {
//            std::cerr << "Failed to create test file" << std::endl;
//            return 1;
//        }
//        const std::string test_data = "Hello Lightweight MMap IO! C++17 (Windows 64bit)";
//        fout.write(test_data.data(), test_data.size());
//    }
//
//    std::error_code ec;
//    auto mmap_reader = lightweight_mmap::MMapIO::open(
//        test_file,
//        lightweight_mmap::MMapIO::AccessMode::ReadOnly,
//        ec
//    );
//    if (ec) {
//        std::cerr << "Open mmap for read failed: " << ec.message() << std::endl;
//        return 1;
//    }
//    if (mmap_reader.has_value() && mmap_reader->is_valid()) {
//        std::cout << "Read from mmap: "
//            << std::string(reinterpret_cast<const char*>(mmap_reader->data()), mmap_reader->size())
//            << std::endl;
//    }
//
//    auto mmap_writer = lightweight_mmap::MMapIO::open(
//        test_file,
//        lightweight_mmap::MMapIO::AccessMode::ReadWrite,
//        ec
//    );
//    if (ec) {
//        std::cerr << "Open mmap for write failed: " << ec.message() << std::endl;
//        return 1;
//    }
//    if (mmap_writer.has_value() && mmap_writer->is_valid()) {
//        const std::string new_data = "Modified by Windows MMap IO! C++17 (64bit)";
//        std::memcpy(mmap_writer->data(), new_data.data(), new_data.size());
//
//        ec = mmap_writer->sync();
//        if (ec) {
//            std::cerr << "Sync mmap failed: " << ec.message() << std::endl;
//        }
//        else {
//            std::cout << "Write to mmap success, sync to file done" << std::endl;
//        }
//    }
//
//    auto mmap_verify = lightweight_mmap::MMapIO::open(test_file, lightweight_mmap::MMapIO::AccessMode::ReadOnly, ec);
//    if (mmap_verify.has_value() && mmap_verify->is_valid()) {
//        std::cout << "Verify modified data: "
//            << std::string(reinterpret_cast<const char*>(mmap_verify->data()), mmap_verify->size())
//            << std::endl;
//    }
//
//    std::filesystem::remove(test_file);
//
//}