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

void VTKWidget::createGroundPlane()// 创建地面平面函数，用于在3D场景中添加一个参考平面
{
	m_planeSource = vtkSmartPointer<vtkPlaneSource>::New();// vtkPlaneSource用于生成一个矩形平面
	m_planeSource->SetCenter(0.0, 0.0, 0.0);   // 设置平面的中心点坐标 (x, y, z) = (0, 0, 0)

	// 设置平面的法线方向，这里设置为z轴正方向，即水平面
	m_planeSource->SetNormal(0.0, 0.0, 1.0);// 法线向量(0,0,1)表示平面平行于XY平面

	// 设置平面的细分分辨率，20x20表示平面被分成20×20个小四边形
	m_planeSource->SetResolution(20, 20);// 分辨率越高，平面越平滑，但计算开销越大

	// 创建平面映射器
	double planeSize = 10;// 定义平面的大小，这里平面将是从-10到10的正方形

	// 设置平面的原点坐标（平面的一个角点）// 原点位于(-10, -10, 0)
	m_planeSource->SetOrigin(-planeSize, -planeSize, 0.0);

	// 设置平面的第一个点（从原点出发的第一条边的终点）// Point1位于(10, -10, 0)，定义了X轴方向的边界
	m_planeSource->SetPoint1(planeSize, -planeSize, 0.0);

	// 设置平面的第二个点（从原点出发的第二条边的终点）// Point2位于(-10, 10, 0)，定义了Y轴方向的边界
	m_planeSource->SetPoint2(-planeSize, planeSize + 10, 0.0);

	// 创建多边形数据映射器，用于将几何数据转换为图形数据
	m_planeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

	// 将平面源的数据连接到映射器的输入端口
	m_planeMapper->SetInputConnection(m_planeSource->GetOutputPort());// GetOutputPort()获取平面源的输出数据流,将数据转换成GPU易于理解的形式

	// 创建平面演员，负责在场景中显示平面
	m_planeActor = vtkSmartPointer<vtkActor>::New();//actor包含mapper,property,transform等等属性,每个actor都是场景中的一个独立对象,应该添加到render中去显示

	m_planeActor->SetMapper(m_planeMapper);// 将映射器设置给演员，这样演员就知道如何渲染平面

	vtkSmartPointer<vtkProperty> planeProperty = vtkSmartPointer<vtkProperty>::New();    // 设置平面属性，控制平面的外观

	planeProperty->SetColor(0.8, 0.8, 0.8);    // 设置平面颜色为浅灰色 (R,G,B) = (0.8, 0.8, 0.8)

	planeProperty->SetOpacity(0.7);    // 设置平面透明度为0.7（1.0为完全不透明，0.0为完全透明）

	planeProperty->SetEdgeVisibility(true);    // 启用边缘显示，使平面网格的边界可见

	planeProperty->SetEdgeColor(0.4, 0.4, 0.4);    // 设置边缘颜色为深灰色 (R,G,B) = (0.4, 0.4, 0.4)

	// 设置边缘线宽为1.5个单位
	planeProperty->SetLineWidth(1.5);

	// 将设置的属性应用到平面演员
	m_planeActor->SetProperty(planeProperty);

	// 将平面演员添加到渲染器中，使其在场景中可见
	m_renderer->AddActor(m_planeActor);

	// ========== 添加原点标记（红色小球） ==========

	// 创建球体源，用于生成表示原点的球体几何
	vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();

	sphereSource->SetCenter(0.0, 0.0, 0.0); // 设置球体中心在世界坐标系原点 (0, 0, 0)

	sphereSource->SetRadius(0.05);          // 设置球体半径为0.05个单位

	sphereSource->SetPhiResolution(16);     // 设置球体的经度方向细分数量（垂直方向）

	sphereSource->SetThetaResolution(16);   // 设置球体的纬度方向细分数量（水平方向）

	vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();    // 创建球体的映射器

	// 将球体源的数据连接到映射器
	sphereMapper->SetInputConnection(sphereSource->GetOutputPort());

	// 创建球体演员
	vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();

	// 将映射器设置给球体演员
	sphereActor->SetMapper(sphereMapper);

	// 设置球体颜色为纯红色 (R,G,B) = (1.0, 0.0, 0.0)
	sphereActor->GetProperty()->SetColor(1.0, 0.0, 0.0);

	// 将球体演员（原点标记）添加到渲染器
	m_renderer->AddActor(sphereActor);

	// ========== 添加相机位置标记（绿色小球） ==========

	// 创建另一个球体源，用于表示相机位置
	vtkSmartPointer<vtkSphereSource> cameraSphereSource = vtkSmartPointer<vtkSphereSource>::New();

	// 设置球体中心在坐标(1.0, 1.0, 1.0)，这通常表示相机可能的位置
	cameraSphereSource->SetCenter(1.0, 1.0, 1.0);

	// 设置球体半径为0.05个单位，与原点球体相同大小
	cameraSphereSource->SetRadius(0.05);

	// 设置球体细分参数
	cameraSphereSource->SetPhiResolution(16);
	cameraSphereSource->SetThetaResolution(16);

	// 创建相机位置球体的映射器
	vtkSmartPointer<vtkPolyDataMapper> cameraSphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

	// 连接球体源数据到映射器
	cameraSphereMapper->SetInputConnection(cameraSphereSource->GetOutputPort());

	// 创建相机位置球体演员
	vtkSmartPointer<vtkActor> cameraSphereActor = vtkSmartPointer<vtkActor>::New();

	// 将映射器设置给演员
	cameraSphereActor->SetMapper(cameraSphereMapper);

	// 设置球体颜色为纯绿色 (R,G,B) = (0.0, 1.0, 0.0)
	cameraSphereActor->GetProperty()->SetColor(0.0, 1.0, 0.0);

	// 将相机位置球体演员添加到渲染器
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