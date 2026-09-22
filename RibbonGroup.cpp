#include "RibbonGroup.h"
#include "ThemeManager.h"

#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>
#include <QPainter>
#include <QIcon>
#include <QFont>

namespace {
// 用 unicode 字形绘制图标（无图片资源时也能有图标效果）
QIcon makeGlyphIcon(const QString& glyph, const QColor& color)
{
    QPixmap pm(36, 36);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(color);
    QFont f;
    f.setPointSize(18);
    f.setBold(true);
    p.setFont(f);
    p.drawText(pm.rect(), Qt::AlignCenter, glyph);
    return QIcon(pm);
}
} // namespace

//-----------------------------------------------------------------------------
RibbonGroup::RibbonGroup(const QString& title, QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("ribbonGroup"));
    setAttribute(Qt::WA_StyledBackground, true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 4, 6, 2);
    mainLayout->setSpacing(2);

    // 工具按钮行
    m_buttonsLayout = new QHBoxLayout;
    m_buttonsLayout->setSpacing(3);
    mainLayout->addLayout(m_buttonsLayout);

    // 组标题（底部）
    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName(QStringLiteral("ribbonGroupTitle"));
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mainLayout->addWidget(m_titleLabel);

    // 主题切换时重新生成图标颜色
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this](Theme) { refreshIcons(); });
}

//-----------------------------------------------------------------------------
QToolButton* RibbonGroup::addTool(const QString& text, const QString& glyph)
{
    auto* btn = new QToolButton(this);
    btn->setObjectName(QStringLiteral("ribbonToolButton"));
    btn->setText(text);
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setIconSize(QSize(36, 36));
    btn->setFixedSize(64, 58);
    btn->setToolTip(text);

    if (!glyph.isEmpty()) {
        m_glyphs.insert(btn, glyph);
        btn->setIcon(makeGlyphIcon(glyph, ThemeManager::color(ColorRole::TitleText)));
    }

    m_buttonsLayout->addWidget(btn);
    return btn;
}

//-----------------------------------------------------------------------------
void RibbonGroup::refreshIcons()
{
    const QColor color = ThemeManager::color(ColorRole::TitleText);
    for (auto it = m_glyphs.constBegin(); it != m_glyphs.constEnd(); ++it) {
        if (it.key()) it.key()->setIcon(makeGlyphIcon(it.value(), color));
    }
}
