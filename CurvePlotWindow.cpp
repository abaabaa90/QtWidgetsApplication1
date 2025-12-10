#include "CurvePlotWindow.h"
#include "Reader.h"
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStatusBar>
#include <QFileInfo>
#include <QDateTime>
#include <QRandomGenerator>
#include <QMouseEvent>
#include <algorithm>
#include <cmath>
#pragma execution_character_set("utf-8")
CurvePlotWindow::CurvePlotWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_reader(new Reader())
    , m_currentXIndex(0)
    , m_currentYIndex(1)
{
    // 设置窗口属性
    setWindowTitle("曲线图绘制工具 - VS2022 + Qt5.14.2");
    resize(1200, 800);

    // 初始化UI
    initUI();
    initConnections();
    setupPlot();

    // 初始状态
    showStatus("就绪");
}

CurvePlotWindow::~CurvePlotWindow()
{
    delete m_reader;
}

void CurvePlotWindow::initUI()
{
    // 创建中央部件
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // 添加控制面板
    mainLayout->addWidget(createControlPanel());

    // 添加绘图区域
    mainLayout->addWidget(createPlotWidget(), 1); // 1表示拉伸因子

    // 添加状态栏区域
    mainLayout->addWidget(createStatusBar());

    // 设置中央部件
    setCentralWidget(centralWidget);
}

QWidget* CurvePlotWindow::createControlPanel()
{
    QGroupBox* controlGroup = new QGroupBox("控制面板", this);
    QHBoxLayout* layout = new QHBoxLayout(controlGroup);

    // 创建按钮
    m_btnLoadSingle = new QPushButton("加载单个文件", this);
    m_btnLoadMultiple = new QPushButton("加载多个文件", this);
    m_btnClear = new QPushButton("清除图表", this);
    m_btnSave = new QPushButton("保存图表", this);

    // 设置按钮样式
    QString buttonStyle = "QPushButton { padding: 8px; font-weight: bold; }";
    m_btnLoadSingle->setStyleSheet(buttonStyle);
    m_btnLoadMultiple->setStyleSheet(buttonStyle);
    m_btnClear->setStyleSheet(buttonStyle);
    m_btnSave->setStyleSheet(buttonStyle);

    // 创建坐标轴选择
    QLabel* lblXAxis = new QLabel("X轴:", this);
    m_cmbXAxis = new QComboBox(this);
    m_cmbXAxis->setMinimumWidth(120);

    QLabel* lblYAxis = new QLabel("Y轴:", this);
    m_cmbYAxis = new QComboBox(this);
    m_cmbYAxis->setMinimumWidth(120);

    // 添加到布局
    layout->addWidget(m_btnLoadSingle);
    layout->addWidget(m_btnLoadMultiple);
    layout->addWidget(m_btnClear);
    layout->addWidget(m_btnSave);
    layout->addStretch();
    layout->addWidget(lblXAxis);
    layout->addWidget(m_cmbXAxis);
    layout->addWidget(lblYAxis);
    layout->addWidget(m_cmbYAxis);

    return controlGroup;
}

QWidget* CurvePlotWindow::createPlotWidget()
{
    QGroupBox* plotGroup = new QGroupBox("曲线图", this);
    QVBoxLayout* layout = new QVBoxLayout(plotGroup);

    // 创建QCustomPlot
    m_customPlot = new QCustomPlot(this);
    layout->addWidget(m_customPlot);

    return plotGroup;
}

QWidget* CurvePlotWindow::createStatusBar()
{
    QWidget* statusWidget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(statusWidget);

    // 坐标显示标签
    m_lblCoordinates = new QLabel("坐标: (0.000, 0.000)", this);
    m_lblCoordinates->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_lblCoordinates->setMinimumWidth(200);

    // 状态显示标签
    m_lblStatus = new QLabel("就绪", this);
    m_lblStatus->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_lblStatus->setMinimumWidth(400);

    // 添加到布局
    layout->addWidget(m_lblCoordinates);
    layout->addStretch();
    layout->addWidget(m_lblStatus);

    return statusWidget;
}

void CurvePlotWindow::initConnections()
{
    // 按钮信号连接
    connect(m_btnLoadSingle, &QPushButton::clicked, this, &CurvePlotWindow::onLoadSingleFile);
    connect(m_btnLoadMultiple, &QPushButton::clicked, this, &CurvePlotWindow::onLoadMultipleFiles);
    connect(m_btnClear, &QPushButton::clicked, this, &CurvePlotWindow::onClearPlot);
    connect(m_btnSave, &QPushButton::clicked, this, &CurvePlotWindow::onSavePlot);

    // 坐标轴下拉框信号连接
    connect(m_cmbXAxis, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &CurvePlotWindow::onXAxisChanged);
    connect(m_cmbYAxis, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &CurvePlotWindow::onYAxisChanged);

    // 鼠标移动信号连接
    connect(m_customPlot, &QCustomPlot::mouseMove, this, &CurvePlotWindow::onMouseMove);
}

void CurvePlotWindow::setupPlot()
{
    // 设置背景
    m_customPlot->setBackground(QBrush(QColor(255, 255, 255)));

    // 设置坐标轴
    m_customPlot->xAxis->setLabel("X轴");
    m_customPlot->yAxis->setLabel("Y轴");

    // 设置坐标轴标签字体
    QFont labelFont("Microsoft YaHei", 10, QFont::Bold);
    m_customPlot->xAxis->setLabelFont(labelFont);
    m_customPlot->yAxis->setLabelFont(labelFont);

    // 设置坐标轴刻度字体
    QFont tickFont("Microsoft YaHei", 9);
    m_customPlot->xAxis->setTickLabelFont(tickFont);
    m_customPlot->yAxis->setTickLabelFont(tickFont);

    // 设置初始范围
    m_customPlot->xAxis->setRange(-10, 10);
    m_customPlot->yAxis->setRange(-10, 10);

    // 设置网格
    m_customPlot->xAxis->grid()->setVisible(true);
    m_customPlot->yAxis->grid()->setVisible(true);
    m_customPlot->xAxis->grid()->setSubGridVisible(true);
    m_customPlot->yAxis->grid()->setSubGridVisible(true);
    m_customPlot->xAxis->grid()->setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));
    m_customPlot->yAxis->grid()->setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));

    // 设置图例
    m_customPlot->legend->setVisible(true);
    m_customPlot->legend->setFont(QFont("Microsoft YaHei", 9));
    m_customPlot->legend->setBrush(QBrush(QColor(255, 255, 255, 200)));
    m_customPlot->legend->setBorderPen(QPen(QColor(150, 150, 150, 200)));

    // 启用交互功能
    m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    // 初始化坐标轴下拉框
    updateAxisComboBoxes();
}

QColor CurvePlotWindow::getRandomColor(int index) const
{
    // 预定义一组美观的颜色
    static const QVector<QColor> colorTable = {
        QColor(31, 119, 180),    // 蓝色
        QColor(255, 127, 14),    // 橙色
        QColor(44, 160, 44),     // 绿色
        QColor(214, 39, 40),     // 红色
        QColor(148, 103, 189),   // 紫色
        QColor(140, 86, 75),     // 棕色
        QColor(227, 119, 194),   // 粉色
        QColor(127, 127, 127),   // 灰色
        QColor(188, 189, 34),    // 橄榄色
        QColor(23, 190, 207)     // 青色
    };

    // 如果索引在预定义颜色范围内，使用预定义颜色
    if (index >= 0 && index < colorTable.size()) {
        return colorTable[index];
    }

    // 否则生成随机颜色
    return QColor::fromHsv(
        QRandomGenerator::global()->bounded(360),  // 色调 (0-359)
        150 + QRandomGenerator::global()->bounded(106),  // 饱和度 (150-255)
        150 + QRandomGenerator::global()->bounded(106)   // 亮度 (150-255)
    );
}

void CurvePlotWindow::updateAxisComboBoxes()//更新坐标系的x轴和y轴吗
{
    // 阻塞信号，避免多次触发
    m_cmbXAxis->blockSignals(true);
    m_cmbYAxis->blockSignals(true);

    // 清空下拉框
    m_cmbXAxis->clear();
    m_cmbYAxis->clear();

    if (m_loadedFiles.isEmpty()) {
        // 如果没有加载文件，使用默认选项
        m_cmbXAxis->addItem("X");
        m_cmbYAxis->addItem("Y");

        m_currentXIndex = 0;
        m_currentYIndex = 1;
    }
    else {
        // 使用第一个文件的变量列表
        
        auto variables = CurvePlot[0]->Variables;
        for (int i = 0; i < variables.size(); ++i) {
            QString varName =  QString::fromStdString(variables[i]);
            if (varName.isEmpty()) {
                std::cout << "变量名为空\n";
                varName = QString("变量%1").arg(i + 1);
            }
            m_cmbXAxis->addItem(varName);
            m_cmbYAxis->addItem(varName);
        }

        // 确保当前索引在有效范围内
        if (m_currentXIndex >= m_cmbXAxis->count()) {
            m_currentXIndex = 0;
        }
        if (m_currentYIndex >= m_cmbYAxis->count()) {
            m_currentYIndex = qMin(1, m_cmbYAxis->count() - 1);
        }

        // 设置当前选择
        m_cmbXAxis->setCurrentIndex(m_currentXIndex);
        m_cmbYAxis->setCurrentIndex(m_currentYIndex);
    }

    // 恢复信号
    m_cmbXAxis->blockSignals(false);
    m_cmbYAxis->blockSignals(false);
}

bool CurvePlotWindow::readFile(const QString& filePath)//传进来文件路径然后呢
{
    QFileInfo fileInfo(filePath);

    // 检查文件是否存在
    if (!fileInfo.exists()) {
        showStatus(QString("文件不存在: %1").arg(filePath), true);
        return false;
    }

    // 检查文件大小
    if (fileInfo.size() == 0) {
        showStatus(QString("文件为空: %1").arg(filePath), true);
        return false;
    }

    // 使用Reader读取文件
    QByteArray filePathBytes = filePath.toLocal8Bit();
    if (!m_reader->ReadFile(filePathBytes.constData())) {
        showStatus(QString("无法读取文件: %1").arg(filePath), true);
        return false;
    }

    // 检查数据维度
    size_t dimension = m_reader->GetDimension();
    if (dimension < 2) {
        showStatus(QString("文件维度小于2: %1").arg(filePath), true);
        return false;
    }

    // 检查数据点数量
    size_t pointCount = m_reader->GetPointCount();
    if (pointCount == 0) {
        showStatus(QString("文件中没有有效数据点: %1").arg(filePath), true);
        return false;
    }

    return true;
}
//这个类应该只接收传过来的数据,然后，传进来的应该是什么呢，tecplot里面是一个变量列表和一个长数组,而我这里只是简单的二维坐标

void CurvePlotWindow::addCurveToPlot(int fileindex)//添加曲线
{
    // 检查坐标轴索引是否有效
    int xIdx = m_currentXIndex;
    int yIdx = m_currentYIndex;

    
    // 准备X和Y数据
    QVector<double> xData, yData;
    for (size_t i = 0; i < CurvePlot[fileindex]->Points.size(); i++)//遍历其每一行,然后遍历列
    {
        std::cout << "X:" << CurvePlot[fileindex]->Points[i][0] << "Y:" << CurvePlot[fileindex]->Points[i][1] << std::endl;
        for (size_t j = 0; j < 2; j++)
        {
            if (j == 0)xData.push_back(CurvePlot[fileindex]->Points[i][0]);
            if (j == 1)yData.push_back(CurvePlot[fileindex]->Points[i][1]);
        }
    }
    if (xData.isEmpty() || yData.isEmpty()) {
        return;
    }

    // 创建图形
    m_customPlot->addGraph();
    QCPGraph* graph = m_customPlot->graph();
    graph->setData(xData, yData);

    // 设置曲线名称
    QString graphName = QString::fromStdString(CurvePlot[0]->DataTitle);
    graph->setName(graphName);
    // 设置曲线样式
    graph->setPen(QPen(Qt::black, 2));
    graph->setLineStyle(QCPGraph::lsLine);
    graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::black, Qt::white, 6));
}

void CurvePlotWindow::updatePlot()
{
    m_customPlot->clearGraphs();

    // 添加所有文件的曲线
    for (int i = 0; i < CurvePlot.size(); ++i) {
        addCurveToPlot(i);
    }

    // 自动调整坐标轴范围
    m_customPlot->rescaleAxes();

    // 稍微扩大范围以便美观
    m_customPlot->xAxis->scaleRange(1.1, m_customPlot->xAxis->range().center());
    m_customPlot->yAxis->scaleRange(1.1, m_customPlot->yAxis->range().center());

    // 更新坐标轴标签
    m_customPlot->xAxis->setLabel(QString::fromStdString(CurvePlot[0]->Variables[0]));
    m_customPlot->yAxis->setLabel(QString::fromStdString(CurvePlot[0]->Variables[1]));

    // 重绘图
    m_customPlot->replot();
    showStatus(QString("已加载 %1 个文件，共 %2 个数据点")
        .arg(m_loadedFiles.size()));
}

void CurvePlotWindow::resetPlotRange()
{
    m_customPlot->xAxis->setRange(-10, 10);
    m_customPlot->yAxis->setRange(-10, 10);
    m_customPlot->xAxis->setLabel("X轴");
    m_customPlot->yAxis->setLabel("Y轴");
    m_customPlot->replot();
}

void CurvePlotWindow::showStatus(const QString& message, bool isError)
{
    m_lblStatus->setText(message);

    // 根据消息类型设置颜色
    if (isError) {
        m_lblStatus->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    }
    else {
        m_lblStatus->setStyleSheet("QLabel { color: green; font-weight: bold; }");
    }
}

void CurvePlotWindow::clearAllData()
{
    m_loadedFiles.clear();
    m_customPlot->clearGraphs();
    resetPlotRange();
    updateAxisComboBoxes();
    showStatus("已清除所有数据");
}

// ==================== 槽函数实现 ====================

void CurvePlotWindow::onLoadSingleFile()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择数据文件",
        QDir::currentPath(),
        "文本文件 (*.txt);;所有文件 (*.*)"
    );
    qDebug() << filePath << endl;//这里我需要看一下是绝对路径还是相对路径
    Reader* t = new Reader();
    if (t->ReadFile(filePath.toStdString().c_str())) {
        CurvePlot.push_back(t);
        updateAxisComboBoxes();
        updatePlot();
        showStatus(QString("成功加载文件: %1").arg(QFileInfo(filePath).fileName()));
    }
}

void CurvePlotWindow::onLoadMultipleFiles()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        "选择多个数据文件",
        QDir::currentPath(),
        "文本文件 (*.txt);;所有文件 (*.*)"
    );

    if (filePaths.isEmpty()) {
        return;
    }

    int successCount = 0;
    for (const QString& filePath : filePaths) {
        if (readFile(filePath)) {
            successCount++;
        }
    }

    if (successCount > 0) {
        updateAxisComboBoxes();
        updatePlot();
        showStatus(QString("成功加载 %1/%2 个文件").arg(successCount).arg(filePaths.size()));
    }
}

void CurvePlotWindow::onClearPlot()
{
    if (m_loadedFiles.isEmpty()) {
        showStatus("没有数据需要清除");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认清除",
        "确定要清除所有曲线数据吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        clearAllData();
    }
}

void CurvePlotWindow::onSavePlot()
{
    if (m_loadedFiles.isEmpty()) {
        showStatus("没有数据可以保存", true);
        return;
    }

    QString defaultName = QString("plot_%1.png")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

    QString filePath = QFileDialog::getSaveFileName(
        this,
        "保存图表",
        QDir::currentPath() + "/" + defaultName,
        "PNG图像 (*.png);;JPEG图像 (*.jpg);;PDF文件 (*.pdf);;BMP图像 (*.bmp)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    // 根据文件扩展名确定保存格式
    QString suffix = QFileInfo(filePath).suffix().toLower();
    bool success = false;

    if (suffix == "png") {
        success = m_customPlot->savePng(filePath, 1920, 1080, 2.0);
    }
    else if (suffix == "jpg" || suffix == "jpeg") {
        success = m_customPlot->saveJpg(filePath, 1920, 1080, 2.0);
    }
    else if (suffix == "pdf") {
        success = m_customPlot->savePdf(filePath);
    }
    else if (suffix == "bmp") {
        success = m_customPlot->saveBmp(filePath, 1920, 1080, 2.0);
    }
    else {
        // 默认保存为PNG
        filePath += ".png";
        success = m_customPlot->savePng(filePath, 1920, 1080, 2.0);
    }

    if (success) {
        showStatus(QString("图表已保存到: %1").arg(QFileInfo(filePath).fileName()));
    }
    else {
        showStatus("保存图表失败", true);
    }
}

void CurvePlotWindow::onXAxisChanged(int index)
{
    if (index >= 0) {
        m_currentXIndex = index;
        updatePlot();
        showStatus(QString("X轴已设置为: %1").arg(m_cmbXAxis->itemText(index)));
    }
}

void CurvePlotWindow::onYAxisChanged(int index)
{
    if (index >= 0) {
        m_currentYIndex = index;
        updatePlot();
        showStatus(QString("Y轴已设置为: %1").arg(m_cmbYAxis->itemText(index)));
    }
}

void CurvePlotWindow::onMouseMove(QMouseEvent* event)
{
    if (m_customPlot->graphCount() > 0) {
        double x = m_customPlot->xAxis->pixelToCoord(event->pos().x());
        double y = m_customPlot->yAxis->pixelToCoord(event->pos().y());
        m_lblCoordinates->setText(QString("坐标: (%1, %2)").arg(x, 0, 'f', 3).arg(y, 0, 'f', 3));
    }
    else {
        m_lblCoordinates->setText("坐标: (0.000, 0.000)");
    }
}