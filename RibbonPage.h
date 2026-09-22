#pragma once
#include <QWidget>

class QHBoxLayout;
class QScrollArea;
class RibbonGroup;

/**
 * @brief 功能区的一个页签内容页：横向排列若干工具组
 *
 * 组多时支持横向滚动，保持所有工具可访问（类 Word 的超宽 Ribbon）。
 */
class RibbonPage : public QWidget
{
    Q_OBJECT

public:
    explicit RibbonPage(QWidget* parent = nullptr);

    /// 添加一个工具组，返回组对象（在组里继续 addTool 添加工具按钮）
    RibbonGroup* addGroup(const QString& title);

private:
    QScrollArea*  m_scroll       = nullptr;
    QHBoxLayout*  m_groupsLayout = nullptr;
};
