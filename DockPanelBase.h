#pragma once
#include <QDockWidget>

class QWidget;
class QPushButton;
class DockTitleBar;

/**
 * @brief 可复用停靠面板基类
 *
 * 统一封装（供未来的日志/属性/渲染等窗口继承）：
 *  - 唯一 objectName "dock_<title>"，供 QMainWindow::saveState/restoreState 匹配；
 *  - 允许停靠到上/下/左/右四条边，可浮动、可关闭、可拖动（像 VS 的属性管理器）；
 *  - setTitleBarWidget(DockTitleBar) 替换原生标题栏，拖拽/停靠手势由 QDockWidget
 *    内部机制保留；
 *  - 子类只需创建内容 widget 并调用 setContentWidget()。
 */
class DockPanelBase : public QDockWidget
{
    Q_OBJECT

public:
    explicit DockPanelBase(const QString& title, QWidget* parent = nullptr);

    void setContentWidget(QWidget* widget);
    QWidget* contentWidget() const;

    /// 在标题栏插入一个动作按钮，返回按钮以便 connect
    QPushButton* addTitleAction(const QString& text, const QString& objectName);
    /// 更新标题（标题栏文字 + QDockWidget 窗口标题）
    void setPanelTitle(const QString& title);

protected:
    /// 子类重写返回内容区 widget。基类提供默认空实现；基类构造不会调用虚函数，
    /// 请在子类构造体内调用。
    virtual QWidget* createContent();

private slots:
    void onFloatToggled();
    void onCloseClicked();
    void onTopLevelChanged(bool floating);

private:
    DockTitleBar* m_titleBar = nullptr;
    QWidget*      m_content  = nullptr;
};
