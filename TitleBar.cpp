// TitleBar.cpp
#include "TitleBar.h"

TitleBar::TitleBar(QWidget* parent) : QWidget(parent)
{
    HMainLayout = new QHBoxLayout(this);
    HMainLayout->setSpacing(0);
    HMainLayout->setContentsMargins(0, 0, 0, 0);

    // 创建按钮并保存到容器中
    for (int i = 0; i < TitleBarContent.size(); i++) {
        QPushButton* btn = new QPushButton(TitleBarContent[i]);
        btn->setCheckable(true);  // 设置为可选中
        btn->setStyleSheet(buttonStyle);

        // 如果是第一个按钮，默认选中
        if (i == 0) {btn->setChecked(true);currentCheckedBtnIdx = 0;}

        HMainLayout->addWidget(btn);
        btnList.append(btn);  // 保存按钮指针

        // 连接点击信号，使用lambda捕获按钮索引
        connect(btn, &QPushButton::clicked, [this, i]() {
            this->SetStatus(i);
            });
    }

    HMainLayout->addStretch();
    setFixedHeight(40);
}

void TitleBar::SetStatus(int idx)
{
    // 如果点击的是已经选中的按钮，直接返回
    if (idx == currentCheckedBtnIdx) {
        return;
    }

    // 取消之前选中按钮的状态
    if (currentCheckedBtnIdx >= 0 && currentCheckedBtnIdx < btnList.size()) {
        btnList[currentCheckedBtnIdx]->setChecked(false);
    }

    // 设置新选中按钮的状态
    if (idx >= 0 && idx < btnList.size()) {
        btnList[idx]->setChecked(true);
        currentCheckedBtnIdx = idx;

        // 可以在这里发射信号，通知其他组件哪个按钮被选中了
        // emit buttonSelected(idx);
    }
}

TitleBar::~TitleBar()
{
    // 不需要手动删除子部件，Qt的父子对象机制会自动处理
}