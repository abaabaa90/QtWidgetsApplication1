#include "FrameworkWindow.h"
#include "ThemeManager.h"

#include <QCloseEvent>
#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QPushButton>
#include <QVBoxLayout>

FrameworkWindow::FrameworkWindow(QWidget* parent)
    : BaseWindow(parent, true)
    , m_shell(nullptr)
    , m_themeButton(nullptr)
{
    buildShell();
    initThemeButton();

    // 应用当前主题（含上次保存的主题），让首帧样式就绪
    ThemeManager::instance().setTheme(ThemeManager::instance().currentTheme());
}

void FrameworkWindow::buildShell()
{
    m_shell = new QMainWindow(m_contentWidget);
    m_shell->setObjectName(QStringLiteral("appShell"));

    // 关键：QMainWindow 构造会把 Qt::Window 强行加入 flags（即使有父窗口），
    // 导致 shell 变成独立的顶层窗口，既不嵌入布局也不显示，面板/菜单/中央
    // 视图全部不可见。这里显式去掉该标志，让 shell 作为子控件撑满内容区。
    m_shell->setWindowFlag(Qt::Window, false);

    m_shell->setDockOptions(QMainWindow::AllowNestedDocks |
                            QMainWindow::AllowTabbedDocks |
                            QMainWindow::AnimatedDocks);
    m_contentLayout->addWidget(m_shell, 1);

    // 菜单栏 / 工具栏并入顶部标题栏，合并为一条聚合顶栏
    m_menuBar = new QMenuBar(m_titleBar);
    m_menuBar->setObjectName(QStringLiteral("titleBarMenuBar"));
    m_titleLayout->insertWidget(m_titleLayout->indexOf(m_titleLabel) + 1, m_menuBar);

    m_toolBar = new QToolBar(QStringLiteral("主工具栏"), m_titleBar);
    m_toolBar->setObjectName(QStringLiteral("titleBarToolBar"));
    m_toolBar->setMovable(false);
    m_titleLayout->insertWidget(m_titleLayout->indexOf(m_menuBar) + 1, m_toolBar);
}

QMenuBar* FrameworkWindow::menuBar() const
{
    return m_menuBar;
}

QToolBar* FrameworkWindow::addToolBar(const QString& title)
{
    Q_UNUSED(title);
    return m_toolBar;
}

QDockWidget* FrameworkWindow::addDockPanel(const QString& title, QWidget* widget,
                                           Qt::DockWidgetArea area)
{
    if (!m_shell) return nullptr;

    auto* dock = new QDockWidget(title, m_shell);
    dock->setObjectName(QStringLiteral("dock_%1").arg(title));
    dock->setWidget(widget);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_shell->addDockWidget(area, dock);
    return dock;
}

void FrameworkWindow::addDockPanel(QDockWidget* dock, Qt::DockWidgetArea area)
{
    if (m_shell && dock) m_shell->addDockWidget(area, dock);
}

void FrameworkWindow::setCentralWidget(QWidget* widget)
{
    if (m_shell) m_shell->setCentralWidget(widget);
}

QStatusBar* FrameworkWindow::statusBar() const
{
    return m_shell ? m_shell->statusBar() : nullptr;
}

void FrameworkWindow::showStatusMessage(const QString& message, int timeout)
{
    if (m_shell && m_shell->statusBar()) {
        m_shell->statusBar()->showMessage(message, timeout);
    }
}

void FrameworkWindow::setDockOptions(QMainWindow::DockOptions options)
{
    if (m_shell) m_shell->setDockOptions(options);
}

void FrameworkWindow::tabifyDockWidget(QDockWidget* first, QDockWidget* second)
{
    if (m_shell) m_shell->tabifyDockWidget(first, second);
}

void FrameworkWindow::resizeDocks(const QList<QDockWidget*>& docks,
                                  const QList<int>& sizes,
                                  Qt::Orientation orientation)
{
    if (m_shell) m_shell->resizeDocks(docks, sizes, orientation);
}

QByteArray FrameworkWindow::saveDockState() const
{
    return m_shell ? m_shell->saveState() : QByteArray();
}

void FrameworkWindow::restoreDockState(const QByteArray& state)
{
    if (m_shell) m_shell->restoreState(state);
}

void FrameworkWindow::restorePersistentState()
{
    if (!m_shell) return;

    const QByteArray dockState =
        ThemeManager::appSettings().value(QStringLiteral("layout/dockState")).toByteArray();
    if (!dockState.isEmpty()) {
        m_shell->restoreState(dockState);
    }
}

void FrameworkWindow::closeEvent(QCloseEvent* event)
{
    if (m_shell) {
        ThemeManager::appSettings().setValue(QStringLiteral("layout/dockState"),
                                             m_shell->saveState());
    }
    QWidget::closeEvent(event);
}

void FrameworkWindow::initThemeButton()
{
    m_themeButton = addTitleBarButton(QStringLiteral("☀"), QStringLiteral("themeButton"));
    if (!m_themeButton) return;

    connect(m_themeButton, &QPushButton::clicked, this, []() {
        ThemeManager::instance().toggleTheme();
    });
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &FrameworkWindow::onThemeChanged);

    onThemeChanged(ThemeManager::instance().currentTheme());
}

void FrameworkWindow::onThemeChanged(Theme theme)
{
    if (!m_themeButton) return;
    m_themeButton->setText(theme == Theme::Light ? QStringLiteral("☀") : QStringLiteral("🌙"));
    m_themeButton->setToolTip(theme == Theme::Light
                              ? QStringLiteral("切换到深色主题")
                              : QStringLiteral("切换到浅色主题"));
}
