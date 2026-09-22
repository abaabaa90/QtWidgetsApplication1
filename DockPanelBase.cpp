#include "DockPanelBase.h"
#include "DockTitleBar.h"

#include <QPushButton>

//-----------------------------------------------------------------------------
DockPanelBase::DockPanelBase(const QString& title, QWidget* parent)
    : QDockWidget(title, parent)
{
    // 唯一 objectName：供 QMainWindow::saveState/restoreState 匹配布局
    setObjectName(QStringLiteral("dock_%1").arg(title));

    // Movable|Floatable|Closable：可拖动、可浮动、可关闭；不加 VerticalTitleBar
    setFeatures(QDockWidget::DockWidgetMovable |
                QDockWidget::DockWidgetFloatable |
                QDockWidget::DockWidgetClosable);
    setAllowedAreas(Qt::AllDockWidgetAreas);

    // 替换原生标题栏为自定义标题栏（拖拽/停靠手势由 QDockWidget 内部机制保留）
    m_titleBar = new DockTitleBar(this);
    m_titleBar->setTitle(title);
    m_titleBar->setDockWidget(this);   // 供标题栏执行 VS 风格拖拽停靠
    setTitleBarWidget(m_titleBar);

    connect(m_titleBar, &DockTitleBar::floatRequested,
            this, &DockPanelBase::onFloatToggled);
    connect(m_titleBar, &DockTitleBar::closeRequested,
            this, &DockPanelBase::onCloseClicked);
    connect(this, &QDockWidget::topLevelChanged,
            this, &DockPanelBase::onTopLevelChanged);
}

//-----------------------------------------------------------------------------
QWidget* DockPanelBase::createContent()
{
    // 默认：空内容区，供直接实例化 DockPanelBase 的面板使用
    return new QWidget(this);
}

//-----------------------------------------------------------------------------
void DockPanelBase::setContentWidget(QWidget* widget)
{
    m_content = widget;
    setWidget(widget);
}

//-----------------------------------------------------------------------------
QWidget* DockPanelBase::contentWidget() const
{
    return m_content;
}

//-----------------------------------------------------------------------------
QPushButton* DockPanelBase::addTitleAction(const QString& text, const QString& objectName)
{
    return m_titleBar ? m_titleBar->addActionButton(text, objectName) : nullptr;
}

//-----------------------------------------------------------------------------
void DockPanelBase::setPanelTitle(const QString& title)
{
    setWindowTitle(title);
    if (m_titleBar) {
        m_titleBar->setTitle(title);
    }
}

//-----------------------------------------------------------------------------
void DockPanelBase::onFloatToggled()
{
    setFloating(!isFloating());
}

//-----------------------------------------------------------------------------
void DockPanelBase::onCloseClicked()
{
    close();
}

//-----------------------------------------------------------------------------
void DockPanelBase::onTopLevelChanged(bool floating)
{
    // 无论按钮还是直接拖拽浮动，都同步按钮字形
    if (m_titleBar) {
        m_titleBar->setFloatState(floating);
    }
}
