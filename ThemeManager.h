#pragma once
#include <QObject>
#include <QColor>
#include <QMap>
#include <QPalette>
#include <QSettings>
#include <QString>

// 主题枚举：当前只有白 / 黑两套
enum class Theme {
    Light,
    Dark
};

// 颜色角色：与 theme.qss 模板里的 $token 一一对应，
// C++ 里需要按主题取色的控件（如 RenderViewFrame）也用这套角色。
enum class ColorRole {
    Window,          // 窗口/内容大背景
    Panel,           // 面板/按钮底色
    PanelAlt,        // 悬停/凸起
    TitleBar,        // 主标题栏
    TitleText,       // 标题栏文字
    TextPrimary,     // 正文
    TextSecondary,   // 次要文字
    TextDisabled,    // 禁用文字
    Border,          // 常规边框/分隔线
    BorderStrong,    // 强调边框/handle hover
    Accent,          // 强调色（选中/激活帧边框）
    AccentText,      // 强调底上的文字
    InputBg,         // 输入框/表格底
    InputText,       // 输入文字
    SelectionBg,     // 高亮选中
    MenuBarBg,       // 菜单栏
    ToolBarBg,       // 工具栏
    StatusBarBg,     // 状态栏
    HoverOverlay     // 透明悬停覆盖（按钮 hover）
};

/**
 * @brief 主题管理器（单例）
 *
 * 维护 Light/Dark 两套主题。切换流程：
 *   1. qApp->setPalette(paletteFor(theme)) 设置全局调色板；
 *   2. 读 theme.qss token 模板，替换 $token 占位符生成完整 QSS，
 *      用 qApp->setStyleSheet() 整体替换（不是追加）；
 *   3. 发出 themeChanged 信号，各控件自行重刷硬编码取色。
 */
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager& instance();

    Theme currentTheme() const { return m_current; }

    /// 应用指定主题（palette + QSS + 发信号）
    void setTheme(Theme theme);
    /// 在 Light/Dark 之间切换
    void toggleTheme();

    /// 主题显示名："亮" / "暗"
    static QString themeName(Theme t);

    /// 取当前主题下某角色的颜色
    static QColor color(ColorRole role);
    /// 取指定主题下某角色的颜色
    static QColor color(Theme t, ColorRole role);

    /// QColor -> QSS 颜色字符串（带 alpha 时输出 rgba()）
    static QString qssColor(const QColor& c);

    /// 生成某主题的 QPalette
    QPalette paletteFor(Theme t) const;
    /// 渲染某主题的完整 QSS（便于调试）
    QString qssFor(Theme t) const;

    /// 统一的 QSettings 入口（保存主题 / 停靠布局等）
    static QSettings appSettings();

signals:
    /// 主题切换完成时发出
    void themeChanged(Theme theme);

private:
    explicit ThemeManager(QObject* parent = nullptr);
    void loadTemplate();
    QString renderQss(Theme t) const;

    QString m_template;                                        // theme.qss 模板
    QMap<Theme, QMap<ColorRole, QColor>> m_colorTable;         // 两套主题的颜色表
    Theme m_current = Theme::Light;
};
