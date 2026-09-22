#pragma once
#include <QWidget>

class QTabBar;
class QStackedWidget;
class RibbonPage;

/**
 * @brief 类 Office 的功能区工具栏（Ribbon）
 *
 * 顶部为多个页签（类似 Word 的 文件/开始/插入/视图...），点击页签切换下方对应的工具区。
 * 高复用、可扩展：addPage() 新增页签，页面内用 RibbonPage::addGroup() 新增工具组。
 *
 * 布局：QVBoxLayout [页签栏] [内容栈]
 */
class RibbonBar : public QWidget
{
    Q_OBJECT

public:
    explicit RibbonBar(QWidget* parent = nullptr);

    /// 添加一个页签，返回其内容页（之后往页面里加工具组）
    RibbonPage* addPage(const QString& title);
    /// 设置当前页签
    void setCurrentPage(int index);
    /// 页签数量
    int pageCount() const;
    /// 获取指定页签
    RibbonPage* page(int index) const;

private:
    QTabBar*        m_tabBar = nullptr;
    QStackedWidget* m_stack  = nullptr;
};
