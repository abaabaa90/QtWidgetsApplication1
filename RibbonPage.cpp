#include "RibbonPage.h"
#include "RibbonGroup.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QFrame>

//-----------------------------------------------------------------------------
RibbonPage::RibbonPage(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("ribbonPage"));
    setAttribute(Qt::WA_StyledBackground, true);

    auto* pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    // 横向滚动区，容纳多个工具组
    m_scroll = new QScrollArea(this);
    m_scroll->setObjectName(QStringLiteral("ribbonScroll"));
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setFrameShape(QFrame::NoFrame);
    pageLayout->addWidget(m_scroll);

    auto* content = new QWidget;
    content->setObjectName(QStringLiteral("ribbonPageContent"));
    m_groupsLayout = new QHBoxLayout(content);
    m_groupsLayout->setContentsMargins(6, 4, 6, 4);
    m_groupsLayout->setSpacing(6);
    m_groupsLayout->addStretch();   // 末尾弹性，让组靠左排布

    m_scroll->setWidget(content);
}

//-----------------------------------------------------------------------------
RibbonGroup* RibbonPage::addGroup(const QString& title)
{
    auto* group = new RibbonGroup(title, m_scroll->widget());
    // 插到末尾 stretch 之前
    m_groupsLayout->insertWidget(m_groupsLayout->count() - 1, group);
    return group;
}
