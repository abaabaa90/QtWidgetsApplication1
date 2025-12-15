// TitleBar.h
#pragma once
#include "pch.h"

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    TitleBar(QWidget* parent = nullptr);
    ~TitleBar();
    void SetStatus(int idx);

private:
    QHBoxLayout* HMainLayout = nullptr;
    QVector<QPushButton*> btnList;  // �������а�ť��ָ��
    QStringList TitleBarContent = { "File", "Edit", "View", "Help" };
    int currentCheckedBtnIdx = -1;  // ��ǰѡ�а�ť������

    // ���������ʽ��
    QString buttonStyle = R"(
        QPushButton {
            background-color: #000000;  /* ��ɫ���� */
            color: #FFFFFF;             /* ��ɫ���� */
            border: none;
            padding: 8px 16px;
            margin: 0px;
            min-width: 60px;
            font-size: 14px;
        }
        
        QPushButton:hover {
            background-color: #333333;
        }
        
        QPushButton:pressed {
            background-color: #666666;
        }
        
        QPushButton:checked {
            background-color: #FFFFFF;  /* ѡ��ʱ��ɫ���� */
            color: #000000;             /* ѡ��ʱ��ɫ���� */
            font-weight: bold;
            border-bottom: 2px solid #FF0000; /* ��ɫ�ײ��߿���Ϊѡ�б�ʶ */
        }
        
        QPushButton:checked:hover {
            background-color: #F0F0F0;  /* ѡ����ͣʱǳ��ɫ */
        }
    )";
};