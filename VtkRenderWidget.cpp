#include "VtkRenderWidget.h"
#include <QVBoxLayout>

// VTKͷ�ļ�
#include <vtkConeSource.h>
#include <vtkSphereSource.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkLight.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>
#include <vtkTecplotReader.h>




// ����ָ��
#include <vtkSmartPointer.h>

VtkRenderWidget::VtkRenderWidget(QWidget* parent)
    : QWidget(parent)
    , m_initialized(false)
{
    // ��������
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // ����VTK OpenGL����
    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    layout->addWidget(m_vtkWidget);

    // ���ô�С����
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ��ʼ��VTK
    initializeVtk();
}

VtkRenderWidget::~VtkRenderWidget()
{
    // VTK����ָ����Զ������ڴ棬����ֻ��Ҫɾ��Qt����
    delete m_vtkWidget;
}

void VtkRenderWidget::initializeVtk()
{
    if (m_initialized) return;

    // ��ȡ�򴴽���Ⱦ��
    m_renderer = vtkRenderer::New();
    m_renderer->SetBackground(0.1, 0.2, 0.4);  // ����ɫ����

    // ������Ⱦ����
    m_vtkWidget->renderWindow()->AddRenderer(m_renderer);

    // ��ӹ�Դ
    addLights();

    // ����Ĭ�ϳ���
    createDefaultScene();

    // ���ý�������ʽ�������������
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
        vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    m_vtkWidget->interactor()->SetInteractorStyle(style);

    m_initialized = true;

    // ��ʼ��Ⱦ
    m_vtkWidget->renderWindow()->Render();
}

void VtkRenderWidget::addLights()
{
    // �������Դ
    vtkSmartPointer<vtkLight> light1 = vtkSmartPointer<vtkLight>::New();
    light1->SetLightTypeToSceneLight();
    light1->SetPosition(1, 1, 1);
    light1->SetFocalPoint(0, 0, 0);
    light1->SetColor(1, 1, 1);
    light1->SetIntensity(0.8);
    m_renderer->AddLight(light1);

    // ��Ӹ�����Դ
    vtkSmartPointer<vtkLight> light2 = vtkSmartPointer<vtkLight>::New();
    light2->SetLightTypeToSceneLight();
    light2->SetPosition(-1, -1, 1);
    light2->SetFocalPoint(0, 0, 0);
    light2->SetColor(0.8, 0.8, 1.0);
    light2->SetIntensity(0.4);
    m_renderer->AddLight(light2);
}

void VtkRenderWidget::createDefaultScene()
{
    // Ĭ�����һ��Բ׶�塢һ�����塢һ��Բ����
    double red[] = { 1.0, 0.0, 0.0 };
    double green[] = { 0.0, 1.0, 0.0 };
    double blue[] = { 0.0, 0.5, 1.0 };

    double conePos[] = { -3.0, 0.0, 0.0 };
    double spherePos[] = { 0.0, 0.0, 0.0 };
    double cylinderPos[] = { 3.0, 0.0, 0.0 };

    addCone(3.0, 1.0, 20, red, conePos);
    addSphere(1.5, 30, 30, green, spherePos);
    addCylinder(3.0, 1.0, 20, blue, cylinderPos);
}

vtkActor* VtkRenderWidget::createActor(vtkPolyDataMapper* mapper, const double color[3], const double position[3])
{
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // ������ɫ
    if (color) {
        actor->GetProperty()->SetColor(color[0], color[1], color[2]);
    }

    // ����λ��
    if (position) {
        actor->SetPosition(position[0], position[1], position[2]);
    }

    // ���ò�������
    actor->GetProperty()->SetAmbient(0.3);
    actor->GetProperty()->SetDiffuse(0.7);
    actor->GetProperty()->SetSpecular(0.5);
    actor->GetProperty()->SetSpecularPower(50);

    m_renderer->AddActor(actor);
    m_actors.push_back(actor);

    return actor;
}

void VtkRenderWidget::addCone(double height, double radius, int resolution, const double color[3], const double position[3])
{
    vtkSmartPointer<vtkConeSource> cone = vtkSmartPointer<vtkConeSource>::New();
    cone->SetHeight(height);
    cone->SetRadius(radius);
    cone->SetResolution(resolution);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cone->GetOutputPort());

    createActor(mapper, color, position);
    m_vtkWidget->renderWindow()->Render();
}

void VtkRenderWidget::addSphere(double radius, int thetaResolution, int phiResolution, const double color[3], const double position[3])
{
    vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetRadius(radius);
    sphere->SetThetaResolution(thetaResolution);
    sphere->SetPhiResolution(phiResolution);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphere->GetOutputPort());

    createActor(mapper, color, position);
    m_vtkWidget->renderWindow()->Render();
}

void VtkRenderWidget::addCylinder(double height, double radius, int resolution, const double color[3], const double position[3])
{
    vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
    cylinder->SetHeight(height);
    cylinder->SetRadius(radius);
    cylinder->SetResolution(resolution);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cylinder->GetOutputPort());

    createActor(mapper, color, position);
    m_vtkWidget->renderWindow()->Render();
}

void VtkRenderWidget::setBackgroundColor(double r, double g, double b)
{
    m_renderer->SetBackground(r, g, b);
    m_vtkWidget->renderWindow()->Render();
}

void VtkRenderWidget::resetCamera()
{
    m_renderer->ResetCamera();
    m_vtkWidget->renderWindow()->Render();
}

vtkRenderWindow* VtkRenderWidget::getRenderWindow()
{
    return m_vtkWidget->renderWindow();
}

vtkRenderer* VtkRenderWidget::getRenderer()
{
    return m_renderer;
}

void VtkRenderWidget::showAxes(bool show)
{
    // ������������������ʾ�߼�
    // ���ڴ��볤�����ƣ�����ֻ�ṩ�ӿ�
    m_vtkWidget->renderWindow()->Render();
}

void VtkRenderWidget::setInteractionMode(int mode)
{
    // ����������ò�ͬ�Ľ���ģʽ
    m_vtkWidget->renderWindow()->Render();
}