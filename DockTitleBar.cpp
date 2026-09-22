#include "DockTitleBar.h"
#include "StyleManager.h"

#include <QDockWidget>
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>

namespace {
const int kDragThreshold = 5;   // 进入拖拽的最小移动像素
const qreal kDragOpacity = 0.35;  // 浮动窗口进入主窗口时半透明
}

//-----------------------------------------------------------------------------
DockTitleBar::DockTitleBar(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

//-----------------------------------------------------------------------------
void DockTitleBar::setupUI()
{
    // 自定义 QWidget 子类需要 WA_StyledBackground，QSS 的 background-color 才会绘制
    setObjectName(QStringLiteral("dockTitleBar"));
    setAttribute(Qt::WA_StyledBackground, true);

    int unit = StyleManager::baseUnit();
    int btnSize = static_cast<int>(unit * 0.40);
    m_buttonSize = QSize(btnSize, btnSize);

    m_titleLayout = new QHBoxLayout(this);
    m_titleLayout->setContentsMargins(static_cast<int>(unit * 0.12), 0,
                                      static_cast<int>(unit * 0.06), 0);
    m_titleLayout->setSpacing(static_cast<int>(unit * 0.06));

    m_titleLabel = new QLabel(QStringLiteral("Panel"), this);
    m_titleLabel->setObjectName(QStringLiteral("dockTitleLabel"));
    // 不允许选中文本，否则会吞掉鼠标事件、破坏拖拽
    m_titleLabel->setTextInteractionFlags(Qt::NoTextInteraction);

    m_floatBtn = new QPushButton(QStringLiteral("⤢"), this);
    m_closeBtn = new QPushButton(QStringLiteral("×"), this);
    m_floatBtn->setObjectName(QStringLiteral("dockTitleBtn"));
    m_closeBtn->setObjectName(QStringLiteral("dockTitleCloseBtn"));

    m_floatBtn->setFixedSize(m_buttonSize);
    m_closeBtn->setFixedSize(m_buttonSize);
    m_floatBtn->setFocusPolicy(Qt::NoFocus);
    m_closeBtn->setFocusPolicy(Qt::NoFocus);
    m_floatBtn->setToolTip(QStringLiteral("浮动为独立窗口"));
    m_closeBtn->setToolTip(QStringLiteral("关闭面板"));

    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_floatBtn);
    m_titleLayout->addWidget(m_closeBtn);

    connect(m_floatBtn, &QPushButton::clicked, this, &DockTitleBar::floatRequested);
    connect(m_closeBtn, &QPushButton::clicked, this, &DockTitleBar::closeRequested);
}

//-----------------------------------------------------------------------------
void DockTitleBar::setTitle(const QString& title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
}

//-----------------------------------------------------------------------------
QString DockTitleBar::title() const
{
    return m_titleLabel ? m_titleLabel->text() : QString();
}

//-----------------------------------------------------------------------------
QPushButton* DockTitleBar::addActionButton(const QString& text, const QString& objectName)
{
    auto* btn = new QPushButton(text, this);
    btn->setObjectName(objectName);
    btn->setFixedSize(m_buttonSize);
    btn->setFocusPolicy(Qt::NoFocus);

    // 插到浮动按钮左侧
    if (m_titleLayout) {
        m_titleLayout->insertWidget(m_titleLayout->indexOf(m_floatBtn), btn);
    }
    return btn;
}

//-----------------------------------------------------------------------------
void DockTitleBar::setFloatState(bool floating)
{
    if (!m_floatBtn) return;
    m_floatBtn->setText(floating ? QStringLiteral("⤡") : QStringLiteral("⤢"));
    m_floatBtn->setToolTip(floating ? QStringLiteral("停靠回主窗口")
                                    : QStringLiteral("浮动为独立窗口"));
}

//-----------------------------------------------------------------------------
QSize DockTitleBar::sizeHint() const
{
    return QSize(200, m_buttonSize.height() + 8);
}

//-----------------------------------------------------------------------------
void DockTitleBar::setDockWidget(QDockWidget* dock)
{
    m_dock = dock;
}

//-----------------------------------------------------------------------------
void DockTitleBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        m_pressGlobalPos = event->globalPos();
        if (m_dock) {
            m_dockStartPos = m_dock->isFloating() ? m_dock->pos() : QPoint();
        }
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

//-----------------------------------------------------------------------------
void DockTitleBar::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_pressed || !m_dock) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    const QPoint delta = event->globalPos() - m_pressGlobalPos;
    if (!m_dragging && delta.manhattanLength() > kDragThreshold) {
        m_dragging = true;
        grabMouse();
        ensureOverlay();
        if (m_overlay) m_overlay->show();
    }
    if (!m_dragging) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    // 浮动窗口：始终跟随鼠标移动（进入主窗口后半透明，让指示器透出来）
    if (m_dock->isFloating()) {
        m_dock->move(m_dockStartPos + delta);
    }

    auto* shell = qobject_cast<QMainWindow*>(m_dock->parent());
    const QPoint shellPos = shell ? shell->mapFromGlobal(event->globalPos()) : QPoint();
    const bool overShell = shell && shell->rect().contains(shellPos);

    if (overShell && m_overlay) {
        m_overlay->setGeometry(shell->rect());
        if (m_overlay->isHidden()) m_overlay->show();
        m_overlay->raise();
        m_overlay->setHoverZone(m_overlay->zoneAt(shellPos));
    } else {
        if (m_overlay) m_overlay->hide();
    }

    if (m_dock->isFloating()) {
        m_dock->setWindowOpacity(overShell ? kDragOpacity : 1.0);
    }

    event->accept();
}

//-----------------------------------------------------------------------------
void DockTitleBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_dragging) {
        m_dragging = false;
        releaseMouse();
        if (m_dock) m_dock->setWindowOpacity(1.0);

        auto* shell = qobject_cast<QMainWindow*>(m_dock ? m_dock->parent() : nullptr);
        if (shell && m_overlay && m_overlay->isVisible()) {
            const QPoint shellPos = shell->mapFromGlobal(event->globalPos());
            performDrop(shell, shellPos);
        }
        if (m_overlay) m_overlay->hide();
    }
    m_pressed = false;
    QWidget::mouseReleaseEvent(event);
}

//-----------------------------------------------------------------------------
void DockTitleBar::ensureOverlay()
{
    auto* shell = qobject_cast<QMainWindow*>(m_dock ? m_dock->parent() : nullptr);
    if (!shell) return;
    if (!m_overlay || m_overlay->parentWidget() != shell) {
        if (m_overlay) m_overlay->deleteLater();
        m_overlay = new DockDragOverlay(shell);
        m_overlay->setGeometry(shell->rect());
        m_overlay->hide();
    }
}

//-----------------------------------------------------------------------------
void DockTitleBar::performDrop(QMainWindow* shell, const QPoint& shellPos)
{
    if (!m_dock || !shell) return;

    const DockDropZone zone = m_overlay ? m_overlay->zoneAt(shellPos) : DockDropZone::None;
    if (zone == DockDropZone::None) return;

    // 若是浮动窗口，先停靠回主窗口再按区域摆放
    if (m_dock->isFloating()) m_dock->setFloating(false);

    switch (zone) {
    case DockDropZone::Left:
        shell->addDockWidget(Qt::LeftDockWidgetArea, m_dock);
        break;
    case DockDropZone::Right:
        shell->addDockWidget(Qt::RightDockWidgetArea, m_dock);
        break;
    case DockDropZone::Top:
        shell->addDockWidget(Qt::TopDockWidgetArea, m_dock);
        break;
    case DockDropZone::Bottom:
        shell->addDockWidget(Qt::BottomDockWidgetArea, m_dock);
        break;
    case DockDropZone::Center: {
        // 中央：与鼠标下的已有面板标签页合并；无目标则停靠到右侧
        QDockWidget* target = dockAt(shell, shellPos);
        if (target && target != m_dock) {
            shell->tabifyDockWidget(target, m_dock);
            m_dock->raise();
        } else {
            shell->addDockWidget(Qt::RightDockWidgetArea, m_dock);
        }
        break;
    }
    default:
        break;
    }
}

//-----------------------------------------------------------------------------
QDockWidget* DockTitleBar::dockAt(QMainWindow* shell, const QPoint& shellPos) const
{
    if (!shell) return nullptr;
    const auto docks = shell->findChildren<QDockWidget*>();
    for (QDockWidget* d : docks) {
        if (!d || d == m_dock || d->isFloating()) continue;
        if (d->isVisible() && d->geometry().contains(shellPos)) return d;
    }
    return nullptr;
}
