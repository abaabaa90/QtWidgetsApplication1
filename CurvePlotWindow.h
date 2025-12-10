#pragma once

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QColor>
#include "qcustomplot.h"
#include <iostream>
#pragma execution_character_set("utf-8")
// 前向声明
class Reader;
QT_BEGIN_NAMESPACE
class QPushButton;
class QCustomPlot;
class QLabel;
class QComboBox;
QT_END_NAMESPACE
class CurvePlotWindow : public QMainWindow
{
    Q_OBJECT

public:
    CurvePlotWindow(QWidget* parent = nullptr);
    ~CurvePlotWindow();

private slots:
    // 按钮点击槽函数
    void onLoadSingleFile();
    void onLoadMultipleFiles();
    void onClearPlot();
    void onSavePlot();

    // 坐标轴选择槽函数
    void onXAxisChanged(int index);
    void onYAxisChanged(int index);

    // 图表交互
    void onMouseMove(QMouseEvent* event);

private:
    // UI控件
    QCustomPlot* m_customPlot;
    QPushButton* m_btnLoadSingle;
    QPushButton* m_btnLoadMultiple;
    QPushButton* m_btnClear;
    QPushButton* m_btnSave;
    QComboBox* m_cmbXAxis;
    QComboBox* m_cmbYAxis;
    QLabel* m_lblCoordinates;
    QLabel* m_lblStatus;
    QString filePath;
    // 数据存储
    QString fileName;
    QVector<QVector<double>> points;
    QVector<QString> variables;

    QVector<QString> m_loadedFiles;//加载的文件
    Reader* m_reader;

    // 当前选择的坐标轴索引
    int m_currentXIndex;
    int m_currentYIndex;

    // 初始化函数
    void initUI();
    void initConnections();
    void setupPlot();

    // 数据操作函数
    bool readFile(const QString& filePath);
    void addCurveToPlot(int fileindex);
    void updatePlot();
    void updateAxisComboBoxes();
    void clearAllData();

    // 工具函数
    QColor getRandomColor(int index) const;
    void showStatus(const QString& message, bool isError = false);
    void resetPlotRange();

    // 界面布局
    QWidget* createControlPanel();
    QWidget* createPlotWidget();
    QWidget* createStatusBar();

    std::vector<Reader*> CurvePlot;
};