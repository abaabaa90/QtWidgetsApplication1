#include "AdvancedCurveStyler.h"
#include <QPainter>
#include <QPolygonF>

AdvancedCurveStyler::AdvancedCurveStyler(QCustomPlot* plot, QObject* parent)
    : QObject(parent)
    , m_plot(plot)
    , m_autoStyleEnabled(true)
    , m_currentColorMode(CategoricalColors)
{
    initializeRandomSeed();
}

void AdvancedCurveStyler::setPlot(QCustomPlot* plot)
{
    m_plot = plot;
}

QCustomPlot* AdvancedCurveStyler::plot() const
{
    return m_plot;
}

void AdvancedCurveStyler::styleCurve(QCPGraph* graph, int index,
    ColorMode mode, bool showPoints)
{
    if (!graph) return;

    CurveStyle style = generateStyle(index, mode);
    style.showPoints = showPoints;

    // 设置线条样式
    QPen pen(style.lineColor, style.lineWidth, style.lineStyle);
    graph->setPen(pen);

    // 设置点样式
    if (style.showPoints && style.pointShape != QCPScatterStyle::ssNone) {
        QCPScatterStyle scatterStyle;
        scatterStyle.setShape(style.pointShape);
        scatterStyle.setBrush(QBrush(style.pointColor));
        scatterStyle.setPen(QPen(Qt::white, 1));
        scatterStyle.setSize(style.pointSize);
        graph->setScatterStyle(scatterStyle);
    }
    else {
        graph->setScatterStyle(QCPScatterStyle());
    }

    emit styleChanged(index);
}

void AdvancedCurveStyler::styleAllCurves(ColorMode mode)
{
    if (!m_plot) return;

    m_currentColorMode = mode;

    for (int i = 0; i < m_plot->graphCount(); ++i) {
        QCPGraph* graph = m_plot->graph(i);
        if (graph) {
            styleCurve(graph, i, mode);
        }
    }

    if (m_plot->graphCount() > 0) {
        emit stylesUpdated();
        emit colorModeChanged(mode);
    }
}

void AdvancedCurveStyler::setCustomStyle(int index, const CurveStyle& style)
{
    if (!m_plot || index < 0 || index >= m_plot->graphCount()) return;

    QCPGraph* graph = m_plot->graph(index);
    if (!graph) return;

    // 设置线条
    QPen pen(style.lineColor, style.lineWidth, style.lineStyle);
    graph->setPen(pen);

    // 设置点
    if (style.showPoints && style.pointShape != QCPScatterStyle::ssNone) {
        QCPScatterStyle scatterStyle;
        scatterStyle.setShape(style.pointShape);
        scatterStyle.setBrush(QBrush(style.pointColor));
        scatterStyle.setPen(QPen(Qt::white, 1));
        scatterStyle.setSize(style.pointSize);
        graph->setScatterStyle(scatterStyle);
    }
    else {
        graph->setScatterStyle(QCPScatterStyle());
    }

    emit styleChanged(index);
}

AdvancedCurveStyler::CurveStyle AdvancedCurveStyler::getCurveStyle(int index) const
{
    CurveStyle style;

    if (!m_plot || index < 0 || index >= m_plot->graphCount()) {
        return style;
    }

    QCPGraph* graph = m_plot->graph(index);
    if (!graph) return style;

    // 获取线条样式
    QPen pen = graph->pen();
    style.lineColor = pen.color();
    style.lineWidth = pen.width();
    style.lineStyle = pen.style();

    // 获取点样式
    QCPScatterStyle scatterStyle = graph->scatterStyle();
    style.pointShape = scatterStyle.shape();
    style.pointColor = scatterStyle.brush().color();
    style.pointSize = scatterStyle.size();
    style.showPoints = (scatterStyle.shape() != QCPScatterStyle::ssNone);

    return style;
}

QList<QColor> AdvancedCurveStyler::getDefaultColors()
{
    return {
        QColor(31, 119, 180),   // 蓝色
        QColor(255, 127, 14),   // 橙色
        QColor(44, 160, 44),    // 绿色
        QColor(214, 39, 40),    // 红色
        QColor(148, 103, 189),  // 紫色
        QColor(140, 86, 75),    // 棕色
        QColor(227, 119, 194),  // 粉色
        QColor(127, 127, 127),  // 灰色
        QColor(188, 189, 34),   // 黄绿色
        QColor(23, 190, 207)    // 青色
    };
}

QList<QCPScatterStyle::ScatterShape> AdvancedCurveStyler::getScatterShapes()
{
    return {
        QCPScatterStyle::ssTriangle,
        QCPScatterStyle::ssCircle,
        QCPScatterStyle::ssSquare,
        QCPScatterStyle::ssDiamond,
        QCPScatterStyle::ssCross,
        QCPScatterStyle::ssPlus,
        QCPScatterStyle::ssStar,
        QCPScatterStyle::ssTriangleInverted,
        QCPScatterStyle::ssCrossSquare,
        QCPScatterStyle::ssPlusSquare
    };
}

QColor AdvancedCurveStyler::generateRandomColor()
{
    // 生成鲜艳的颜色（避免太暗或太亮）
    int h = QRandomGenerator::global()->bounded(360);  // 色调 0-359
    int s = QRandomGenerator::global()->bounded(150, 255);  // 饱和度 59-100%
    int v = QRandomGenerator::global()->bounded(150, 255);  // 亮度 59-100%

    return QColor::fromHsv(h, s, v);
}

QColor AdvancedCurveStyler::generateSequentialColor(int index, int total)
{
    if (total <= 0) total = 10;

    // 使用HSV，固定饱和度，变化色调
    int hue = (index * 360) / total;  // 均匀分布色调
    return QColor::fromHsv(hue % 360, 200, 230);
}

QColor AdvancedCurveStyler::getCategoricalColor(int index)
{
    QList<QColor> colors = getDefaultColors();
    return colors[index % colors.size()];
}

QPixmap AdvancedCurveStyler::createStylePreview(const CurveStyle& style, QSize size)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景（可选）
    painter.setBrush(QBrush(QColor(240, 240, 240)));
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, size.width(), size.height());

    // 绘制线条
    QPen linePen(style.lineColor, style.lineWidth, style.lineStyle);
    painter.setPen(linePen);
    painter.drawLine(10, size.height() / 2, size.width() - 10, size.height() / 2);

    // 绘制点
    if (style.showPoints) {
        QBrush pointBrush(style.pointColor);
        QPen pointPen(Qt::white, 1);

        painter.setBrush(pointBrush);
        painter.setPen(pointPen);

        int centerX = size.width() / 2;
        int centerY = size.height() / 2;
        int radius = style.pointSize / 2;

        // 根据形状绘制点
        switch (style.pointShape) {
        case QCPScatterStyle::ssCircle:
            painter.drawEllipse(QPoint(centerX, centerY), radius, radius);
            break;
        case QCPScatterStyle::ssTriangle: {
            QPolygonF triangle;
            triangle << QPointF(centerX, centerY - radius)
                << QPointF(centerX - radius, centerY + radius)
                << QPointF(centerX + radius, centerY + radius);
            painter.drawPolygon(triangle);
            break;
        }
        case QCPScatterStyle::ssSquare:
            painter.drawRect(centerX - radius, centerY - radius,
                radius * 2, radius * 2);
            break;
        case QCPScatterStyle::ssDiamond: {
            QPolygonF diamond;
            diamond << QPointF(centerX, centerY - radius)
                << QPointF(centerX + radius, centerY)
                << QPointF(centerX, centerY + radius)
                << QPointF(centerX - radius, centerY);
            painter.drawPolygon(diamond);
            break;
        }
        case QCPScatterStyle::ssCross:
            painter.drawLine(centerX - radius, centerY, centerX + radius, centerY);
            painter.drawLine(centerX, centerY - radius, centerX, centerY + radius);
            break;
        case QCPScatterStyle::ssPlus:
            painter.drawLine(centerX - radius, centerY, centerX + radius, centerY);
            painter.drawLine(centerX, centerY - radius, centerX, centerY + radius);
            break;
        default:
            painter.drawEllipse(QPoint(centerX, centerY), radius, radius);
        }
    }

    return pixmap;
}

void AdvancedCurveStyler::updateAllStyles()
{
    styleAllCurves(m_currentColorMode);
}

void AdvancedCurveStyler::setAutoStyleEnabled(bool autoStyle)
{
    m_autoStyleEnabled = autoStyle;
}

bool AdvancedCurveStyler::isAutoStyleEnabled() const
{
    return m_autoStyleEnabled;
}

void AdvancedCurveStyler::applyRandomColors()
{
    styleAllCurves(RandomColors);
}

void AdvancedCurveStyler::applySequentialColors()
{
    styleAllCurves(SequentialColors);
}

void AdvancedCurveStyler::applyCategoricalColors()
{
    styleAllCurves(CategoricalColors);
}

void AdvancedCurveStyler::togglePointsVisibility(bool show)
{
    if (!m_plot) return;

    for (int i = 0; i < m_plot->graphCount(); ++i) {
        QCPGraph* graph = m_plot->graph(i);
        if (graph) {
            CurveStyle style = getCurveStyle(i);
            style.showPoints = show;
            setCustomStyle(i, style);
        }
    }

    if (m_plot->graphCount() > 0) {
        m_plot->replot();
    }
}

void AdvancedCurveStyler::setAllLineWidths(int width)
{
    if (!m_plot) return;

    for (int i = 0; i < m_plot->graphCount(); ++i) {
        QCPGraph* graph = m_plot->graph(i);
        if (graph) {
            QPen pen = graph->pen();
            pen.setWidth(width);
            graph->setPen(pen);
        }
    }

    if (m_plot->graphCount() > 0) {
        m_plot->replot();
        emit stylesUpdated();
    }
}

void AdvancedCurveStyler::setAllPointSizes(int size)
{
    if (!m_plot) return;

    for (int i = 0; i < m_plot->graphCount(); ++i) {
        QCPGraph* graph = m_plot->graph(i);
        if (graph) {
            QCPScatterStyle style = graph->scatterStyle();
            if (style.shape() != QCPScatterStyle::ssNone) {
                style.setSize(size);
                graph->setScatterStyle(style);
            }
        }
    }

    if (m_plot->graphCount() > 0) {
        m_plot->replot();
        emit stylesUpdated();
    }
}

void AdvancedCurveStyler::initializeRandomSeed()
{
    static bool seeded = false;
    if (!seeded) {
        QRandomGenerator::global()->seed(QDateTime::currentMSecsSinceEpoch());
        seeded = true;
    }
}

AdvancedCurveStyler::CurveStyle AdvancedCurveStyler::generateStyle(int index, ColorMode mode)
{
    CurveStyle style;

    // 设置默认值
    style.lineWidth = 2;
    style.lineStyle = Qt::SolidLine;
    style.pointSize = 8;
    style.showPoints = true;

    // 根据模式设置颜色
    switch (mode) {
    case RandomColors:
        style.lineColor = generateRandomColor();
        style.pointColor = Qt::white;
        style.pointShape = getScatterShapes()[index % getScatterShapes().size()];
        break;

    case SequentialColors:
        style.lineColor = generateSequentialColor(index, 10);
        style.pointColor = style.lineColor;
        style.pointShape = QCPScatterStyle::ssCircle;
        break;

    case CategoricalColors:
        style.lineColor = getCategoricalColor(index);
        style.pointColor = Qt::white;
        style.pointShape = getScatterShapes()[index % getScatterShapes().size()];
        break;

    case CustomColors:
        // 对于自定义模式，使用预定义颜色但可以后续修改
        style.lineColor = getCategoricalColor(index);
        style.pointColor = Qt::white;
        style.pointShape = QCPScatterStyle::ssTriangle;
        break;
    }

    return style;
}