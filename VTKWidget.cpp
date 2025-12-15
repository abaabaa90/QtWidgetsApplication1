#include "VTKWidget.h"

// VTK�����ļ�
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
	// ����ͨ��OpenGL��Ⱦ���ڣ��ؼ��޸���
	m_renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
	setRenderWindow(m_renderWindow);

	// ������Ⱦ��
	m_renderer = vtkSmartPointer<vtkRenderer>::New();
	m_renderer->SetBackground(0.1, 0.2, 0.3);
	m_renderer->SetBackground2(0.3, 0.4, 0.5);  // ���䱳��
	m_renderer->SetGradientBackground(true);
	m_renderWindow->AddRenderer(m_renderer);

	// ��������ƽ��
	createGroundPlane();

	// �������
	setupCamera();

	// ���ý������������������������ã�
	setupInteractor();

	// ���������ᣨ���ڽ������Ѿ����ڣ�
	setupAxes();

	// ������ͼ
	resetView();

	m_isInitialized = true;
}

void VTKWidget::createGroundPlane()// ��������ƽ�溯����������3D���������һ���ο�ƽ��
{
	m_planeSource = vtkSmartPointer<vtkPlaneSource>::New();// vtkPlaneSource��������һ������ƽ��
	m_planeSource->SetCenter(0.0, 0.0, 0.0);   // ����ƽ������ĵ����� (x, y, z) = (0, 0, 0)

	// ����ƽ��ķ��߷�����������Ϊz�������򣬼�ˮƽ��
	m_planeSource->SetNormal(0.0, 0.0, 1.0);// ��������(0,0,1)��ʾƽ��ƽ����XYƽ��

	// ����ƽ���ϸ�ֱַ��ʣ�20x20��ʾƽ�汻�ֳ�20��20��С�ı���
	m_planeSource->SetResolution(20, 20);// �ֱ���Խ�ߣ�ƽ��Խƽ���������㿪��Խ��

	// ����ƽ��ӳ����
	double planeSize = 10;// ����ƽ��Ĵ�С������ƽ�潫�Ǵ�-10��10��������

	// ����ƽ���ԭ�����꣨ƽ���һ���ǵ㣩// ԭ��λ��(-10, -10, 0)
	m_planeSource->SetOrigin(-planeSize, -planeSize, 0.0);

	// ����ƽ��ĵ�һ���㣨��ԭ������ĵ�һ���ߵ��յ㣩// Point1λ��(10, -10, 0)��������X�᷽��ı߽�
	m_planeSource->SetPoint1(planeSize, -planeSize, 0.0);

	// ����ƽ��ĵڶ����㣨��ԭ������ĵڶ����ߵ��յ㣩// Point2λ��(-10, 10, 0)��������Y�᷽��ı߽�
	m_planeSource->SetPoint2(-planeSize, planeSize + 10, 0.0);

	// �������������ӳ���������ڽ���������ת��Ϊͼ������
	m_planeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

	// ��ƽ��Դ���������ӵ�ӳ����������˿�
	m_planeMapper->SetInputConnection(m_planeSource->GetOutputPort());// GetOutputPort()��ȡƽ��Դ�����������,������ת����GPU����������ʽ

	// ����ƽ����Ա�������ڳ�������ʾƽ��
	m_planeActor = vtkSmartPointer<vtkActor>::New();//actor����mapper,property,transform�ȵ�����,ÿ��actor���ǳ����е�һ����������,Ӧ����ӵ�render��ȥ��ʾ

	m_planeActor->SetMapper(m_planeMapper);// ��ӳ�������ø���Ա��������Ա��֪�������Ⱦƽ��

	vtkSmartPointer<vtkProperty> planeProperty = vtkSmartPointer<vtkProperty>::New();    // ����ƽ�����ԣ�����ƽ������

	planeProperty->SetColor(0.8, 0.8, 0.8);    // ����ƽ����ɫΪǳ��ɫ (R,G,B) = (0.8, 0.8, 0.8)

	planeProperty->SetOpacity(0.7);    // ����ƽ��͸����Ϊ0.7��1.0Ϊ��ȫ��͸����0.0Ϊ��ȫ͸����

	planeProperty->SetEdgeVisibility(true);    // ���ñ�Ե��ʾ��ʹƽ������ı߽�ɼ�

	planeProperty->SetEdgeColor(0.4, 0.4, 0.4);    // ���ñ�Ե��ɫΪ���ɫ (R,G,B) = (0.4, 0.4, 0.4)

	// ���ñ�Ե�߿�Ϊ1.5����λ
	planeProperty->SetLineWidth(1.5);

	// �����õ�����Ӧ�õ�ƽ����Ա
	m_planeActor->SetProperty(planeProperty);

	// ��ƽ����Ա��ӵ���Ⱦ���У�ʹ���ڳ����пɼ�
	m_renderer->AddActor(m_planeActor);

	// ========== ���ԭ���ǣ���ɫС�� ==========

	// ��������Դ���������ɱ�ʾԭ������弸��
	vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();

	sphereSource->SetCenter(0.0, 0.0, 0.0); // ����������������������ϵԭ�� (0, 0, 0)

	sphereSource->SetRadius(0.05);          // ��������뾶Ϊ0.05����λ

	sphereSource->SetPhiResolution(16);     // ��������ľ��ȷ���ϸ����������ֱ����

	sphereSource->SetThetaResolution(16);   // ���������γ�ȷ���ϸ��������ˮƽ����

	vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();    // ���������ӳ����

	// ������Դ���������ӵ�ӳ����
	sphereMapper->SetInputConnection(sphereSource->GetOutputPort());

	// ����������Ա
	vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();

	// ��ӳ�������ø�������Ա
	sphereActor->SetMapper(sphereMapper);

	// ����������ɫΪ����ɫ (R,G,B) = (1.0, 0.0, 0.0)
	sphereActor->GetProperty()->SetColor(1.0, 0.0, 0.0);

	// ��������Ա��ԭ���ǣ���ӵ���Ⱦ��
	m_renderer->AddActor(sphereActor);

	// ========== ������λ�ñ�ǣ���ɫС�� ==========

	// ������һ������Դ�����ڱ�ʾ���λ��
	vtkSmartPointer<vtkSphereSource> cameraSphereSource = vtkSmartPointer<vtkSphereSource>::New();

	// ������������������(1.0, 1.0, 1.0)����ͨ����ʾ������ܵ�λ��
	cameraSphereSource->SetCenter(1.0, 1.0, 1.0);

	// ��������뾶Ϊ0.05����λ����ԭ��������ͬ��С
	cameraSphereSource->SetRadius(0.05);

	// ��������ϸ�ֲ���
	cameraSphereSource->SetPhiResolution(16);
	cameraSphereSource->SetThetaResolution(16);

	// �������λ�������ӳ����
	vtkSmartPointer<vtkPolyDataMapper> cameraSphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

	// ��������Դ���ݵ�ӳ����
	cameraSphereMapper->SetInputConnection(cameraSphereSource->GetOutputPort());

	// �������λ��������Ա
	vtkSmartPointer<vtkActor> cameraSphereActor = vtkSmartPointer<vtkActor>::New();

	// ��ӳ�������ø���Ա
	cameraSphereActor->SetMapper(cameraSphereMapper);

	// ����������ɫΪ����ɫ (R,G,B) = (0.0, 1.0, 0.0)
	cameraSphereActor->GetProperty()->SetColor(0.0, 1.0, 0.0);

	// �����λ��������Ա��ӵ���Ⱦ��
	m_renderer->AddActor(cameraSphereActor);
}

void VTKWidget::setupCamera()
{
	vtkCamera* camera = m_renderer->GetActiveCamera();
	camera->SetPosition(1.0, 1.0, 1.0);
	camera->SetFocalPoint(0.0, 0.0, 0.0);
	camera->SetViewUp(0.0, 0.0, 1.0);
	camera->SetViewAngle(30.0);

	// ���һЩ�ƹ�Ч��
	m_renderer->SetUseShadows(false);
	m_renderer->SetAmbient(0.3, 0.3, 0.3);
}

void VTKWidget::setupInteractor()
{
	// ȷ����Ⱦ����������
	if (!m_renderWindow)
		return;

	// ��ȡ������
	m_interactor = m_renderWindow->GetInteractor();

	if (!m_interactor)
	{
		qDebug() << "Error: Failed to get interactor";
		return;
	}

	// ���ý�����ʽ
	m_interactorStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	m_interactor->SetInteractorStyle(m_interactorStyle);

	// ���ý������ؼ������������ý������������ò�����
	m_interactor->Initialize();
}

void VTKWidget::setupAxes()
{
	// ȷ���������Ѵ���
	if (!m_interactor)
	{
		qDebug() << "Warning: Interactor not available for axes setup";
		return;
	}

	// ����������
	m_axesActor = vtkSmartPointer<vtkAxesActor>::New();
	m_axesActor->SetShaftTypeToLine();
	m_axesActor->SetNormalizedShaftLength(0.8, 0.8, 0.8);
	m_axesActor->SetNormalizedTipLength(0.2, 0.2, 0.2);
	m_axesActor->AxisLabelsOff();  // ��ѡ���رձ�ǩ��ʾ

	// ������������ɫ�ʹ�ϸ
	m_axesActor->GetXAxisShaftProperty()->SetColor(1.0, 0.0, 0.0);  // ��ɫX��
	m_axesActor->GetYAxisShaftProperty()->SetColor(0.0, 1.0, 0.0);  // ��ɫY��
	m_axesActor->GetZAxisShaftProperty()->SetColor(0.0, 0.0, 1.0);  // ��ɫZ��
	m_axesActor->GetXAxisShaftProperty()->SetLineWidth(2.0);
	m_axesActor->GetYAxisShaftProperty()->SetLineWidth(2.0);
	m_axesActor->GetZAxisShaftProperty()->SetLineWidth(2.0);

	// ���������ǲ���
	m_axesWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
	m_axesWidget->SetOrientationMarker(m_axesActor);

	// �ؼ��޸������������ý������������ò���
	m_axesWidget->SetInteractor(m_interactor);
	m_axesWidget->SetEnabled(1);
	m_axesWidget->SetInteractive(0);  // ����Ϊ�ǽ���ʽ
	m_axesWidget->SetViewport(0.0, 0.0, 0.2, 0.2);

	// ��Ⱦ
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