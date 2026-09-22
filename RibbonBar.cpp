#include "RibbonBar.h"
#include "RibbonPage.h"

#include <QTabBar>
#include <QStackedWidget>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------
RibbonBar::RibbonBar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("ribbonBar"));
    // 自定义 QWidget 子类需要该属性 QSS 的 background-color 才会绘制
    setAttribute(Qt::WA_StyledBackground, true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_tabBar = new QTabBar(this);
    m_tabBar->setObjectName(QStringLiteral("ribbonTabBar"));
    m_tabBar->setDrawBase(false);   // 去掉页签下的横线，与内容连成一体
    m_tabBar->setExpanding(false);
    layout->addWidget(m_tabBar);

    m_stack = new QStackedWidget(this);
    m_stack->setObjectName(QStringLiteral("ribbonStack"));
    layout->addWidget(m_stack, 1);

    connect(m_tabBar, &QTabBar::currentChanged,
            m_stack, &QStackedWidget::setCurrentIndex);
}

//-----------------------------------------------------------------------------
RibbonPage* RibbonBar::addPage(const QString& title)
{
    auto* page = new RibbonPage(this);
    m_stack->addWidget(page);
    m_tabBar->addTab(title);
    return page;
}

//-----------------------------------------------------------------------------
void RibbonBar::setCurrentPage(int index)
{
    m_tabBar->setCurrentIndex(index);
}

//-----------------------------------------------------------------------------
int RibbonBar::pageCount() const
{
    return m_tabBar->count();
}

//-----------------------------------------------------------------------------
RibbonPage* RibbonBar::page(int index) const
{
    if (!m_stack) return nullptr;
    return static_cast<RibbonPage*>(m_stack->widget(index));
}
