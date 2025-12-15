#ifndef ADVANCEDCURVESTYLER_H
#define ADVANCEDCURVESTYLER_H

#include <QObject>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QRandomGenerator>
#include <QDateTime>
#include <QPixmap>
#include <QSize>

#include "qcustomplot.h"

/**
 * @brief 高级曲线样式管理器
 *
 * 这个类用于管理 QCustomPlot 中曲线的样式，支持随机颜色、顺序颜色、分类颜色等多种颜色模式，
 * 以及多种点样式（三角形、圆形、方形等）和自定义线条样式。
 */
class AdvancedCurveStyler : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 颜色模式枚举
     */
    enum ColorMode {
        RandomColors,      ///< 随机颜色模式
        SequentialColors,  ///< 顺序颜色模式
        CategoricalColors, ///< 分类颜色模式
        CustomColors       ///< 自定义颜色模式
    };
    Q_ENUM(ColorMode)

        /**
         * @brief 曲线样式结构体
         *
         * 包含线条颜色、点颜色、线宽、线条样式、点形状、点大小等样式属性
         */
        struct CurveStyle {
        QColor lineColor;                    ///< 线条颜色
        QColor pointColor;                   ///< 点填充颜色
        int lineWidth;                       ///< 线宽
        Qt::PenStyle lineStyle;              ///< 线条样式（实线、虚线等）
        QCPScatterStyle::ScatterShape pointShape;  ///< 点形状
        int pointSize;                       ///< 点大小
        bool showPoints;                     ///< 是否显示点

        /**
         * @brief 默认构造函数
         */
        CurveStyle()
            : lineColor(Qt::blue)
            , pointColor(Qt::white)
            , lineWidth(2)
            , lineStyle(Qt::SolidLine)
            , pointShape(QCPScatterStyle::ssCircle)
            , pointSize(8)
            , showPoints(true)
        {}
    };

    /**
     * @brief 构造函数
     * @param plot 要管理的 QCustomPlot 对象
     * @param parent 父对象
     */
    explicit AdvancedCurveStyler(QCustomPlot* plot = nullptr, QObject* parent = nullptr);

    /**
     * @brief 设置要管理的 QCustomPlot 对象
     * @param plot QCustomPlot 对象
     */
    void setPlot(QCustomPlot* plot);

    /**
     * @brief 获取当前管理的 QCustomPlot 对象
     * @return 当前管理的 QCustomPlot 对象
     */
    QCustomPlot* plot() const;

    /**
     * @brief 为单条曲线应用样式
     * @param graph 要应用样式的曲线对象
     * @param index 曲线索引（用于颜色生成）
     * @param mode 颜色模式（默认为分类颜色模式）
     * @param showPoints 是否显示点（默认为true）
     */
    void styleCurve(QCPGraph* graph, int index,
        ColorMode mode = CategoricalColors,
        bool showPoints = true);

    /**
     * @brief 为所有曲线应用样式
     * @param mode 颜色模式（默认为分类颜色模式）
     */
    void styleAllCurves(ColorMode mode = CategoricalColors);

    /**
     * @brief 为指定索引的曲线应用自定义样式
     * @param index 曲线索引
     * @param style 自定义样式
     */
    void setCustomStyle(int index, const CurveStyle& style);

    /**
     * @brief 获取指定索引曲线的当前样式
     * @param index 曲线索引
     * @return 曲线样式
     */
    CurveStyle getCurveStyle(int index) const;

    /**
     * @brief 获取默认的颜色列表（预定义的分类颜色）
     * @return 颜色列表
     */
    static QList<QColor> getDefaultColors();

    /**
     * @brief 获取默认的点形状列表
     * @return 点形状列表
     */
    static QList<QCPScatterStyle::ScatterShape> getScatterShapes();

    /**
     * @brief 生成随机颜色
     * @return 随机生成的颜色
     */
    static QColor generateRandomColor();

    /**
     * @brief 生成顺序颜色（基于索引）
     * @param index 索引
     * @param total 总数（用于归一化）
     * @return 生成的颜色
     */
    static QColor generateSequentialColor(int index, int total = 10);

    /**
     * @brief 生成分类颜色（基于索引）
     * @param index 索引
     * @return 分类颜色
     */
    static QColor getCategoricalColor(int index);

    /**
     * @brief 创建样式预览图像
     * @param style 要预览的样式
     * @param size 预览图像大小
     * @return 预览图像
     */
    static QPixmap createStylePreview(const CurveStyle& style,
        QSize size = QSize(100, 30));

    /**
     * @brief 重新应用所有曲线的样式
     *
     * 当添加新曲线后调用此方法，确保所有曲线都有合适的样式
     */
    void updateAllStyles();

    /**
     * @brief 设置是否自动为新曲线应用样式
     * @param autoStyle 是否自动应用样式
     */
    void setAutoStyleEnabled(bool autoStyle);

    /**
     * @brief 获取是否启用了自动样式
     * @return 是否启用了自动样式
     */
    bool isAutoStyleEnabled() const;

public slots:
    /**
     * @brief 应用随机颜色到所有曲线
     */
    void applyRandomColors();

    /**
     * @brief 应用顺序颜色到所有曲线
     */
    void applySequentialColors();

    /**
     * @brief 应用分类颜色到所有曲线
     */
    void applyCategoricalColors();

    /**
     * @brief 切换是否显示数据点
     * @param show 是否显示
     */
    void togglePointsVisibility(bool show);

    /**
     * @brief 设置所有曲线的线宽
     * @param width 线宽
     */
    void setAllLineWidths(int width);

    /**
     * @brief 设置所有曲线的点大小
     * @param size 点大小
     */
    void setAllPointSizes(int size);

signals:
    /**
     * @brief 当曲线样式发生变化时发出的信号
     * @param index 曲线索引
     */
    void styleChanged(int index);

    /**
     * @brief 当所有曲线样式已更新时发出的信号
     */
    void stylesUpdated();

    /**
     * @brief 当颜色模式发生变化时发出的信号
     * @param mode 新的颜色模式
     */
    void colorModeChanged(ColorMode mode);

protected:
    /**
     * @brief 初始化随机种子
     */
    void initializeRandomSeed();

    /**
     * @brief 生成样式（内部使用）
     * @param index 索引
     * @param mode 颜色模式
     * @return 生成的样式
     */
    CurveStyle generateStyle(int index, ColorMode mode);

private:
    QCustomPlot* m_plot;           ///< 管理的 QCustomPlot 对象
    bool m_autoStyleEnabled;       ///< 是否自动为新曲线应用样式
    ColorMode m_currentColorMode;  ///< 当前颜色模式
};

#endif // ADVANCEDCURVESTYLER_H