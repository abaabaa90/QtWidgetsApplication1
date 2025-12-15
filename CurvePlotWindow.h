#pragma once

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QColor>
#include "qcustomplot.h"
#include <iostream>
#pragma execution_character_set("utf-8")
// ǰ������
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
    // ��ť����ۺ���
    void onLoadSingleFile();
    void onLoadMultipleFiles();
    void onClearPlot();
    void onSavePlot();

    // ������ѡ��ۺ���
    void onXAxisChanged(int index);
    void onYAxisChanged(int index);

    // ͼ�����
    void onMouseMove(QMouseEvent* event);

private:
    // UI�ؼ�
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
    // ���ݴ洢
    QString fileName;
    QVector<QVector<double>> points;
    QVector<QString> variables;

    QVector<QString> m_loadedFiles;//���ص��ļ�
    Reader* m_reader;

    // ��ǰѡ�������������
    int m_currentXIndex;
    int m_currentYIndex;

    // ��ʼ������
    void initUI();
    void initConnections();
    void setupPlot();

    // ���ݲ�������
    bool readFile(const QString& filePath);
    void addCurveToPlot(int fileindex);
    void updatePlot();
    void updateAxisComboBoxes();
    void clearAllData();

    // ���ߺ���
    QColor getRandomColor(int index) const;
    void showStatus(const QString& message, bool isError = false);
    void resetPlotRange();

    // ���沼��
    QWidget* createControlPanel();
    QWidget* createPlotWidget();
    QWidget* createStatusBar();

    std::vector<Reader*> CurvePlot;
};