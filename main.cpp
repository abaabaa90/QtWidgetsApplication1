#pragma once
#include "CylinderExample.h"
#include "BaseWindow.h"
#include "MainWindow.h"
#include "pch.h"
#include <vtkAxesActor.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextActor.h>
#include "DimensionPlane.h"
#include <vtkAutoInit.h>
#include <vtkTecplotReader.h>
#include "Reader.h"
#include "CurvePlotWindow.h"
#include "thread_pool.h"
#include "ThreadPool.h"
#include "qthreadpool.h"
#include "ReaderHighSpeedDemo.cpp"
#include <fstream>
#pragma execution_character_set("utf-8")    
//VTK_MODULE_INIT(vtkRenderingOpenGL2);// VTK模块初始化
//VTK_MODULE_INIT(vtkInteractionStyle);
//VTK_MODULE_INIT(vtkRenderingFreeType);
//VTK_MODULE_INIT(vtkRenderingContextOpenGL2);

namespace fs = std::filesystem;

// 高性能文件读取器
class HighPerformanceFileReader;
class Timer {
public:
    Timer(const std::string& name) : name(name) {
        start = std::chrono::high_resolution_clock::now();
    }

    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << name << " 耗时: " << duration.count() << " μs" << std::endl;
    }

private:
    std::string name;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

int main(int argc, char* argv[])
{
	QApplication app(argc, argv); 
	{
	// 设置OpenGL表面格式（确保兼容性）
	//QSurfaceFormat format;
	//format.setRenderableType(QSurfaceFormat::OpenGL);
	//format.setProfile(QSurfaceFormat::CoreProfile);
	//format.setVersion(3, 2);
	//format.setSamples(8);
	//QSurfaceFormat::setDefaultFormat(format);
	//DimensionPlane w;
	//w.show();
	}
	//尝试qcustomplot进行绘图	
    {
        /*CurvePlotWindow a;

       a.show();
       Reader reader1;
       if (reader1.ReadFile("file1.txt"))
       {
           std::cout << "Title: " << reader1.DataTitle << std::endl;std::cout << "Variables: ";
           for (const auto& var : reader1.Variables)
           {
               std::cout << var << " ";
           }
           std::cout << std::endl;

           std::cout << "Points (" << reader1.GetPointCount() << "):" << std::endl;
           std::cout << std::fixed << std::setprecision(2);

           size_t count = 0;
           for (const auto& point : reader1.Points)
           {
               std::cout << "Point " << ++count << ": ";
               for (float coord : point)
               {
                   std::cout << coord << " ";
               }
               std::cout << std::endl;
           }
       }
       else
       {
           std::cout << "Failed to read file1.txt" << std::endl;
       }*/}
    auto start = std::chrono::high_resolution_clock::now();
    std::ifstream file("file2.txt");
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "读取时间: " << elapsed.count() << " 秒\n";
    try {        // 根据CPU核心数创建线程池（通常设置为核心数或核心数*2）
        const size_t threadCount = std::thread::hardware_concurrency();
        ThreadPool threadPool(threadCount);
        threadPool.init();

        HighPerformanceFileReader reader(threadPool, 16 * 1024 * 1024);  // 16MB块大小

        const std::string filePath = "file2.txt";  // 替换为你的大文件路径
        // 计时
        auto start = std::chrono::high_resolution_clock::now();
        // 读取文件
        auto data = reader.readFile(filePath);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        // 输出结果
        std::cout << "文件大小: " << data.size() / (1024 * 1024) << " MB\n";
        std::cout << "读取时间: " << elapsed.count() << " 秒\n";
        std::cout << "读取速度: " << (data.size() / (1024 * 1024)) / elapsed.count() << " MB/s\n";
        threadPool.shutdown();
    }
    catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }





	return app.exec();
}
