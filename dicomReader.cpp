#include "pch.h"
// 创建回调函数处理键盘事件
class KeyPressCallback : public vtkCommand
{
public:
    static KeyPressCallback* New() { return new KeyPressCallback; }

    void SetRenderers(std::vector<vtkRenderer*> renderers) { this->renderers = renderers; }

    virtual void Execute(vtkObject* caller, unsigned long eventId, void* callData)
    {
        vtkRenderWindowInteractor* iren = static_cast<vtkRenderWindowInteractor*>(caller);
        std::string key = iren->GetKeySym();

        vtkRenderer* currentRenderer = iren->GetRenderWindow()->GetInteractor()->FindPokedRenderer(
            iren->GetEventPosition()[0], iren->GetEventPosition()[1]);

        if (key == "r" || key == "R")
        {
            if (currentRenderer)
            {
                currentRenderer->ResetCamera();
                std::cout << "重置当前视图的相机" << std::endl;
            }
        }
        else if (key == "a" || key == "A")
        {
            // 重置所有视图
            for (auto renderer : renderers)
            {
                if (renderer) renderer->ResetCamera();
            }
            std::cout << "重置所有视图的相机" << std::endl;
        }
        else if (key == "1")
        {
            // 切换到轴向视图设置
            std::cout << "轴向视图 (XY) - 切片: " << mapperXY->GetSliceNumber() << std::endl;
        }
        else if (key == "2")
        {
            // 切换到矢状视图设置
            std::cout << "矢状视图 (YZ) - 切片: " << mapperYZ->GetSliceNumber() << std::endl;
        }
        else if (key == "3")
        {
            // 切换到冠状视图设置
            std::cout << "冠状视图 (XZ) - 切片: " << mapperXZ->GetSliceNumber() << std::endl;
        }

        iren->GetRenderWindow()->Render();
    }

    vtkImageSliceMapper* mapperXY;
    vtkImageSliceMapper* mapperYZ;
    vtkImageSliceMapper* mapperXZ;

private:
    std::vector<vtkRenderer*> renderers;
};
int test() {

    vtkNew<vtkDICOMImageReader> reader;
    reader->SetDirectoryName("testdcm");
    reader->Update();

    vtkImageData* imageData = reader->GetOutput();
    if (!imageData)
    {
        std::cerr << "错误: 无法读取DICOM数据" << std::endl;
        return -1;
    }

    int dims[3];
    imageData->GetDimensions(dims);

    std::cout << "DICOM数据信息:" << std::endl;
    std::cout << "  图像尺寸: " << dims[0] << " × " << dims[1] << " × " << dims[2] << std::endl;
    std::cout << "  切片总数: " << dims[2] << std::endl;
    std::cout << "  尝试不同切片位置..." << std::endl;

    // 创建渲染窗口和交互器
    vtkNew<vtkRenderWindow> renderWindow;
    renderWindow->SetSize(1200, 800);
    renderWindow->SetWindowName("DICOM 多平面查看器");

    vtkNew<vtkRenderWindowInteractor> interactor;
    interactor->SetRenderWindow(renderWindow);

    // 创建渲染器列表
    std::vector<vtkRenderer*> renderers;

    // 轴向视图 (XY) - 左上角
    vtkNew<vtkImageSliceMapper> mapperXY;
    mapperXY->SetInputConnection(reader->GetOutputPort());
    mapperXY->SetSliceNumber(dims[2] / 4);  // 尝试1/4位置
    mapperXY->SetOrientationToZ();

    vtkNew<vtkImageSlice> sliceXY;
    sliceXY->SetMapper(mapperXY);

    vtkNew<vtkRenderer> rendererXY;
    rendererXY->AddViewProp(sliceXY);
    rendererXY->SetViewport(0.0, 0.5, 0.5, 1.0);
    rendererXY->SetBackground(0.2, 0.2, 0.3);
    renderWindow->AddRenderer(rendererXY);
    renderers.push_back(rendererXY);

    // 矢状视图 (YZ) - 右上角
    vtkNew<vtkImageSliceMapper> mapperYZ;
    mapperYZ->SetInputConnection(reader->GetOutputPort());
    mapperYZ->SetSliceNumber(dims[0] / 4);  // 尝试1/4位置
    mapperYZ->SetOrientationToX();

    vtkNew<vtkImageSlice> sliceYZ;
    sliceYZ->SetMapper(mapperYZ);

    vtkNew<vtkRenderer> rendererYZ;
    rendererYZ->AddViewProp(sliceYZ);
    rendererYZ->SetViewport(0.5, 0.5, 1.0, 1.0);
    rendererYZ->SetBackground(0.2, 0.3, 0.2);
    renderWindow->AddRenderer(rendererYZ);
    renderers.push_back(rendererYZ);

    // 冠状视图 (XZ) - 左下角
    vtkNew<vtkImageSliceMapper> mapperXZ;
    mapperXZ->SetInputConnection(reader->GetOutputPort());
    mapperXZ->SetSliceNumber(dims[1] / 4);  // 尝试1/4位置
    mapperXZ->SetOrientationToY();

    vtkNew<vtkImageSlice> sliceXZ;
    sliceXZ->SetMapper(mapperXZ);

    vtkNew<vtkRenderer> rendererXZ;
    rendererXZ->AddViewProp(sliceXZ);
    rendererXZ->SetViewport(0.0, 0.0, 0.5, 0.5);
    rendererXZ->SetBackground(0.3, 0.2, 0.2);
    renderWindow->AddRenderer(rendererXZ);
    renderers.push_back(rendererXZ);

    // 信息面板 - 右下角
    vtkNew<vtkRenderer> rendererInfo;
    rendererInfo->SetViewport(0.5, 0.0, 1.0, 0.5);
    rendererInfo->SetBackground(0.1, 0.1, 0.1);
    renderWindow->AddRenderer(rendererInfo);
    renderers.push_back(rendererInfo);

    // 设置交互样式
    vtkNew<vtkInteractorStyleImage> style;
    interactor->SetInteractorStyle(style);

    // 添加键盘回调
    vtkNew<KeyPressCallback> keyPressCallback;
    keyPressCallback->mapperXY = mapperXY;
    keyPressCallback->mapperYZ = mapperYZ;
    keyPressCallback->mapperXZ = mapperXZ;
    keyPressCallback->SetRenderers(renderers);
    interactor->AddObserver(vtkCommand::KeyPressEvent, keyPressCallback);

    // 打印使用说明
    std::cout << "\n使用说明:" << std::endl;
    std::cout << "  1. 点击激活任意一个视图" << std::endl;
    std::cout << "  2. 鼠标滚轮: 在激活的视图中切换切片" << std::endl;
    std::cout << "  3. 鼠标左键拖动: 平移图像" << std::endl;
    std::cout << "  4. 鼠标右键拖动: 缩放图像" << std::endl;
    std::cout << "  5. 按 'r' 键: 重置当前视图" << std::endl;
    std::cout << "  6. 按 'a' 键: 重置所有视图" << std::endl;
    std::cout << "  7. 按 '1', '2', '3' 键: 查看当前切片位置" << std::endl;

    // 重置所有相机
    for (auto renderer : renderers)
    {
        renderer->ResetCamera();
    }

    // 渲染
    renderWindow->Render();
    interactor->Start();
    return 0;
}