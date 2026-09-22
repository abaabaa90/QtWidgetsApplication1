#include "DockDragOverlay.h"
#include "ThemeManager.h"

#include <QPainter>

//-----------------------------------------------------------------------------
DockDragOverlay::DockDragOverlay(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("dockDragOverlay"));
    // 只做视觉指示，不拦截鼠标事件（拖拽跟踪由标题栏完成）
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
}

//-----------------------------------------------------------------------------
void DockDragOverlay::setHoverZone(DockDropZone zone)
{
    if (m_hover != zone) {
        m_hover = zone;
        update();
    }
}

//-----------------------------------------------------------------------------
DockDropZone DockDragOverlay::hoverZone() const
{
    return m_hover;
}

//-----------------------------------------------------------------------------
DockDropZone DockDragOverlay::zoneAt(const QPoint& p) const
{
    const int cx = width() / 2;
    const int cy = height() / 2;
    const int R = qMin(width(), height()) * 22 / 100;   // 十字外半径
    const int t = R / 2;                                // 十字臂厚度的半宽

    // 四方向臂 + 中央格，按矩形命中
    if (p.x() >= cx - R && p.x() <= cx - t && p.y() >= cy - t && p.y() <= cy + t)
        return DockDropZone::Left;
    if (p.x() >= cx + t && p.x() <= cx + R && p.y() >= cy - t && p.y() <= cy + t)
        return DockDropZone::Right;
    if (p.y() >= cy - R && p.y() <= cy - t && p.x() >= cx - t && p.x() <= cx + t)
        return DockDropZone::Top;
    if (p.y() >= cy + t && p.y() <= cy + R && p.x() >= cx - t && p.x() <= cx + t)
        return DockDropZone::Bottom;
    if (p.x() >= cx - t && p.x() <= cx + t && p.y() >= cy - t && p.y() <= cy + t)
        return DockDropZone::Center;
    return DockDropZone::None;
}

//-----------------------------------------------------------------------------
void DockDragOverlay::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 半透明遮罩，压暗底层内容
    p.fillRect(rect(), QColor(0, 0, 0, 110));

    const int cx = width() / 2;
    const int cy = height() / 2;
    // 十字外半径：占短边 13%，并限制最大 150px，避免大窗口下过大
    int R = qMin(width(), height()) * 13 / 100;
    R = qMin(R, 150);
    const int t = R / 2;   // 臂根部半宽

    const QColor accent = ThemeManager::color(ColorRole::Accent);

    auto fillZone = [&](const QPainterPath& path, DockDropZone z) {
        const bool hot = (m_hover == z);
        p.setBrush(hot ? QColor(accent.red(), accent.green(), accent.blue(), 210)
                       : QColor(255, 255, 255, 55));
        p.setPen(QPen(hot ? QColor(255, 255, 255, 235) : QColor(255, 255, 255, 120), 2));
        p.drawPath(path);
    };

    // 四方向三角箭头（紧凑、不超出外半径）
    auto triangle = [&](const QPoint& a, const QPoint& b, const QPoint& c, DockDropZone z) {
        QPainterPath path;
        path.moveTo(a);
        path.lineTo(b);
        path.lineTo(c);
        path.closeSubpath();
        fillZone(path, z);
    };

    triangle(QPoint(cx - t, cy - t), QPoint(cx - t, cy + t), QPoint(cx - R, cy), DockDropZone::Left);
    triangle(QPoint(cx + t, cy - t), QPoint(cx + t, cy + t), QPoint(cx + R, cy), DockDropZone::Right);
    triangle(QPoint(cx - t, cy - t), QPoint(cx + t, cy - t), QPoint(cx, cy - R), DockDropZone::Top);
    triangle(QPoint(cx - t, cy + t), QPoint(cx + t, cy + t), QPoint(cx, cy + R), DockDropZone::Bottom);

    // 中央标签区
    QPainterPath centerPath;
    centerPath.addRoundedRect(QRect(cx - t, cy - t, t * 2, t * 2), 4, 4);
    fillZone(centerPath, DockDropZone::Center);
}
