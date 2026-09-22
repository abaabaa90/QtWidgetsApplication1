//#ifndef LEGENDDRAGGABLEPLOT_H
//#define LEGENDDRAGGABLEPLOT_H
//
//#include <QWidget>
//#include <QVBoxLayout>
//#include "qcustomplot.h"
//#include <QMouseEvent>
//#include <QPoint>
//
//class LegendDraggablePlot : public QWidget
//{
//    Q_OBJECT
//
//public:
//    explicit LegendDraggablePlot(QWidget* parent = nullptr);
//
//    QCustomPlot* plot() const { return m_plot; }
//
//    // 图例设置
//    void setLegendDraggable(bool draggable);
//    void setLegendVisible(bool visible);
//    void setLegendPosition(const QPointF& position, QCPLegend::PositionStyle style = QCPLegend::psManual);
//
//    // 添加图形
//    void addGraph(const QString& name = QString(),
//        const QPen& pen = QPen(Qt::blue, 2));
//    void setGraphData(int index, const QVector<double>& x, const QVector<double>& y);
//
//protected:
//    bool eventFilter(QObject* watched, QEvent* event) override;
//
//private:
//    void setupPlot();
//    void setupLegend();
//    bool isMouseOverLegend(const QPoint& pos) const;
//    void startLegendDrag(const QPoint& pos);
//    void updateLegendDrag(const QPoint& pos);
//    void endLegendDrag();
//
//private:
//    QCustomPlot* m_plot;
//    QVBoxLayout* m_layout;
//
//    // 拖动相关变量
//    bool m_legendDragging;
//    QPoint m_dragStartPos;
//    QPointF m_legendStartPos;
//    bool m_legendDraggable;
//};
//
//#endif // LEGENDDRAGGABLEPLOT_H
