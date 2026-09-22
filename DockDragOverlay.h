#pragma once
#include <QWidget>
#include <Qt>

class QPaintEvent;

/// 拖拽停靠的落点区域（四方向 + 中央标签合并区）
enum class DockDropZone {
    None,
    Left,
    Right,
    Top,
    Bottom,
    Center
};

/**
 * @brief VS 风格拖拽停靠指示器（覆盖层）
 *
 * 在宿主 QMainWindow 上覆盖一个半透明遮罩，中央绘制十字形的
 * 四方向停靠指示器 + 中央标签区。DockTitleBar 拖拽时用它显示/高亮，
 * 并在释放时根据 zoneAt() 命中的区域执行 addDockWidget / tabify。
 *
 * 该控件不接收鼠标事件（WA_TransparentForMouseEvents），所有拖拽跟踪
 * 由标题栏的鼠标抓取完成。
 */
class DockDragOverlay : public QWidget
{
public:
    explicit DockDragOverlay(QWidget* parent = nullptr);

    void setHoverZone(DockDropZone zone);
    DockDropZone hoverZone() const;

    /// 根据覆盖层局部坐标计算命中的落点区域
    DockDropZone zoneAt(const QPoint& localPos) const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    DockDropZone m_hover = DockDropZone::None;
};
