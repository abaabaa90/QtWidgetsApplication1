#pragma once
#include <string>
#include <vector>
#include <memory>

class Reader
{
public:
    Reader();
    ~Reader();

    // 读取文件
    bool ReadFile(const char* fileName);
    // 清空当前数据
    void Clear();

    // 获取统计信息
    size_t GetPointCount() const { return Points.size(); }
    size_t GetDimension() const { return Variables.size(); }
    // 数据存储
    std::string DataTitle;
    std::vector<std::string> Variables;
    std::vector < std::vector<double >> Points;
private:
    // 内部解析方法
    bool ParseHeader();
    bool ParseData();
    std::string GetNextToken();

    // 文件读取相关成员变量（按照你提供的GetNextToken函数需要）
    std::string TokenBackup;
    char TheNextChar;
    bool NextCharEOF;
    bool NextCharEOL;
    bool NextCharValid;
    bool TokenIsString;
    // 文件流（使用智能指针管理）
    std::unique_ptr<FILE, decltype(&fclose)> FileStream;

    // 禁用拷贝
    Reader(const Reader&) = delete;
    Reader& operator=(const Reader&) = delete;
};