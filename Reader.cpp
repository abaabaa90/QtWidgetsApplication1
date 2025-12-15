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

    // ����Ѿ������ļ�ĩβ��EOF����������Ȼ�е���������һ����ǣ��򷵻ؿ��ַ���
    if (NextCharEOF)
    {
        return "";
    }

    // �����н�����־���ַ�����Ǳ�־
    NextCharEOL = false;
    TokenIsString = false;

    std::string retval;

    // �����һ���ַ���Ч������δ���ļ����ж�ȡ�ַ��������ȡһ���ַ�
    if (!NextCharValid)
    {
        TheNextChar = fgetc(FileStream.get());
        NextCharValid = true;

        // ����ȡ���Ƿ񵽴��ļ�ĩβ
        if (TheNextChar == EOF)
        {
            NextCharEOF = true;
        }
    }

    // �����ǰ�ַ���ע�ͷ���'#'������������ֱ���������з�
    if (!NextCharEOF && TheNextChar == '#')
    {
        // ѭ��ֱ���������з����ļ�����
        while ((!NextCharEOF) && (TheNextChar != '\n') && (TheNextChar != '\r'))
        {
            TheNextChar = fgetc(FileStream.get());
            // ����������з��������н�����־
            if (TheNextChar == '\n' || TheNextChar == '\r')
            {
                NextCharEOL = true;
            }
        }
    }

    // �������֮��Ŀհ��ַ��������ո񡢻��з����Ʊ���Լ�һЩ����ָ��
    while (!NextCharEOF &&
        (TheNextChar == ' ' || TheNextChar == '\n' || TheNextChar == '\r' ||
            TheNextChar == '\t' || TheNextChar == '=' || TheNextChar == '(' ||
            TheNextChar == ')' || TheNextChar == ','))
    {
        // ����������з��������н�����־
        if (TheNextChar == '\n' || TheNextChar == '\r')
        {
            NextCharEOL = true;
        }

        // ��ȡ��һ���ַ�
        TheNextChar = fgetc(FileStream.get());
        // ����ȡ���Ƿ񵽴��ļ�ĩβ
        if (TheNextChar == EOF)
        {
            NextCharEOF = true;
        }

        // ����Ѿ������н��������ҵ�ǰ���������հ��ַ���
        // ��ݹ����GetNextToken()����ȡ��һ���ǿհױ��
        // �������Ժ��Կ��У���Ϊ���ǲ����ر��
        if (NextCharEOL)
        {
            return GetNextToken();
        }
    }

    // �����ǰ�ַ���˫���ţ������ַ������
    if (TheNextChar == '\"')
    {
        TokenIsString = true;
        // ��ȡ˫���ź�ĵ�һ���ַ�
        TheNextChar = fgetc(FileStream.get());
        // ����Ƿ񵽴��ļ�ĩβ
        if (TheNextChar == EOF)
        {
            NextCharEOF = true;
        }

        // ѭ����ȡ�ַ���ֱ������������˫����
        while (!NextCharEOF && TheNextChar != '\"')
        {
            retval += TheNextChar;
            TheNextChar = fgetc(FileStream.get());

            if (TheNextChar == EOF)
            {
                NextCharEOF = true;
            }
        }

        // ��ȡ����˫���ź����һ���ַ���Ϊ��һ�ζ�ȡ��׼����
        TheNextChar = fgetc(FileStream.get());
        if (TheNextChar == EOF)
        {
            NextCharEOF = true;
        }
    }
    else
    {
        // ������ͨ��ǣ����ַ�����
        // ѭ����ȡ�ַ���ֱ�������հ��ַ���ָ��
        while (!NextCharEOF &&
            (TheNextChar != ' ' && TheNextChar != '\n' && TheNextChar != '\r' &&
                TheNextChar != '\t' && TheNextChar != '=' && TheNextChar != '(' &&
                TheNextChar != ')' && TheNextChar != ','))
        {
            // ����ַ���Сд��ĸ����ת��Ϊ��д��ʵ�ִ�Сд����У�
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

    // ������Ǻ�Ŀհ��ַ��������ָ������ֱ�������н���������һ����ǵĿ�ʼ
    while (!NextCharEOF &&
        (TheNextChar == ' ' || TheNextChar == '\n' || TheNextChar == '\r' ||
            TheNextChar == '\t' || TheNextChar == '=' || TheNextChar == '(' ||
            TheNextChar == ')' || TheNextChar == ','))
    {
        // ����������з��������н�����־
        if (TheNextChar == '\n' || TheNextChar == '\r')
        {
            NextCharEOL = true;
        }

        TheNextChar = fgetc(FileStream.get());
        if (TheNextChar == EOF)
        {
            NextCharEOF = true;
        }

        // ����Ѿ������н�����������ѭ��
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

    // �������� - ֧�� Title = "test" ��ʽ
    if (token == "TITLE" || token == "Title")
    {
        // �������ܵĵȺŻ�ð��
        std::string next = GetNextToken();
        while (next == "=" || next == ":")
        {
            next = GetNextToken();
        }

        // ��ȡ�������ݣ������Ǵ�˫���ŵ��ַ�����
        DataTitle = next;

        // ������β��׼����ȡ��һ��
        if (NextCharEOL)
        {
            NextCharEOL = false;
        }
    }
    else
    {
        // ���û����ȷ��ʶ�������һ��token�Ǳ���
        DataTitle = token;
        TokenBackup = token; // �Żأ���Ϊ�����������ʱ���õ�
    }

    // �������� - ֧�� Varibles = "X[m]","Y[m]" ��ʽ
    // ע�⣺ԭ������Variables������Ϊ�˼���ƴд����Ҳ����Varibles
    token = GetNextToken();
    if (token == "VARIABLES" || token == "Variables" || token == "Varibles")
    {
        // �������ܵĵȺŻ�ð��
        std::string next = GetNextToken();
        while (next == "=" || next == ":")
        {
            next = GetNextToken();
        }

        // ������ǵȺŻ�ð�ţ����������ǵ�һ��������
        if (!next.empty() && next != "=" && next != ":")
        {
            Variables.push_back(next);
        }

        // ������ȡ����ֱ���н���
        while (!NextCharEOL && !NextCharEOF)
        {
            token = GetNextToken();
            if (token.empty()) break;
            if (!token.empty())
            {
                Variables.push_back(token);
            }
        }

        // �����н�����־
        NextCharEOL = false;
    }
    else
    {
        // ������Ǳ����У������������еĿ�ʼ
        TokenBackup = token;
    }
    return true;
}

bool Reader::ParseData()
{
    NextCharEOL = false; // �����н�����־��Ϊ��ȡ������׼��

    // ��ȡ���ݵ�
    while (!NextCharEOF)
    {
        std::vector<double> point;

        // ��ȡһ���е�������ֵ
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
                // ת��ʧ�ܵĴ��������token������Ч�������ַ�����
                std::cerr << "Warning: Failed to convert token '" << token
                    << "' to number: " << e.what() << std::endl;
                break;
            }
        }

        // ��������
        if (point.empty() && NextCharEOL)
        {
            NextCharEOL = false;
            continue;
        }

        // ������Ч��
        if (!point.empty())
        {
            // ���������δ֪�����ݵ�һ����ȷ��
            if (Variables.empty())
            {
                for (size_t i = 0; i < point.size(); i++)
                {
                    Variables.push_back("Var_" + std::to_string(i + 1));
                }
            }

            // ����ά���Ƿ�ƥ�������
            if (point.size() != Variables.size())
            {
                std::cerr << "Warning: Point dimension mismatch. Expected "
                    << Variables.size() << " values, got "
                    << point.size() << ". Skipping point." << std::endl;
                continue;
            }

            Points.push_back(std::move(point));
        }

        NextCharEOL = false; // ����Ϊ��һ����׼��
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

    // ����ͷ��
    if (!ParseHeader())
    {
        std::cerr << "Error: Failed to parse header of file '" << fileName << "'" << std::endl;
        FileStream.reset(nullptr);
        return false;
    }

    // ��������
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