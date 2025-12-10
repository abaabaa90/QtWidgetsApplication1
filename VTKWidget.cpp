#include "VTKWidget.h"

// VTK包含文件
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCamera.h>
#include <vtkSphereSource.h>
#include <vtkPolyData.h>
#include <qDebug>

VTKWidget::VTKWidget(QWidget* parent)
    : QVTKOpenGLNativeWidget(parent)
    , m_isInitialized(false)
{
    initializeVTK();
}

VTKWidget::~VTKWidget()
{
}

void VTKWidget::initializeVTK()
{
    // 创建通用OpenGL渲染窗口（关键修复）
    m_renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    setRenderWindow(m_renderWindow);

    // 创建渲染器
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderer->SetBackground(0.1, 0.2, 0.3);
    m_renderer->SetBackground2(0.3, 0.4, 0.5);  // 渐变背景
    m_renderer->SetGradientBackground(true);
    m_renderWindow->AddRenderer(m_renderer);

    // 创建地面平面
    createGroundPlane();

    // 设置相机
    setupCamera();

    // 设置交互器（必须先于坐标轴设置）
    setupInteractor();

    // 设置坐标轴（现在交互器已经存在）
    setupAxes();

    // 重置视图
    resetView();

    m_isInitialized = true;
}

void VTKWidget::createGroundPlane()
{
    // 创建平面源
    m_planeSource = vtkSmartPointer<vtkPlaneSource>::New();
    m_planeSource->SetCenter(0.0, 0.0, 0.0);
    m_planeSource->SetNormal(0.0, 0.0, 1.0);
    m_planeSource->SetResolution(20, 20);

    // 创建平面映射器
    double planeSize = 1000;
    m_planeSource->SetOrigin(-planeSize, -planeSize, 0.0);
    m_planeSource->SetPoint1(planeSize, -planeSize, 0.0);
    m_planeSource->SetPoint2(-planeSize, planeSize, 0.0);
    m_planeSource->SetResolution(20, 20);

    m_planeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_planeMapper->SetInputConnection(m_planeSource->GetOutputPort());

    // 创建平面演员
    m_planeActor = vtkSmartPointer<vtkActor>::New();
    m_planeActor->SetMapper(m_planeMapper);

    // 设置平面属性
    vtkSmartPointer<vtkProperty> planeProperty = vtkSmartPointer<vtkProperty>::New();
    planeProperty->SetColor(0.8, 0.8, 0.8);
    planeProperty->SetOpacity(0.7);
    planeProperty->SetEdgeVisibility(true);
    planeProperty->SetEdgeColor(0.4, 0.4, 0.4);
    planeProperty->SetLineWidth(1.5);

    m_planeActor->SetProperty(planeProperty);
    m_renderer->AddActor(m_planeActor);

    // 添加原点标记（红色小球）
    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetCenter(0.0, 0.0, 0.0);
    sphereSource->SetRadius(0.05);
    sphereSource->SetPhiResolution(16);
    sphereSource->SetThetaResolution(16);

    vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    sphereMapper->SetInputConnection(sphereSource->GetOutputPort());

    vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();
    sphereActor->SetMapper(sphereMapper);
    sphereActor->GetProperty()->SetColor(1.0, 0.0, 0.0);

    m_renderer->AddActor(sphereActor);

    // 添加相机位置标记（绿色小球）
    vtkSmartPointer<vtkSphereSource> cameraSphereSource = vtkSmartPointer<vtkSphereSource>::New();
    cameraSphereSource->SetCenter(1.0, 1.0, 1.0);
    cameraSphereSource->SetRadius(0.05);
    cameraSphereSource->SetPhiResolution(16);
    cameraSphereSource->SetThetaResolution(16);

    vtkSmartPointer<vtkPolyDataMapper> cameraSphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    cameraSphereMapper->SetInputConnection(cameraSphereSource->GetOutputPort());

    vtkSmartPointer<vtkActor> cameraSphereActor = vtkSmartPointer<vtkActor>::New();
    cameraSphereActor->SetMapper(cameraSphereMapper);
    cameraSphereActor->GetProperty()->SetColor(0.0, 1.0, 0.0);

    m_renderer->AddActor(cameraSphereActor);
}


void VTKWidget::setupCamera()
{
    vtkCamera* camera = m_renderer->GetActiveCamera();
    camera->SetPosition(1.0, 1.0, 1.0);
    camera->SetFocalPoint(0.0, 0.0, 0.0);
    camera->SetViewUp(0.0, 0.0, 1.0);
    camera->SetViewAngle(30.0);

    // 添加一些灯光效果
    m_renderer->SetUseShadows(false);
    m_renderer->SetAmbient(0.3, 0.3, 0.3);
}

void VTKWidget::setupInteractor()
{
    // 确保渲染窗口已设置
    if (!m_renderWindow)
        return;

    // 获取交互器
    m_interactor = m_renderWindow->GetInteractor();

    if (!m_interactor)
    {
        qDebug() << "Error: Failed to get interactor";
        return;
    }

    // 设置交互样式
    m_interactorStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    m_interactor->SetInteractorStyle(m_interactorStyle);

    // 启用交互（关键：必须先设置交互器，再启用部件）
    m_interactor->Initialize();
}

void VTKWidget::setupAxes()
{
    // 确保交互器已存在
    if (!m_interactor)
    {
        qDebug() << "Warning: Interactor not available for axes setup";
        return;
    }

    // 创建坐标轴
    m_axesActor = vtkSmartPointer<vtkAxesActor>::New();
    m_axesActor->SetShaftTypeToLine();
    m_axesActor->SetNormalizedShaftLength(0.8, 0.8, 0.8);
    m_axesActor->SetNormalizedTipLength(0.2, 0.2, 0.2);
    m_axesActor->AxisLabelsOff();  // 可选：关闭标签显示

    // 设置坐标轴颜色和粗细
    m_axesActor->GetXAxisShaftProperty()->SetColor(1.0, 0.0, 0.0);  // 红色X轴
    m_axesActor->GetYAxisShaftProperty()->SetColor(0.0, 1.0, 0.0);  // 绿色Y轴
    m_axesActor->GetZAxisShaftProperty()->SetColor(0.0, 0.0, 1.0);  // 蓝色Z轴
    m_axesActor->GetXAxisShaftProperty()->SetLineWidth(2.0);
    m_axesActor->GetYAxisShaftProperty()->SetLineWidth(2.0);
    m_axesActor->GetZAxisShaftProperty()->SetLineWidth(2.0);

    // 创建方向标记部件
    m_axesWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    m_axesWidget->SetOrientationMarker(m_axesActor);

    // 关键修复：必须先设置交互器，再启用部件
    m_axesWidget->SetInteractor(m_interactor);
    m_axesWidget->SetEnabled(1);
    m_axesWidget->SetInteractive(0);  // 设置为非交互式
    m_axesWidget->SetViewport(0.0, 0.0, 0.2, 0.2);

    // 渲染
    m_renderWindow->Render();
}

void VTKWidget::resetView()
{
    if (!m_isInitialized)
        return;

    setCameraToDefault();
    m_renderer->ResetCamera();
    m_renderWindow->Render();
}

void VTKWidget::setCameraToDefault()
{
    if (!m_isInitialized)
        return;

    vtkCamera* camera = m_renderer->GetActiveCamera();
    camera->SetPosition(1.0, 1.0, 1.0);
    camera->SetFocalPoint(0.0, 0.0, 0.0);
    camera->SetViewUp(0.0, 0.0, 1.0);
    m_renderWindow->Render();
}