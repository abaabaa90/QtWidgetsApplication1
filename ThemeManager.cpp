#include "ThemeManager.h"
#include <QApplication>
#include <QFile>
#include <QSet>
#include <algorithm>

namespace {

// 颜色角色 -> QSS 模板里的 token 名
QString tokenFor(ColorRole role)
{
    switch (role) {
    case ColorRole::Window:        return QStringLiteral("$windowBg");
    case ColorRole::Panel:         return QStringLiteral("$panelBg");
    case ColorRole::PanelAlt:      return QStringLiteral("$panelAltBg");
    case ColorRole::TitleBar:      return QStringLiteral("$titleBarBg");
    case ColorRole::TitleText:     return QStringLiteral("$titleBarText");
    case ColorRole::TextPrimary:   return QStringLiteral("$textPrimary");
    case ColorRole::TextSecondary: return QStringLiteral("$textSecondary");
    case ColorRole::TextDisabled:  return QStringLiteral("$textDisabled");
    case ColorRole::Border:        return QStringLiteral("$border");
    case ColorRole::BorderStrong:  return QStringLiteral("$borderStrong");
    case ColorRole::Accent:        return QStringLiteral("$accent");
    case ColorRole::AccentText:    return QStringLiteral("$accentText");
    case ColorRole::InputBg:       return QStringLiteral("$inputBg");
    case ColorRole::InputText:     return QStringLiteral("$inputText");
    case ColorRole::SelectionBg:   return QStringLiteral("$selectionBg");
    case ColorRole::MenuBarBg:     return QStringLiteral("$menuBarBg");
    case ColorRole::ToolBarBg:     return QStringLiteral("$toolBarBg");
    case ColorRole::StatusBarBg:   return QStringLiteral("$statusBarBg");
    case ColorRole::HoverOverlay:  return QStringLiteral("$hoverOverlay");
    }
    return QStringLiteral("$windowBg");
}

// 某一主题的完整颜色表
QMap<ColorRole, QColor> buildColorTable(Theme theme)
{
    QMap<ColorRole, QColor> t;
    if (theme == Theme::Light) {
        t[ColorRole::Window]        = QColor(0xEC, 0xEC, 0xEC);
        t[ColorRole::Panel]         = QColor(0xF5, 0xF5, 0xF5);
        t[ColorRole::PanelAlt]      = QColor(0xE2, 0xE2, 0xE2);
        t[ColorRole::TitleBar]      = QColor(0xE4, 0xE4, 0xE4);
        t[ColorRole::TitleText]     = QColor(0x2A, 0x2A, 0x2A);
        t[ColorRole::TextPrimary]   = QColor(0x32, 0x32, 0x32);
        t[ColorRole::TextSecondary] = QColor(0x50, 0x50, 0x50);
        t[ColorRole::TextDisabled]  = QColor(0x8A, 0x8A, 0x8A);
        t[ColorRole::Border]        = QColor(0xA0, 0xA0, 0xA0);
        t[ColorRole::BorderStrong]  = QColor(0x78, 0x78, 0x78);
        t[ColorRole::Accent]        = QColor(0x46, 0x82, 0xC8);
        t[ColorRole::AccentText]    = QColor(0xFF, 0xFF, 0xFF);
        t[ColorRole::InputBg]       = QColor(0xFF, 0xFF, 0xFF);
        t[ColorRole::InputText]     = QColor(0x2A, 0x2A, 0x2A);
        t[ColorRole::SelectionBg]   = QColor(0x46, 0x82, 0xC8);
        t[ColorRole::MenuBarBg]     = QColor(0xF5, 0xF5, 0xF5);
        t[ColorRole::ToolBarBg]     = QColor(0xF5, 0xF5, 0xF5);
        t[ColorRole::StatusBarBg]   = QColor(0xE8, 0xE8, 0xE8);
        t[ColorRole::HoverOverlay]  = QColor(0, 0, 0, 25);
    } else { // Dark
        t[ColorRole::Window]        = QColor(0x26, 0x26, 0x2B);
        t[ColorRole::Panel]         = QColor(0x2E, 0x2E, 0x34);
        t[ColorRole::PanelAlt]      = QColor(0x3B, 0x3B, 0x42);
        t[ColorRole::TitleBar]      = QColor(0x1B, 0x1B, 0x1F);
        t[ColorRole::TitleText]     = QColor(0xE6, 0xE6, 0xE6);
        t[ColorRole::TextPrimary]   = QColor(0xD8, 0xD8, 0xDC);
        t[ColorRole::TextSecondary] = QColor(0x9E, 0x9E, 0xA6);
        t[ColorRole::TextDisabled]  = QColor(0x5A, 0x5A, 0x62);
        t[ColorRole::Border]        = QColor(0x46, 0x46, 0x4E);
        t[ColorRole::BorderStrong]  = QColor(0x70, 0x70, 0x7A);
        t[ColorRole::Accent]        = QColor(0x4F, 0xA3, 0xE0);
        t[ColorRole::AccentText]    = QColor(0x0B, 0x0B, 0x0E);
        t[ColorRole::InputBg]       = QColor(0x1E, 0x1E, 0x24);
        t[ColorRole::InputText]     = QColor(0xE0, 0xE0, 0xE4);
        t[ColorRole::SelectionBg]   = QColor(0x2C, 0x5F, 0x8A);
        t[ColorRole::MenuBarBg]     = QColor(0x2E, 0x2E, 0x34);
        t[ColorRole::ToolBarBg]     = QColor(0x2E, 0x2E, 0x34);
        t[ColorRole::StatusBarBg]   = QColor(0x20, 0x20, 0x26);
        t[ColorRole::HoverOverlay]  = QColor(255, 255, 255, 22);
    }
    return t;
}

} // namespace

ThemeManager& ThemeManager::instance()
{
    static ThemeManager inst;
    return inst;
}

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
{
    m_colorTable[Theme::Light] = buildColorTable(Theme::Light);
    m_colorTable[Theme::Dark]  = buildColorTable(Theme::Dark);
    loadTemplate();

    // 恢复上次保存的主题
    const QString saved = appSettings()
        .value(QStringLiteral("appearance/theme"), QStringLiteral("light")).toString();
    m_current = (saved.compare(QStringLiteral("dark"), Qt::CaseInsensitive) == 0)
                ? Theme::Dark : Theme::Light;
}

void ThemeManager::loadTemplate()
{
    QFile file(QStringLiteral(":/QtWidgetsApplication1/theme.qss"));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_template = QString::fromUtf8(file.readAll());
        file.close();
    }
}

void ThemeManager::setTheme(Theme theme)
{
    m_current = theme;

    // 持久化当前主题，下次启动恢复
    appSettings().setValue(QStringLiteral("appearance/theme"),
                           theme == Theme::Dark ? QStringLiteral("dark") : QStringLiteral("light"));

    qApp->setPalette(paletteFor(theme));
    qApp->setStyleSheet(renderQss(theme));

    emit themeChanged(theme);
}

QSettings ThemeManager::appSettings()
{
    return QSettings(QStringLiteral("MyVTK"), QStringLiteral("3D Visualization Platform"));
}

void ThemeManager::toggleTheme()
{
    setTheme(m_current == Theme::Light ? Theme::Dark : Theme::Light);
}

QString ThemeManager::themeName(Theme t)
{
    return t == Theme::Light ? QStringLiteral("亮") : QStringLiteral("暗");
}

QColor ThemeManager::color(ColorRole role)
{
    return color(instance().currentTheme(), role);
}

QColor ThemeManager::color(Theme t, ColorRole role)
{
    return instance().m_colorTable.value(t).value(role);
}

QString ThemeManager::qssColor(const QColor& c)
{
    if (c.alpha() < 255) {
        return QStringLiteral("rgba(%1,%2,%3,%4)")
            .arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
    }
    return c.name();
}

QPalette ThemeManager::paletteFor(Theme t) const
{
    QPalette p;
    auto c = [&](ColorRole r) { return color(t, r); };

    p.setColor(QPalette::Window,         c(ColorRole::Window));
    p.setColor(QPalette::WindowText,     c(ColorRole::TextPrimary));
    p.setColor(QPalette::Base,           c(ColorRole::InputBg));
    p.setColor(QPalette::AlternateBase,  c(ColorRole::PanelAlt));
    p.setColor(QPalette::Text,           c(ColorRole::InputText));
    p.setColor(QPalette::Button,         c(ColorRole::Panel));
    p.setColor(QPalette::ButtonText,     c(ColorRole::TextPrimary));
    p.setColor(QPalette::Highlight,      c(ColorRole::SelectionBg));
    p.setColor(QPalette::HighlightedText,c(ColorRole::AccentText));
    p.setColor(QPalette::PlaceholderText,c(ColorRole::TextSecondary));
    p.setColor(QPalette::Link,           c(ColorRole::Accent));
    p.setColor(QPalette::ToolTipBase,    c(ColorRole::InputBg));
    p.setColor(QPalette::ToolTipText,    c(ColorRole::TextPrimary));

    // Disabled 组
    const QColor disabled = c(ColorRole::TextDisabled);
    p.setColor(QPalette::Disabled, QPalette::WindowText,  disabled);
    p.setColor(QPalette::Disabled, QPalette::Text,        disabled);
    p.setColor(QPalette::Disabled, QPalette::ButtonText,  disabled);
    p.setColor(QPalette::Disabled, QPalette::Base,        c(ColorRole::Window));
    return p;
}

QString ThemeManager::renderQss(Theme t) const
{
    QString qss = m_template;
    const QMap<ColorRole, QColor>& table = m_colorTable.value(t);

    // token 存在前缀冲突：$accent 是 $accentText 的前缀、$border 是 $borderStrong 的前缀。
    // 若先替换短 token，$accentText 会变成 "#accentText" 导致 QSS 解析警告。
    // 因此按 token 长度从长到短依次替换。
    QList<ColorRole> roles = table.keys();
    std::sort(roles.begin(), roles.end(),
              [](ColorRole a, ColorRole b) { return tokenFor(a).size() > tokenFor(b).size(); });
    for (ColorRole role : roles) {
        qss.replace(tokenFor(role), qssColor(table.value(role)));
    }
    return qss;
}

QString ThemeManager::qssFor(Theme t) const
{
    return renderQss(t);
}
