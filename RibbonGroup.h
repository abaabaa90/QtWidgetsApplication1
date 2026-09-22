#pragma once
#include <QWidget>

#include <QHash>

class QLabel;
class QToolButton;
class QHBoxLayout;

/**
 * @brief 功能区工具组：一行工具按钮 + 底部组标题（Word 风格）
 *
 * 每个工具按钮用 unicode 字形生成图标（无图片资源也可有图标效果），
 * 主题切换时自动按当前文字色重新生成图标。
 */
class RibbonGroup : public QWidget
{
    Q_OBJECT

public:
    explicit RibbonGroup(const QString& title, QWidget* parent = nullptr);

    /// 添加一个工具按钮（glyph 为图标字符，如 ▣ ● ▲ ◎），返回按钮供 connect
    QToolButton* addTool(const QString& text, const QString& glyph = QString());

private:
    void refreshIcons();

    QLabel*      m_titleLabel    = nullptr;
    QHBoxLayout* m_buttonsLayout = nullptr;
    QHash<QToolButton*, QString> m_glyphs;   // 按钮 -> 图标字形（供主题切换重绘）
};
