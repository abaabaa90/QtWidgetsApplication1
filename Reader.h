#pragma once
#include <string>
#include <vector>
#include <memory>

class Reader
{
public:
    Reader();
    ~Reader();

    // ��ȡ�ļ�
    bool ReadFile(const char* fileName);
    // ��յ�ǰ����
    void Clear();

    // ��ȡͳ����Ϣ
    size_t GetPointCount() const { return Points.size(); }
    size_t GetDimension() const { return Variables.size(); }
    // ���ݴ洢
    std::string DataTitle;
    std::vector<std::string> Variables;
    std::vector < std::vector<double >> Points;
private:
    // �ڲ���������
    bool ParseHeader();
    bool ParseData();
    std::string GetNextToken();

    // �ļ���ȡ��س�Ա�������������ṩ��GetNextToken������Ҫ��
    std::string TokenBackup;
    char TheNextChar;
    bool NextCharEOF;
    bool NextCharEOL;
    bool NextCharValid;
    bool TokenIsString;
    // �ļ�����ʹ������ָ������
    std::unique_ptr<FILE, decltype(&fclose)> FileStream;

    // ���ÿ���
    Reader(const Reader&) = delete;
    Reader& operator=(const Reader&) = delete;
};