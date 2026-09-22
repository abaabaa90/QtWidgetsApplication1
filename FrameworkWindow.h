#pragma once
#include "BaseWindow.h"
#include "ThemeManager.h"
#include <QDockWidget>
#include <QList>

class QMainWindow;
class QMenuBar;
class QToolBar;
class QStatusBar;

/**
 * @brief ParaView 风格框架窗口基类
 *
 * 继承 BaseWindow（无边框外壳：标题栏/拖动/阴影/圆角），
 * 在内容区里内嵌一个 QMainWindow 作为"宿主 shell"，
 * 由它承载菜单栏、工具栏、可拖拽停靠面板、中央内容和状态栏。
 *
 * 业务窗口继承此类，用 addDockPanel / setCentralWidget / addToolBar
 * 把功能模块嵌入到"大窗口"里。
 */
class FrameworkWindow : public BaseWindow
{
    Q_OBJECT

public:
    explicit FrameworkWindow(QWidget* parent = nullptr);

    /// 宿主 shell（QMainWindow），一般无需直接使用
    QMainWindow* shell() const { return m_shell; }

    /// 菜单栏（首次调用自动创建）
    QMenuBar* menuBar() const;
    /// 添加一个工具栏
    QToolBar* addToolBar(const QString& title);
    /// 注册一个可拖拽停靠的面板
    QDockWidget* addDockPanel(const QString& title, QWidget* widget,
                              Qt::DockWidgetArea area = Qt::LeftDockWidgetArea);
    /// 注册一个已创建的可停靠面板（DockPanelBase 等）
    void addDockPanel(QDockWidget* dock, Qt::DockWidgetArea area);
    /// 设置中央内容（渲染视图等）
    void setCentralWidget(QWidget* widget);
    /// 状态栏（首次调用自动创建）
    QStatusBar* statusBar() const;
    /// 在状态栏显示消息
    void showStatusMessage(const QString& message, int timeout = 0);

    void setDockOptions(QMainWindow::DockOptions options);
    /// 把两个 dock 合并为标签页
    void tabifyDockWidget(QDockWidget* first, QDockWidget* second);
    void resizeDocks(const QList<QDockWidget*>& docks, const QList<int>& sizes,
                     Qt::Orientation orientation);
    QByteArray saveDockState() const;
    void restoreDockState(const QByteArray& state);

    /// 从 QSettings 恢复上次保存的停靠布局（应在子类构建完所有面板后调用）
    void restorePersistentState();

protected:
    /// 创建宿主 shell 并放入内容区（子类可重写扩展）
    virtual void buildShell();

    /// 关闭时保存停靠布局到 QSettings
    void closeEvent(QCloseEvent* event) override;

    QMainWindow* m_shell = nullptr;
    QPushButton* m_themeButton = nullptr;   // 标题栏上的主题切换按钮
    QMenuBar*    m_menuBar = nullptr;       // 并入顶部栏的菜单栏
    QToolBar*    m_toolBar = nullptr;       // 并入顶部栏的工具栏

private slots:
    void onThemeChanged(Theme theme);

private:
    void initThemeButton();
};
