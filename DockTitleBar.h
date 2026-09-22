#pragma once
#include <QWidget>
#include "DockDragOverlay.h"

class QLabel;
class QPushButton;
class QHBoxLayout;
class QDockWidget;
class QMainWindow;
class QMouseEvent;

/**
 * @brief 停靠面板自定义标题栏
 *
 * 外观与 RenderViewFrame 的深色标题栏保持一致（$titleBarBg / $titleBarText token）。
 * 通过 QDockWidget::setTitleBarWidget 挂到停靠面板上。
 *
 * 拖拽停靠（VS 风格）：
 *   - 左键在标题栏按下并拖动超过阈值时进入自定义拖拽；
 *   - 鼠标位于宿主 QMainWindow 内时，显示 DockDragOverlay 中央十字指示器并高亮命中的方向；
 *   - 释放到方向区 -> addDockWidget 停靠到对应边；释放到中央区 -> 与鼠标下面板标签页合并；
 *   - 浮动窗口拖动时跟随鼠标移动（进入主窗口后半透明），无目标释放则保持浮动。
 */
class DockTitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit DockTitleBar(QWidget* parent = nullptr);

    void setTitle(const QString& title);
    QString title() const;

    /// 在浮动按钮左侧插入一个动作按钮（供 DockPanelBase::addTitleAction 使用）
    QPushButton* addActionButton(const QString& text, const QString& objectName);

    /// 同步浮动状态（改按钮字形/提示）
    void setFloatState(bool floating);

    /// 高度供 QDockWidgetLayout::titleHeight() 取 sizeHint 用
    QSize sizeHint() const override;

    /// 关联所属停靠面板（用于拖拽停靠）
    void setDockWidget(QDockWidget* dock);

signals:
    void floatRequested();   // 点击浮动/停靠按钮
    void closeRequested();   // 点击关闭按钮

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void setupUI();
    void ensureOverlay();
    void performDrop(QMainWindow* shell, const QPoint& shellPos);
    QDockWidget* dockAt(QMainWindow* shell, const QPoint& shellPos) const;

    QLabel*      m_titleLabel  = nullptr;
    QPushButton* m_floatBtn    = nullptr;
    QPushButton* m_closeBtn    = nullptr;
    QHBoxLayout* m_titleLayout = nullptr;
    QSize        m_buttonSize  = { 20, 20 };

    QDockWidget*     m_dock           = nullptr;
    DockDragOverlay* m_overlay        = nullptr;
    QPoint           m_pressGlobalPos;
    QPoint           m_dockStartPos;
    bool             m_pressed        = false;
    bool             m_dragging       = false;
};
