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

    if (NextCharEOF)
    {
        return "";
    }

    NextCharEOL = false;
    TokenIsString = false;

    std::string retval;

    if (!NextCharValid)
    {
        TheNextChar = fgetc(FileStream.get());
        NextCharValid = true;

        if (TheNextChar == EOF)
        {
            NextCharEOF = true;
        }
    }

    if (!NextCharEOF && TheNextChar == '#')
    {
        while ((!NextCharEOF) && (TheNextChar != '\n') && (TheNextChar != '\r'))
        {
            TheNextChar = fgetc(FileStream.get());
            if (TheNextChar == '\n' || TheNextChar == '\r')
            {
                NextCharEOL = true;
            }
        }
    }

    while (!NextCharEOF &&
        (TheNextChar == ' ' || TheNextChar == '\n' || TheNextChar == '\r' ||
            TheNextChar == '\t' || TheNextChar == '=' || TheNextChar == '(' ||
            TheNextChar == ')' || TheNextChar == ','))
    {
        if (TheNextChar == '\n' || TheNextChar == '\r')
        {
            NextCharEOL = true;
        }

        TheNextChar = fgetc(FileStream.get());
        if (TheNextChar == EOF)
        {
            NextCharEOF = true;
        }

        if (NextCharEOL)
        {
            return GetNextToken();
        }
    }

    if (TheNextChar == '\"')
    {
        TokenIsString = true;
        TheNextChar = fgetc(FileStream.get());
        if (TheNextChar == EOF)
        {
            NextCharEOF = true;
        }

        while (!NextCharEOF && TheNextChar != '\"')
        {
            retval += TheNextChar;
            TheNextChar = fgetc(FileStream.get());

            if (TheNextChar == EOF)
            {
                NextCharEOF = true;
            }
        }

        TheNextChar = fgetc(FileStream.get());
        if (TheNextChar == EOF)
        {
            NextCharEOF = true;
        }
    }
    else
    {
        while (!NextCharEOF &&
            (TheNextChar != ' ' && TheNextChar != '\n' && TheNextChar != '\r' &&
                TheNextChar != '\t' && TheNextChar != '=' && TheNextChar != '(' &&
                TheNextChar != ')' && TheNextChar != ','))
        {
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

    while (!NextCharEOF &&
        (TheNextChar == ' ' || TheNextChar == '\n' || TheNextChar == '\r' ||
            TheNextChar == '\t' || TheNextChar == '=' || TheNextChar == '(' ||
            TheNextChar == ')' || TheNextChar == ','))
    {
        if (TheNextChar == '\n' || TheNextChar == '\r')
        {
            NextCharEOL = true;
        }

        TheNextChar = fgetc(FileStream.get());
        if (TheNextChar == EOF)
        {
            NextCharEOF = true;
        }

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

    if (token == "TITLE" || token == "Title")
    {
        std::string next = GetNextToken();
        while (next == "=" || next == ":")
        {
            next = GetNextToken();
        }

        DataTitle = next;

        if (NextCharEOL)
        {
            NextCharEOL = false;
        }
    }
    else
    {
        DataTitle = token;
        TokenBackup = token; // �Żأ���Ϊ�����������ʱ���õ�
    }

    token = GetNextToken();
    // GetNextToken 会把非引号 token 大写化，这里统一比较全大写字面量
    // （兼容 Tecplot 常见的 "Varibles" 拼写错误）
    if (token == "VARIABLES" || token == "VARIBLES")
    {
        std::string next = GetNextToken();
        while (next == "=" || next == ":")
        {
            next = GetNextToken();
        }

        if (!next.empty() && next != "=" && next != ":")
        {
            Variables.push_back(next);
        }

        while (!NextCharEOL && !NextCharEOF)
        {
            token = GetNextToken();
            if (token.empty()) break;
            if (!token.empty())
            {
                Variables.push_back(token);
            }
        }

        NextCharEOL = false;
    }
    else
    {
        TokenBackup = token;
    }
    return true;
}

bool Reader::ParseData()
{
    NextCharEOL = false;

    while (!NextCharEOF)
    {
        std::vector<double> point;

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
                std::cerr << "Warning: Failed to convert token '" << token
                    << "' to number: " << e.what() << std::endl;
                break;
            }
        }

        if (point.empty() && NextCharEOL)
        {
            NextCharEOL = false;
            continue;
        }

        if (!point.empty())
        {
            if (Variables.empty())
            {
                for (size_t i = 0; i < point.size(); i++)
                {
                    Variables.push_back("Var_" + std::to_string(i + 1));
                }
            }

            if (point.size() != Variables.size())
            {
                std::cerr << "Warning: Point dimension mismatch. Expected "
                    << Variables.size() << " values, got "
                    << point.size() << ". Skipping point." << std::endl;
                continue;
            }

            Points.push_back(std::move(point));
        }

        NextCharEOL = false;
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

    if (!ParseHeader())
    {
        std::cerr << "Error: Failed to parse header of file '" << fileName << "'" << std::endl;
        FileStream.reset(nullptr);
        return false;
    }

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