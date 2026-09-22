#pragma once
#include <string>
#include <vector>
#include <memory>

class Reader
{
public:
    Reader();
    ~Reader();

    bool ReadFile(const char* fileName);

    void Clear();

    size_t GetPointCount() const { return Points.size(); }
    size_t GetDimension() const { return Variables.size(); }
    std::string DataTitle;
    std::vector<std::string> Variables;
    std::vector < std::vector<double >> Points;
private:
    bool ParseHeader();
    bool ParseData();
    std::string GetNextToken();

    std::string TokenBackup;
    char TheNextChar;
    bool NextCharEOF;
    bool NextCharEOL;
    bool NextCharValid;
    bool TokenIsString;
    std::unique_ptr<FILE, decltype(&fclose)> FileStream;

    Reader(const Reader&) = delete;
    Reader& operator=(const Reader&) = delete;
};