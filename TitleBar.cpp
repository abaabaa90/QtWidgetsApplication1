// TitleBar.cpp
#include "TitleBar.h"

TitleBar::TitleBar(QWidget* parent) : QWidget(parent)
{
    HMainLayout = new QHBoxLayout(this);
    HMainLayout->setSpacing(0);
    HMainLayout->setContentsMargins(0, 0, 0, 0);

    // ������ť�����浽������
    for (int i = 0; i < TitleBarContent.size(); i++) {
        QPushButton* btn = new QPushButton(TitleBarContent[i]);
        btn->setCheckable(true);  // ����Ϊ��ѡ��
        btn->setStyleSheet(buttonStyle);

        // ����ǵ�һ����ť��Ĭ��ѡ��
        if (i == 0) {btn->setChecked(true);currentCheckedBtnIdx = 0;}

        HMainLayout->addWidget(btn);
        btnList.append(btn);  // ���水ťָ��

        // ���ӵ���źţ�ʹ��lambda����ť����
        connect(btn, &QPushButton::clicked, [this, i]() {
            this->SetStatus(i);
            });
    }

    HMainLayout->addStretch();
    setFixedHeight(40);
}

void TitleBar::SetStatus(int idx)
{
    // �����������Ѿ�ѡ�еİ�ť��ֱ�ӷ���
    if (idx == currentCheckedBtnIdx) {
        return;
    }

    // ȡ��֮ǰѡ�а�ť��״̬
    if (currentCheckedBtnIdx >= 0 && currentCheckedBtnIdx < btnList.size()) {
        btnList[currentCheckedBtnIdx]->setChecked(false);
    }

    // ������ѡ�а�ť��״̬
    if (idx >= 0 && idx < btnList.size()) {
        btnList[idx]->setChecked(true);
        currentCheckedBtnIdx = idx;

        // ���������﷢���źţ�֪ͨ��������ĸ���ť��ѡ����
        // emit buttonSelected(idx);
    }
}

TitleBar::~TitleBar()
{
    // ����Ҫ�ֶ�ɾ���Ӳ�����Qt�ĸ��Ӷ�����ƻ��Զ�����
}