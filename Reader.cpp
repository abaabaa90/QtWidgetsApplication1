#include "Reader.h"
#include <cstdio>
#include <cstring>
#include <cctype>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <iostream>
#include <cstdlib>

Reader::Reader()
    : FileStream(nullptr, &fclose)
    , TheNextChar('\0')
    , NextCharEOF(false)
    , NextCharEOL(false)
    , NextCharValid(false)
    , TokenIsString(false)
{
    Clear();
}

Reader::~Reader()
{
    Clear();
}

void Reader::Clear()
{
    DataTitle.clear();
    Variables.clear();
    Points.clear();
    TokenBackup.clear();
    TheNextChar = '\0';
    NextCharEOF = false;
    NextCharEOL = false;
    NextCharValid = false;
    TokenIsString = false;
}

std::string Reader::GetNextToken()
{
    if (!TokenBackup.empty())
    {
        std::string retval = TokenBackup;
        TokenBackup.clear();
        return retval;
    }

    // 如果已经到达文件末尾（EOF），并且仍然有调用请求下一个标记，则返回空字符串
    if (NextCharEOF)
    {
        return "";
    }

    // 重置行结束标志和字符串标记标志
    NextCharEOL = false;
    TokenIsString = false;

    std::string retval;

    // 如果下一个字符无效（即尚未从文件流中读取字符），则读取一个字符
    if (!NextCharValid)
    {
        TheNextChar = fgetc(FileStream.get());
        NextCharValid = true;

        // 检查读取后是否到达文件末尾
        if (TheNextChar == EOF)
        {
            NextCharEOF = true;
        }
    }

    // 如果当前字符是注释符号'#'，则跳过该行直到遇到换行符
    if (!NextCharEOF && TheNextChar == '#')
    {
        // 循环直到遇到换行符或文件结束
        while ((!NextCharEOF) && (TheNextChar != '\n') && (TheNextChar != '\r'))
        {
            TheNextChar = fgetc(FileStream.get());
            // 如果遇到换行符，设置行结束标志
            if (TheNextChar == '\n' || TheNextChar == '\r')
            {
                NextCharEOL = true;
            }
        }
    }

    // 跳过标记之间的空白字符，包括空格、换行符、制表符以及一些特殊分隔符
    while (!NextCharEOF &&
        (TheNextChar == ' ' || TheNextChar == '\n' || TheNextChar == '\r' ||
            TheNextChar == '\t' || TheNextChar == '=' || TheNextChar == '(' ||
            TheNextChar == ')' || TheNextChar == ','))
    {
        // 如果遇到换行符，设置行结束标志
        if (TheNextChar == '\n' || TheNextChar == '\r')
        {
            NextCharEOL = true;
        }

        // 读取下一个字符
        TheNextChar = fgetc(FileStream.get());
        // 检查读取后是否到达文件末尾
        if (TheNextChar == EOF)
        {
            NextCharEOF = true;
        }

        // 如果已经遇到行结束，并且当前正在跳过空白字符，
        // 则递归调用GetNextToken()来获取下一个非空白标记
        // 这样可以忽略空行，因为它们不返回标记
        if (NextCharEOL)
        {
            return GetNextToken();
        }
    }

    // 如果当前字符是双引号，则处理字符串标记
    if (TheNextChar == '\"')
    {
        TokenIsString = true;
        // 读取双引号后的第一个字符
        TheNextChar = fgetc(FileStream.get());
        // 检查是否到达文件末尾
        if (TheNextChar == EOF)
        {
            NextCharEOF = true;
        }

        // 循环读取字符，直到遇到结束的双引号
        while (!NextCharEOF && TheNextChar != '\"')
        {
            retval += TheNextChar;
            TheNextChar = fgetc(FileStream.get());

            if (TheNextChar == EOF)
            {
                NextCharEOF = true;
            }
        }

        // 读取结束双引号后的下一个字符（为下一次读取做准备）
        TheNextChar = fgetc(FileStream.get());
        if (TheNextChar == EOF)
        {
            NextCharEOF = true;
        }
    }
    else
    {
        // 处理普通标记（非字符串）
        // 循环读取字符，直到遇到空白字符或分隔符
        while (!NextCharEOF &&
            (TheNextChar != ' ' && TheNextChar != '\n' && TheNextChar != '\r' &&
                TheNextChar != '\t' && TheNextChar != '=' && TheNextChar != '(' &&
                TheNextChar != ')' && TheNextChar != ','))
        {
            // 如果字符是小写字母，则转换为大写（实现大小写不敏感）
            if (TheNextChar >= 'a' && TheNextChar <= 'z')
            {
                TheNextChar += (int('A') - int('a'));
            }

            retval += TheNextChar;
            TheNextChar = fgetc(FileStream.get());

            if (TheNextChar == EOF)
            {
                NextCharEOF = true;
            }
        }
    }

    // 跳过标记后的空白字符（包括分隔符），直到遇到行结束符或下一个标记的开始
    while (!NextCharEOF &&
        (TheNextChar == ' ' || TheNextChar == '\n' || TheNextChar == '\r' ||
            TheNextChar == '\t' || TheNextChar == '=' || TheNextChar == '(' ||
            TheNextChar == ')' || TheNextChar == ','))
    {
        // 如果遇到换行符，设置行结束标志
        if (TheNextChar == '\n' || TheNextChar == '\r')
        {
            NextCharEOL = true;
        }

        TheNextChar = fgetc(FileStream.get());
        if (TheNextChar == EOF)
        {
            NextCharEOF = true;
        }

        // 如果已经遇到行结束，则跳出循环
        if (NextCharEOL)
        {
            break;
        }
    }

    return retval;
}

bool Reader::ParseHeader()
{
    std::string token = GetNextToken();

    if (token.empty()) return false;

    // 解析标题 - 支持 Title = "test" 格式
    if (token == "TITLE" || token == "Title")
    {
        // 跳过可能的等号或冒号
        std::string next = GetNextToken();
        while (next == "=" || next == ":")
        {
            next = GetNextToken();
        }

        // 获取标题内容（可能是带双引号的字符串）
        DataTitle = next;

        // 跳过行尾，准备读取下一行
        if (NextCharEOL)
        {
            NextCharEOL = false;
        }
    }
    else
    {
        // 如果没有明确标识，假设第一个token是标题
        DataTitle = token;
        TokenBackup = token; // 放回，因为后面解析变量时会用到
    }

    // 解析变量 - 支持 Varibles = "X[m]","Y[m]" 格式
    // 注意：原单词是Variables，这里为了兼容拼写错误，也接受Varibles
    token = GetNextToken();
    if (token == "VARIABLES" || token == "Variables" || token == "Varibles")
    {
        // 跳过可能的等号或冒号
        std::string next = GetNextToken();
        while (next == "=" || next == ":")
        {
            next = GetNextToken();
        }

        // 如果不是等号或冒号，则它可能是第一个变量名
        if (!next.empty() && next != "=" && next != ":")
        {
            Variables.push_back(next);
        }

        // 继续读取变量直到行结束
        while (!NextCharEOL && !NextCharEOF)
        {
            token = GetNextToken();
            if (token.empty()) break;
            if (!token.empty())
            {
                Variables.push_back(token);
            }
        }

        // 跳过行结束标志
        NextCharEOL = false;
    }
    else
    {
        // 如果不是变量行，可能是数据行的开始
        TokenBackup = token;
    }
    return true;
}

bool Reader::ParseData()
{
    NextCharEOL = false; // 重置行结束标志，为读取数据做准备

    // 读取数据点
    while (!NextCharEOF)
    {
        std::vector<double> point;

        // 读取一行中的所有数值
        while (!NextCharEOL && !NextCharEOF)
        {
            std::string token = GetNextToken();
            if (token.empty()) break;

            try
            {
                point.push_back(std::stod(token));
            }
            catch (const std::exception& e)
            {
                // 转换失败的处理（例如token不是有效的数字字符串）
                std::cerr << "Warning: Failed to convert token '" << token
                    << "' to number: " << e.what() << std::endl;
                break;
            }
        }

        // 跳过空行
        if (point.empty() && NextCharEOL)
        {
            NextCharEOL = false;
            continue;
        }

        // 保存有效点
        if (!point.empty())
        {
            // 如果变量数未知，根据第一个点确定
            if (Variables.empty())
            {
                for (size_t i = 0; i < point.size(); i++)
                {
                    Variables.push_back("Var_" + std::to_string(i + 1));
                }
            }

            // 检查点维度是否匹配变量数
            if (point.size() != Variables.size())
            {
                std::cerr << "Warning: Point dimension mismatch. Expected "
                    << Variables.size() << " values, got "
                    << point.size() << ". Skipping point." << std::endl;
                continue;
            }

            Points.push_back(std::move(point));
        }

        NextCharEOL = false; // 重置为下一行做准备
    }

    return !Points.empty();
}

bool Reader::ReadFile(const char* fileName)
{
    Clear();

    
    FILE* file = nullptr;
    errno_t err = fopen_s(&file, fileName, "r");
    if (err != 0 || !file)
    {
        std::cerr << "Error: Failed to open file '" << fileName << "'" << std::endl;
        return false;
    }
    FileStream.reset(file);

    // 解析头部
    if (!ParseHeader())
    {
        std::cerr << "Error: Failed to parse header of file '" << fileName << "'" << std::endl;
        FileStream.reset(nullptr);
        return false;
    }

    // 解析数据
    if (!ParseData())
    {
        std::cerr << "Error: Failed to parse data in file '" << fileName << "'" << std::endl;
        FileStream.reset(nullptr);
        return false;
    }

    FileStream.reset(nullptr);

    std::cout << "Successfully read file '" << fileName << "': "
        << Points.size() << " points, "
        << Variables.size() << " dimensions" << std::endl;

    return true;
}