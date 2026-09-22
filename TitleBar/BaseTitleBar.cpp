#include "BaseTitleBar.h"

BaseTitleBar::BaseTitleBar()
{
	init();
	SetupUI();

	this->installEventFilter(this);
}

// ==================== 边缘检测 ====================
BaseTitleBar::Edge BaseTitleBar::detectEdge(const QPoint& pos) const
{
	int w = this->width();
	int h = this->height();
	int bw = m_borderWidth;

	bool onLeft   = (pos.x() >= 0 && pos.x() <= bw);
	bool onRight  = (pos.x() >= w - bw && pos.x() <= w);
	bool onTop    = (pos.y() >= 0 && pos.y() <= bw);
	bool onBottom = (pos.y() >= h - bw && pos.y() <= h);

	if (onTop && onLeft)     return TopLeft;
	if (onTop && onRight)    return TopRight;
	if (onBottom && onLeft)  return BottomLeft;
	if (onBottom && onRight) return BottomRight;
	if (onLeft)              return Left;
	if (onRight)             return Right;
	if (onTop)               return Top;
	if (onBottom)            return Bottom;

	return None;
}

// ==================== 根据边缘更新鼠标形状 ====================
void BaseTitleBar::updateCursorShape(Edge edge)
{
	switch (edge) {
	case TopLeft:
	case BottomRight:
		this->setCursor(Qt::SizeFDiagCursor);
		break;
	case TopRight:
	case BottomLeft:
		this->setCursor(Qt::SizeBDiagCursor);
		break;
	case Left:
	case Right:
		this->setCursor(Qt::SizeHorCursor);
		break;
	case Top:
	case Bottom:
		this->setCursor(Qt::SizeVerCursor);
		break;
	case None:
	default:
		this->setCursor(Qt::ArrowCursor);
		break;
	}
}

// ==================== 执行窗口缩放 ====================
void BaseTitleBar::resizeWindow(const QPoint& globalPos)
{
	QPoint delta = globalPos - m_resizeStartPos;
	QRect geom = m_resizeStartGeometry;

	int minW = this->minimumWidth();
	int minH = this->minimumHeight();
	if (minW <= 0) minW = 100;
	if (minH <= 0) minH = 100;

	switch (m_resizeEdge) {
	case Left:
		geom.setLeft(m_resizeStartGeometry.left() + delta.x());
		break;
	case Top:
		geom.setTop(m_resizeStartGeometry.top() + delta.y());
		break;
	case Right:
		geom.setRight(m_resizeStartGeometry.right() + delta.x());
		break;
	case Bottom:
		geom.setBottom(m_resizeStartGeometry.bottom() + delta.y());
		break;
	case TopLeft:
		geom.setTop(m_resizeStartGeometry.top() + delta.y());
		geom.setLeft(m_resizeStartGeometry.left() + delta.x());
		break;
	case TopRight:
		geom.setTop(m_resizeStartGeometry.top() + delta.y());
		geom.setRight(m_resizeStartGeometry.right() + delta.x());
		break;
	case BottomLeft:
		geom.setBottom(m_resizeStartGeometry.bottom() + delta.y());
		geom.setLeft(m_resizeStartGeometry.left() + delta.x());
		break;
	case BottomRight:
		geom.setBottom(m_resizeStartGeometry.bottom() + delta.y());
		geom.setRight(m_resizeStartGeometry.right() + delta.x());
		break;
	default:
		return;
	}

	// 确保不小于最小尺寸
	if (geom.width() < minW) {
		if (m_resizeEdge & Left)
			geom.setLeft(geom.right() - minW + 1);
		else
			geom.setRight(geom.left() + minW - 1);
	}
	if (geom.height() < minH) {
		if (m_resizeEdge & Top)
			geom.setTop(geom.bottom() - minH + 1);
		else
			geom.setHeight(minH);
	}

	this->setGeometry(geom);
}

// ==================== 事件过滤器 ====================
bool BaseTitleBar::eventFilter(QObject* watched, QEvent* event)
{
	if (watched != this)
		return QWidget::eventFilter(watched, event);

	// ----- 鼠标移动 -----
	if (event->type() == QEvent::MouseMove) {
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

		// 正在拖动
		if (m_isDragging) {
			move(mouseEvent->globalPos() - m_dragPosition);
			return true;
		}

		// 正在缩放
		if (m_isResizing) {
			resizeWindow(mouseEvent->globalPos());
			return true;
		}

		// 非拖动/缩放状态：检测边缘并更新鼠标形状
		if (!(mouseEvent->buttons() & Qt::LeftButton)) {
			Edge edge = detectEdge(mouseEvent->pos());
			updateCursorShape(edge);
		}
		return false;
	}

	// ----- 鼠标按下 -----
	if (event->type() == QEvent::MouseButtonPress) {
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->button() != Qt::LeftButton)
			return false;

		Edge edge = detectEdge(mouseEvent->pos());

		if (edge != None) {
			// 在边缘 → 开始缩放
			m_isResizing = true;
			m_resizeEdge = edge;
			m_resizeStartPos = mouseEvent->globalPos();
			m_resizeStartGeometry = this->frameGeometry();
			return true;
		}
		else {
			// 不在边缘 → 开始拖动
			m_dragPosition = mouseEvent->globalPos() - frameGeometry().topLeft();
			m_isDragging = true;
			return true;
		}
	}

	// ----- 鼠标释放 -----
	if (event->type() == QEvent::MouseButtonRelease) {
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent->button() == Qt::LeftButton) {
			m_isDragging = false;
			m_isResizing = false;
			m_resizeEdge = None;
			return true;
		}
	}

	return QWidget::eventFilter(watched, event);
}

void BaseTitleBar::init()
{
	this->setWindowFlag(Qt::FramelessWindowHint);
	this->setMouseTracking(true);
	this->setMinimumSize(100, 100);
	this->setMaximumSize(100, 1000);
}

void BaseTitleBar::SetupUI()
{
}