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
    // ���ô�������
    setWindowTitle("����ͼ���ƹ��� - VS2022 + Qt5.14.2");
    resize(1200, 800);

    // ��ʼ��UI
    initUI();
    initConnections();
    setupPlot();

    // ��ʼ״̬
    showStatus("����");
}

CurvePlotWindow::~CurvePlotWindow()
{
    delete m_reader;
}

void CurvePlotWindow::initUI()
{
    // �������벿��
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // ��ӿ������
    mainLayout->addWidget(createControlPanel());

    // ��ӻ�ͼ����
    mainLayout->addWidget(createPlotWidget(), 1); // 1��ʾ��������

    // ���״̬������
    mainLayout->addWidget(createStatusBar());

    // �������벿��
    setCentralWidget(centralWidget);
}

QWidget* CurvePlotWindow::createControlPanel()
{
    QGroupBox* controlGroup = new QGroupBox("�������", this);
    QHBoxLayout* layout = new QHBoxLayout(controlGroup);

    // ������ť
    m_btnLoadSingle = new QPushButton("���ص����ļ�", this);
    m_btnLoadMultiple = new QPushButton("���ض���ļ�", this);
    m_btnClear = new QPushButton("���ͼ��", this);
    m_btnSave = new QPushButton("����ͼ��", this);

    // ���ð�ť��ʽ
    QString buttonStyle = "QPushButton { padding: 8px; font-weight: bold; }";
    m_btnLoadSingle->setStyleSheet(buttonStyle);
    m_btnLoadMultiple->setStyleSheet(buttonStyle);
    m_btnClear->setStyleSheet(buttonStyle);
    m_btnSave->setStyleSheet(buttonStyle);

    // ����������ѡ��
    QLabel* lblXAxis = new QLabel("X��:", this);
    m_cmbXAxis = new QComboBox(this);
    m_cmbXAxis->setMinimumWidth(120);

    QLabel* lblYAxis = new QLabel("Y��:", this);
    m_cmbYAxis = new QComboBox(this);
    m_cmbYAxis->setMinimumWidth(120);

    // ��ӵ�����
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
    QGroupBox* plotGroup = new QGroupBox("����ͼ", this);
    QVBoxLayout* layout = new QVBoxLayout(plotGroup);

    // ����QCustomPlot
    m_customPlot = new QCustomPlot(this);
    layout->addWidget(m_customPlot);

    return plotGroup;
}

QWidget* CurvePlotWindow::createStatusBar()
{
    QWidget* statusWidget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(statusWidget);

    // ������ʾ��ǩ
    m_lblCoordinates = new QLabel("����: (0.000, 0.000)", this);
    m_lblCoordinates->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_lblCoordinates->setMinimumWidth(200);

    // ״̬��ʾ��ǩ
    m_lblStatus = new QLabel("����", this);
    m_lblStatus->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_lblStatus->setMinimumWidth(400);

    // ��ӵ�����
    layout->addWidget(m_lblCoordinates);
    layout->addStretch();
    layout->addWidget(m_lblStatus);

    return statusWidget;
}

void CurvePlotWindow::initConnections()
{
    // ��ť�ź�����
    connect(m_btnLoadSingle, &QPushButton::clicked, this, &CurvePlotWindow::onLoadSingleFile);
    connect(m_btnLoadMultiple, &QPushButton::clicked, this, &CurvePlotWindow::onLoadMultipleFiles);
    connect(m_btnClear, &QPushButton::clicked, this, &CurvePlotWindow::onClearPlot);
    connect(m_btnSave, &QPushButton::clicked, this, &CurvePlotWindow::onSavePlot);

    // �������������ź�����
    connect(m_cmbXAxis, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &CurvePlotWindow::onXAxisChanged);
    connect(m_cmbYAxis, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &CurvePlotWindow::onYAxisChanged);

    // ����ƶ��ź�����
    connect(m_customPlot, &QCustomPlot::mouseMove, this, &CurvePlotWindow::onMouseMove);
}

void CurvePlotWindow::setupPlot()
{
    // ���ñ���
    m_customPlot->setBackground(QBrush(QColor(255, 255, 255)));

    // ����������
    m_customPlot->xAxis->setLabel("X��");
    m_customPlot->yAxis->setLabel("Y��");

    // �����������ǩ����
    QFont labelFont("Microsoft YaHei", 10, QFont::Bold);
    m_customPlot->xAxis->setLabelFont(labelFont);
    m_customPlot->yAxis->setLabelFont(labelFont);

    // ����������̶�����
    QFont tickFont("Microsoft YaHei", 9);
    m_customPlot->xAxis->setTickLabelFont(tickFont);
    m_customPlot->yAxis->setTickLabelFont(tickFont);

    // ���ó�ʼ��Χ
    m_customPlot->xAxis->setRange(-10, 10);
    m_customPlot->yAxis->setRange(-10, 10);

    // ��������
    m_customPlot->xAxis->grid()->setVisible(true);
    m_customPlot->yAxis->grid()->setVisible(true);
    m_customPlot->xAxis->grid()->setSubGridVisible(true);
    m_customPlot->yAxis->grid()->setSubGridVisible(true);
    m_customPlot->xAxis->grid()->setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));
    m_customPlot->yAxis->grid()->setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));

    // ����ͼ��
    m_customPlot->legend->setVisible(true);
    m_customPlot->legend->setFont(QFont("Microsoft YaHei", 9));
    m_customPlot->legend->setBrush(QBrush(QColor(255, 255, 255, 200)));
    m_customPlot->legend->setBorderPen(QPen(QColor(150, 150, 150, 200)));

    // ���ý�������
    m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    // ��ʼ��������������
    updateAxisComboBoxes();
}

QColor CurvePlotWindow::getRandomColor(int index) const
{
    // Ԥ����һ�����۵���ɫ
    static const QVector<QColor> colorTable = {
        QColor(31, 119, 180),    // ��ɫ
        QColor(255, 127, 14),    // ��ɫ
        QColor(44, 160, 44),     // ��ɫ
        QColor(214, 39, 40),     // ��ɫ
        QColor(148, 103, 189),   // ��ɫ
        QColor(140, 86, 75),     // ��ɫ
        QColor(227, 119, 194),   // ��ɫ
        QColor(127, 127, 127),   // ��ɫ
        QColor(188, 189, 34),    // ���ɫ
        QColor(23, 190, 207)     // ��ɫ
    };

    // ���������Ԥ������ɫ��Χ�ڣ�ʹ��Ԥ������ɫ
    if (index >= 0 && index < colorTable.size()) {
        return colorTable[index];
    }

    // �������������ɫ
    return QColor::fromHsv(
        QRandomGenerator::global()->bounded(360),  // ɫ�� (0-359)
        150 + QRandomGenerator::global()->bounded(106),  // ���Ͷ� (150-255)
        150 + QRandomGenerator::global()->bounded(106)   // ���� (150-255)
    );
}

void CurvePlotWindow::updateAxisComboBoxes()//��������ϵ��x���y����
{
    // �����źţ������δ���
    m_cmbXAxis->blockSignals(true);
    m_cmbYAxis->blockSignals(true);

    // ���������
    m_cmbXAxis->clear();
    m_cmbYAxis->clear();

    if (m_loadedFiles.isEmpty()) {
        // ���û�м����ļ���ʹ��Ĭ��ѡ��
        m_cmbXAxis->addItem("X");
        m_cmbYAxis->addItem("Y");

        m_currentXIndex = 0;
        m_currentYIndex = 1;
    }
    else {
        // ʹ�õ�һ���ļ��ı����б�
        
        auto variables = CurvePlot[0]->Variables;
        for (int i = 0; i < variables.size(); ++i) {
            QString varName =  QString::fromStdString(variables[i]);
            if (varName.isEmpty()) {
                std::cout << "������Ϊ��\n";
                varName = QString("����%1").arg(i + 1);
            }
            m_cmbXAxis->addItem(varName);
            m_cmbYAxis->addItem(varName);
        }

        // ȷ����ǰ��������Ч��Χ��
        if (m_currentXIndex >= m_cmbXAxis->count()) {
            m_currentXIndex = 0;
        }
        if (m_currentYIndex >= m_cmbYAxis->count()) {
            m_currentYIndex = qMin(1, m_cmbYAxis->count() - 1);
        }

        // ���õ�ǰѡ��
        m_cmbXAxis->setCurrentIndex(m_currentXIndex);
        m_cmbYAxis->setCurrentIndex(m_currentYIndex);
    }

    // �ָ��ź�
    m_cmbXAxis->blockSignals(false);
    m_cmbYAxis->blockSignals(false);
}

bool CurvePlotWindow::readFile(const QString& filePath)//�������ļ�·��Ȼ����
{
    QFileInfo fileInfo(filePath);

    // ����ļ��Ƿ����
    if (!fileInfo.exists()) {
        showStatus(QString("�ļ�������: %1").arg(filePath), true);
        return false;
    }

    // ����ļ���С
    if (fileInfo.size() == 0) {
        showStatus(QString("�ļ�Ϊ��: %1").arg(filePath), true);
        return false;
    }

    // ʹ��Reader��ȡ�ļ�
    QByteArray filePathBytes = filePath.toLocal8Bit();
    if (!m_reader->ReadFile(filePathBytes.constData())) {
        showStatus(QString("�޷���ȡ�ļ�: %1").arg(filePath), true);
        return false;
    }

    // �������ά��
    size_t dimension = m_reader->GetDimension();
    if (dimension < 2) {
        showStatus(QString("�ļ�ά��С��2: %1").arg(filePath), true);
        return false;
    }

    // ������ݵ�����
    size_t pointCount = m_reader->GetPointCount();
    if (pointCount == 0) {
        showStatus(QString("�ļ���û����Ч���ݵ�: %1").arg(filePath), true);
        return false;
    }

    return true;
}
//�����Ӧ��ֻ���մ�����������,Ȼ�󣬴�������Ӧ����ʲô�أ�tecplot������һ�������б��һ��������,��������ֻ�Ǽ򵥵Ķ�ά����

void CurvePlotWindow::addCurveToPlot(int fileindex)//�������
{
    // ��������������Ƿ���Ч
    int xIdx = m_currentXIndex;
    int yIdx = m_currentYIndex;

    
    // ׼��X��Y����
    QVector<double> xData, yData;
    for (size_t i = 0; i < CurvePlot[fileindex]->Points.size(); i++)//������ÿһ��,Ȼ�������
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

    // ����ͼ��
    m_customPlot->addGraph();
    QCPGraph* graph = m_customPlot->graph();
    graph->setData(xData, yData);

    // ������������
    QString graphName = QString::fromStdString(CurvePlot[0]->DataTitle);
    graph->setName(graphName);
    // ����������ʽ
    graph->setPen(QPen(Qt::black, 2));
    graph->setLineStyle(QCPGraph::lsLine);
    graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::black, Qt::white, 6));
}

void CurvePlotWindow::updatePlot()
{
    m_customPlot->clearGraphs();

    // ��������ļ�������
    for (int i = 0; i < CurvePlot.size(); ++i) {
        addCurveToPlot(i);
    }

    // �Զ����������᷶Χ
    m_customPlot->rescaleAxes();

    // ��΢����Χ�Ա�����
    m_customPlot->xAxis->scaleRange(1.1, m_customPlot->xAxis->range().center());
    m_customPlot->yAxis->scaleRange(1.1, m_customPlot->yAxis->range().center());

    // �����������ǩ
    m_customPlot->xAxis->setLabel(QString::fromStdString(CurvePlot[0]->Variables[0]));
    m_customPlot->yAxis->setLabel(QString::fromStdString(CurvePlot[0]->Variables[1]));

    // �ػ�ͼ
    m_customPlot->replot();
    showStatus(QString("�Ѽ��� %1 ���ļ����� %2 �����ݵ�")
        .arg(m_loadedFiles.size()));
}

void CurvePlotWindow::resetPlotRange()
{
    m_customPlot->xAxis->setRange(-10, 10);
    m_customPlot->yAxis->setRange(-10, 10);
    m_customPlot->xAxis->setLabel("X��");
    m_customPlot->yAxis->setLabel("Y��");
    m_customPlot->replot();
}

void CurvePlotWindow::showStatus(const QString& message, bool isError)
{
    m_lblStatus->setText(message);

    // ������Ϣ����������ɫ
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
    showStatus("�������������");
}

// ==================== �ۺ���ʵ�� ====================

void CurvePlotWindow::onLoadSingleFile()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "ѡ�������ļ�",
        QDir::currentPath(),
        "�ı��ļ� (*.txt);;�����ļ� (*.*)"
    );
    qDebug() << filePath << endl;//��������Ҫ��һ���Ǿ���·���������·��
    Reader* t = new Reader();
    if (t->ReadFile(filePath.toStdString().c_str())) {
        CurvePlot.push_back(t);
        updateAxisComboBoxes();
        updatePlot();
        showStatus(QString("�ɹ������ļ�: %1").arg(QFileInfo(filePath).fileName()));
    }
}

void CurvePlotWindow::onLoadMultipleFiles()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        "ѡ���������ļ�",
        QDir::currentPath(),
        "�ı��ļ� (*.txt);;�����ļ� (*.*)"
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
        showStatus(QString("�ɹ����� %1/%2 ���ļ�").arg(successCount).arg(filePaths.size()));
    }
}

void CurvePlotWindow::onClearPlot()
{
    if (m_loadedFiles.isEmpty()) {
        showStatus("û��������Ҫ���");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "ȷ�����",
        "ȷ��Ҫ�����������������",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        clearAllData();
    }
}

void CurvePlotWindow::onSavePlot()
{
    if (m_loadedFiles.isEmpty()) {
        showStatus("û�����ݿ��Ա���", true);
        return;
    }

    QString defaultName = QString("plot_%1.png")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

    QString filePath = QFileDialog::getSaveFileName(
        this,
        "����ͼ��",
        QDir::currentPath() + "/" + defaultName,
        "PNGͼ�� (*.png);;JPEGͼ�� (*.jpg);;PDF�ļ� (*.pdf);;BMPͼ�� (*.bmp)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    // �����ļ���չ��ȷ�������ʽ
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
        // Ĭ�ϱ���ΪPNG
        filePath += ".png";
        success = m_customPlot->savePng(filePath, 1920, 1080, 2.0);
    }

    if (success) {
        showStatus(QString("ͼ���ѱ��浽: %1").arg(QFileInfo(filePath).fileName()));
    }
    else {
        showStatus("����ͼ��ʧ��", true);
    }
}

void CurvePlotWindow::onXAxisChanged(int index)
{
    if (index >= 0) {
        m_currentXIndex = index;
        updatePlot();
        showStatus(QString("X��������Ϊ: %1").arg(m_cmbXAxis->itemText(index)));
    }
}

void CurvePlotWindow::onYAxisChanged(int index)
{
    if (index >= 0) {
        m_currentYIndex = index;
        updatePlot();
        showStatus(QString("Y��������Ϊ: %1").arg(m_cmbYAxis->itemText(index)));
    }
}

void CurvePlotWindow::onMouseMove(QMouseEvent* event)
{
    if (m_customPlot->graphCount() > 0) {
        double x = m_customPlot->xAxis->pixelToCoord(event->pos().x());
        double y = m_customPlot->yAxis->pixelToCoord(event->pos().y());
        m_lblCoordinates->setText(QString("����: (%1, %2)").arg(x, 0, 'f', 3).arg(y, 0, 'f', 3));
    }
    else {
        m_lblCoordinates->setText("����: (0.000, 0.000)");
    }
}