//#include <QApplication>
//#include <QMainWindow>
//#include <QVBoxLayout>
//#include <QWidget>
//#include <QMessageBox>
//#include <iostream>
//#include <QPushButton>
//#include "CustomLogTicker.h"  // 或 "FixedIntervalFromMinTicker.h"
//#include "GenerateRandom.cpp"
//void smaple() {
//    // 创建随机数生成器
//    RandomGenerator rg;
//
//    // 生成10个整数，范围0-100
//    auto ints = rg.generateIntegers(10, 0, 100);
//    std::cout << "Integers (0-100): ";
//    for (int n : ints) std::cout << n << " ";
//    std::cout << std::endl;
//
//    // 生成5个双精度浮点数，范围0.0-1.0
//    auto doubles = rg.generateFloats<double>(5);
//    std::cout << "Doubles (0.0-1.0): ";
//    for (double d : doubles) std::cout << std::fixed << std::setprecision(3) << d << " ";
//    std::cout << std::endl;
//
//    // 生成3个字符串，长度8
//    auto strings = rg.generateStrings(3, 8);
//    std::cout << "Strings (length 8): ";
//    for (const auto& s : strings) std::cout << s << " ";
//    std::cout << std::endl;
//
//    // 使用通用接口生成不同类型的随机数
//    auto ints2 = rg.generate<int>(5);
//    auto floats = rg.generate<float>(5);
//
//    // 生成不重复的随机数
//    auto unique_ints = rg.generateUnique(10, 1, 20);
//    std::cout << "Unique integers (1-20): ";
//    for (int n : unique_ints) std::cout << n << " ";
//    std::cout << std::endl;
//
//    // 生成正态分布的随机数
//    auto normal_doubles = rg.generateNormal(5, 0.0, 1.0);
//    std::cout << "Normal distribution (mean=0, stddev=1): ";
//    for (double d : normal_doubles) std::cout << std::fixed << std::setprecision(3) << d << " ";
//    std::cout << std::endl;
//
//    // 生成随机字节
//    auto bytes = rg.generateBytes(10);
//    std::cout << "Random bytes: ";
//    for (uint8_t b : bytes) std::cout << std::hex << static_cast<int>(b) << " ";
//    std::cout << std::dec << std::endl;
//
//
//}
//int main(int argc, char* argv[])
//{
//    QApplication a(argc, argv);
//
//    try {
//        QMainWindow mainWindow;
//        mainWindow.setWindowTitle("强制固定间隔对数刻度");
//        mainWindow.resize(1200, 800);
//
//        QWidget* centralWidget = new QWidget(&mainWindow);
//        QVBoxLayout* layout = new QVBoxLayout(centralWidget);
//
//        QCustomPlot* customPlot = new QCustomPlot();
//        layout->addWidget(customPlot);
//
//        // 添加控制按钮
//        QHBoxLayout* buttonLayout = new QHBoxLayout();
//        QPushButton* btnMode1 = new QPushButton("模式1: 从10^0开始");
//        QPushButton* btnMode2 = new QPushButton("模式2: 从最小值开始");
//        buttonLayout->addWidget(btnMode1);
//        buttonLayout->addWidget(btnMode2);
//        layout->addLayout(buttonLayout);
//
//        // 设置数据
//        RandomGenerator rg;
//
//        // 生成10个整数，范围0-100
//        std::vector<double> ints = rg.generateFloats(10, 0.0, 10000000000.0);
//        //QVector<double> xData = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
//        QVector<double> xData = QVector<double>::fromStdVector(ints);
//        //QVector<double> yData = { 1e-3, 1e-1, 1e1, 1e3, 1e5, 1e7, 1e9, 1e11, 1e13 };
//        QVector<double> yData = { 1e-3, 1e-1, 1e1, 1e3, 1e5, 1e7, 1e9, 1e11, 1e13 };
//
//        QCPGraph* graph = customPlot->addGraph();
//        graph->setData(xData, yData);
//        graph->setScatterStyle(QCPScatterStyle::ssCircle);
//        graph->setLineStyle(QCPGraph::lsLine);
//
//        customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
//
//        // 模式1：绝对固定的刻度（从10^0=1开始）
//        auto setupMode1 = [=]() {
//            QSharedPointer<FixedPowerTicker> yTicker(new FixedPowerTicker);
//            yTicker->setStartExponent(0);      // 从10^0=1开始
//            yTicker->setExponentStep(2);       // 间隔为10^2
//            yTicker->setMaxTickCount(12);       // 最多8个刻度
//
//            customPlot->yAxis->setTicker(yTicker);
//            customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
//
//            // 使用自定义标签格式显示为10^n
//            //yTicker->setTickLabelType(QCPAxisTicker::ltDateTime);
//
//            // 设置Y轴范围
//            customPlot->yAxis->setRange(1e-4, 1e14);
//
//            customPlot->replot();
//            };
//
//         //模式2：从最小值开始的固定间隔
//        auto setupMode2 = [=]() {
//            QSharedPointer<FixedIntervalFromMinTicker> yTicker(new FixedIntervalFromMinTicker);
//            yTicker->setExponentStep(2);       // 间隔为10^2
//            yTicker->setForceAllTicks(true);   // 强制所有刻度都显示
//
//            customPlot->yAxis->setTicker(yTicker);
//            customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
//
//            // 设置标签格式为科学计数法
//            customPlot->yAxis->setNumberFormat("eb");
//            customPlot->yAxis->setNumberPrecision(0);
//
//            // 设置Y轴范围
//            customPlot->yAxis->setRange(1e-4, 1e14);
//
//            customPlot->replot();
//            };
//
//        // 连接按钮信号
//        QObject::connect(btnMode1, &QPushButton::clicked, setupMode1);
//        QObject::connect(btnMode2, &QPushButton::clicked, setupMode2);
//
//        // 初始设置为模式1
//        setupMode1();
//
//        // 设置轴标签
//        customPlot->xAxis->setLabel("数据点");
//        customPlot->yAxis->setLabel("Y值 (强制10^2间隔对数刻度)");
//
//        // 设置X轴范围
//        customPlot->xAxis->setRange(0, 10);
//
//        mainWindow.setCentralWidget(centralWidget);
//        mainWindow.show();
//
//        return a.exec();
//    }
//    catch (const std::exception& e) {
//        QMessageBox::critical(nullptr, "错误", QString("程序异常: %1").arg(e.what()));
//        return -1;
//    }
//    catch (...) {
//        QMessageBox::critical(nullptr, "错误", "未知异常");
//        return -1;
//    }
//}
