//对于窗口基类我需要为他们提供一个统一的基础样式,比如说设置它的一些基本组件，设置窗口的背景色都是统一的
#pragma once
#include "pch.h"
#include <QWidget>

class BaseWindow : public QWidget
{
    Q_OBJECT

public:
    explicit BaseWindow(QWidget* parent = nullptr, bool showCloseButton = true);
    virtual ~BaseWindow();

    // 设置窗口标题
    void setWindowTitle(const QString& title);

    // 设置窗口大小
    void setWindowSize(int width, int height);
    void setWindowSize(const QSize& size);

    // 设置固定大小
    void setFixedWindowSize(int width, int height);
    void setFixedWindowSize(const QSize& size);

    // 设置最小/最大尺寸
    void setMinimumWindowSize(int width, int height);
    void setMaximumWindowSize(int width, int height);

    // 设置背景颜色
    void setBackgroundColor(const QColor& color);

    // 设置窗口圆角
    void setWindowRadius(int radius);

    // 设置窗口阴影
    void setWindowShadow(bool enabled, int blurRadius = 10);

    // 设置标题栏样式
    void setTitleBarVisible(bool visible);
    void setTitleBarHeight(int height);
    void setTitleBarColor(const QColor& color);

    // 设置内容区域边距
    void setContentMargins(int left, int top, int right, int bottom);

    // 窗口显示相关
    void showCentered();  // 居中显示
    void showMaximized(); // 最大化显示
    void showMinimized(); // 最小化显示
    void showNormal();    // 正常显示

    // 获取内容区域widget（子类可以在这里添加内容）
    QWidget* contentWidget() const;

protected:
    // 子类可以重写这些方法来自定义样式
    virtual void initUI();
    virtual void initStyle();
    virtual void initConnections();

    // 事件重写
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    // 子类可以访问的成员
    QWidget* m_titleBar;           // 标题栏
    QWidget* m_contentWidget;      // 内容区域
    QLabel* m_titleLabel;          // 标题标签
    QPushButton* m_closeButton;    // 关闭按钮
    QPushButton* m_minButton;      // 最小化按钮
    QPushButton* m_maxButton;      // 最大化按钮

    QVBoxLayout* m_mainLayout;     // 主布局
    QHBoxLayout* m_titleLayout;    // 标题栏布局
    QVBoxLayout* m_contentLayout;  // 内容布局

private slots:
    void onCloseClicked();
    void onMinClicked();
    void onMaxClicked();

private:
    // 初始化方法
    void initMembers();
    void setupLayout();
    void setupTitleBar();
    void setupContentArea();

    // 窗口拖动相关
    bool m_isDragging;
    QPoint m_dragStartPosition;

    // 样式相关
    QColor m_backgroundColor;
    QColor m_titleBarColor;
    int m_windowRadius;
    bool m_shadowEnabled;
    int m_shadowBlurRadius;

    // 状态
    bool m_isMaximized;
    QRect m_normalGeometry;

    // 私有方法
    void updateWindowStyle();
    void drawWindowShadow(QPainter& painter);
    void drawRoundedRect(QPainter& painter, const QRect& rect, int radius);
};