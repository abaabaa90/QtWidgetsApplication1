//#include "LegendDraggablePlot.h"
//#include <QMouseEvent>
//#include <QVBoxLayout>
//#include <QDebug>
//
//LegendDraggablePlot::LegendDraggablePlot(QWidget* parent)
//    : QWidget(parent)
//    , m_plot(new QCustomPlot(this))
//    , m_layout(new QVBoxLayout(this))
//    , m_legendDragging(false)
//    , m_legendDraggable(true)
//{
//    setupPlot();
//    setupLegend();
//
//    m_layout->setContentsMargins(0, 0, 0, 0);
//    m_layout->addWidget(m_plot);
//
//    // 安装事件过滤器
//    m_plot->installEventFilter(this);
//}
//
//void LegendDraggablePlot::setupPlot()
//{
//    m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
//
//    // 设置轴
//    m_plot->xAxis->setLabel("X轴");
//    m_plot->yAxis->setLabel("Y轴");
//
//    // 设置网格
//    m_plot->xAxis->grid()->setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));
//    m_plot->yAxis->grid()->setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));
//    m_plot->xAxis->grid()->setSubGridPen(QPen(QColor(220, 220, 220), 1, Qt::DotLine));
//    m_plot->yAxis->grid()->setSubGridPen(QPen(QColor(220, 220, 220), 1, Qt::DotLine));
//    m_plot->xAxis->grid()->setSubGridVisible(true);
//    m_plot->yAxis->grid()->setSubGridVisible(true);
//
//    // 设置背景
//    m_plot->setBackground(QBrush(QColor(255, 255, 255)));
//}
//
//void LegendDraggablePlot::setupLegend()
//{
//    // 设置图例
//    m_plot->legend->setVisible(true);
//    m_plot->legend->setBrush(QBrush(QColor(255, 255, 255, 230)));
//    m_plot->legend->setBorderPen(QPen(QColor(180, 180, 180), 1));
//    m_plot->legend->setFont(QFont("Arial", 10));
//
//    // 设置图例位置
//    m_plot->legend->setPositionStyle(QCPLegend::psTopRight);
//    m_plot->legend->setMargins(QMargins(10, 10, 10, 10));
//    m_plot->legend->setPadding(QMargins(8, 8, 8, 8));
//
//    // 设置图例可选（允许选择图例项）
//    m_plot->legend->setSelectableParts(QCPLegend::spItems);
//
//    // 添加信号槽连接
//    connect(m_plot, &QCustomPlot::legendClick, [=](QCPLegend* legend, QCPAbstractLegendItem* item, QMouseEvent* event) {
//        if (item && event->button() == Qt::LeftButton) {
//            // 点击图例项可以切换图形的可见性
//            QCPPlottableLegendItem* plItem = qobject_cast<QCPPlottableLegendItem*>(item);
//            if (plItem) {
//                bool visible = plItem->plottable()->visible();
//                plItem->plottable()->setVisible(!visible);
//                m_plot->replot();
//            }
//        }
//        });
//}
//
//void LegendDraggablePlot::setLegendDraggable(bool draggable)
//{
//    m_legendDraggable = draggable;
//}
//
//void LegendDraggablePlot::setLegendVisible(bool visible)
//{
//    m_plot->legend->setVisible(visible);
//    m_plot->replot();
//}
//
//void LegendDraggablePlot::setLegendPosition(const QPointF& position, QCPLegend::PositionStyle style)
//{
//    m_plot->legend->setPositionStyle(style);
//    if (style == QCPLegend::psManual) {
//        m_plot->legend->topLeft()->setPixelPosition(position.toPoint());
//    }
//    m_plot->replot();
//}
//
//void LegendDraggablePlot::addGraph(const QString& name, const QPen& pen)
//{
//    QCPGraph* graph = m_plot->addGraph();
//    graph->setPen(pen);
//    graph->setName(name);
//
//    // 设置散点样式
//    QCPScatterStyle scatterStyle(QCPScatterStyle::ssCircle, pen.color(), pen.color(), 6);
//    graph->setScatterStyle(scatterStyle);
//
//    // 设置图例项可选
//    QCPPlottableLegendItem* legendItem = m_plot->legend->itemWithPlottable(graph);
//    if (legendItem) {
//        legendItem->setSelectable(true);
//    }
//}
//
//void LegendDraggablePlot::setGraphData(int index, const QVector<double>& x, const QVector<double>& y)
//{
//    if (index >= 0 && index < m_plot->graphCount()) {
//        m_plot->graph(index)->setData(x, y);
//    }
//}
//
//bool LegendDraggablePlot::isMouseOverLegend(const QPoint& pos) const
//{
//    if (!m_plot->legend->visible()) {
//        return false;
//    }
//
//    // 获取图例在图表坐标系中的位置
//    QPoint legendTopLeft = m_plot->legend->topLeft()->pixelPosition().toPoint();
//    QSize legendSize = m_plot->legend->rect().size();
//
//    QRect legendRect(legendTopLeft, legendSize);
//    return legendRect.contains(pos);
//}
//
//void LegendDraggablePlot::startLegendDrag(const QPoint& pos)
//{
//    if (!m_legendDraggable || !isMouseOverLegend(pos)) {
//        return;
//    }
//
//    m_legendDragging = true;
//    m_dragStartPos = pos;
//    m_legendStartPos = m_plot->legend->topLeft()->pixelPosition();
//
//    // 改变图例外观
//    m_plot->legend->setBrush(QBrush(QColor(255, 255, 255, 200)));
//    m_plot->legend->setBorderPen(QPen(Qt::red, 2));
//    m_plot->replot();
//}
//
//void LegendDraggablePlot::updateLegendDrag(const QPoint& pos)
//{
//    if (!m_legendDragging) {
//        return;
//    }
//
//    QPoint delta = pos - m_dragStartPos;
//    QPointF newPos = m_legendStartPos + QPointF(delta);
//
//    m_plot->legend->setPositionStyle(QCPLegend::psManual);
//    m_plot->legend->topLeft()->setPixelPosition(newPos);
//    m_plot->replot();
//}
//
//void LegendDraggablePlot::endLegendDrag()
//{
//    if (m_legendDragging) {
//        m_legendDragging = false;
//
//        // 恢复图例外观
//        m_plot->legend->setBrush(QBrush(QColor(255, 255, 255, 230)));
//        m_plot->legend->setBorderPen(QPen(QColor(180, 180, 180), 1));
//        m_plot->replot();
//    }
//}
//
//bool LegendDraggablePlot::eventFilter(QObject* watched, QEvent* event)
//{
//    if (watched == m_plot) {
//        QMouseEvent* mouseEvent = nullptr;
//
//        switch (event->type()) {
//        case QEvent::MouseButtonPress:
//            mouseEvent = static_cast<QMouseEvent*>(event);
//            if (mouseEvent->button() == Qt::LeftButton) {
//                startLegendDrag(mouseEvent->pos());
//                if (m_legendDragging) {
//                    return true;
//                }
//            }
//            break;
//
//        case QEvent::MouseMove:
//            mouseEvent = static_cast<QMouseEvent*>(event);
//            if (m_legendDragging) {
//                updateLegendDrag(mouseEvent->pos());
//                return true;
//            }
//            break;
//
//        case QEvent::MouseButtonRelease:
//            if (m_legendDragging) {
//                endLegendDrag();
//                return true;
//            }
//            break;
//
//        default:
//            break;
//        }
//    }
//
//    return QWidget::eventFilter(watched, event);
//}