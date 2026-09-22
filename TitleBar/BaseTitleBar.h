#include "pch.h"

// 无边框窗口实现（支持拖动和边缘缩放）
class BaseTitleBar : public QWidget {
public:
	BaseTitleBar();
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void init();
	void SetupUI();

	// 边缘检测
	enum Edge {
		None = 0,
		Left = 1,
		Top = 2,
		Right = 4,
		Bottom = 8,
		TopLeft = Top | Left,
		TopRight = Top | Right,
		BottomLeft = Bottom | Left,
		BottomRight = Bottom | Right
	};

	Edge detectEdge(const QPoint& pos) const;
	void updateCursorShape(Edge edge);
	void resizeWindow(const QPoint& globalPos);

private:
	// 窗口拖动相关
	QPoint m_dragPosition;
	bool m_isDragging = false;

	// 窗口缩放相关
	int m_borderWidth = 5;
	bool m_isResizing = false;
	Edge m_resizeEdge = None;
	QPoint m_resizeStartPos;
	QRect m_resizeStartGeometry;
};