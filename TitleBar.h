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
    QVector<QPushButton*> btnList;  // 保存所有按钮的指针
    QStringList TitleBarContent = { "File", "Edit", "View", "Help" };
    int currentCheckedBtnIdx = -1;  // 当前选中按钮的索引

    // 修正后的样式表
    QString buttonStyle = R"(
        QPushButton {
            background-color: #000000;  /* 黑色背景 */
            color: #FFFFFF;             /* 白色文字 */
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
            background-color: #FFFFFF;  /* 选中时白色背景 */
            color: #000000;             /* 选中时黑色文字 */
            font-weight: bold;
            border-bottom: 2px solid #FF0000; /* 红色底部边框作为选中标识 */
        }
        
        QPushButton:checked:hover {
            background-color: #F0F0F0;  /* 选中悬停时浅灰色 */
        }
    )";
};