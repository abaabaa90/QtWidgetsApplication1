#pragma once

#include <QWidget>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>

// 前向声明VTK类
class vtkRenderer;
class vtkGenericOpenGLRenderWindow;
class vtkRenderWindowInteractor;
class vtkPlaneSource;
class vtkActor;
class vtkPolyDataMapper;
class vtkAxesActor;
class vtkOrientationMarkerWidget;
class vtkInteractorStyleTrackballCamera;

class VTKWidget : public QVTKOpenGLNativeWidget
{
    Q_OBJECT

public:
    explicit VTKWidget(QWidget* parent = nullptr);
    ~VTKWidget();

    void resetView();
    void setCameraToDefault();

private:
    void initializeVTK();
    void createGroundPlane();
    void setupCamera();
    void setupAxes();
    void setupInteractor();

    // VTK智能指针
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkRenderWindowInteractor> m_interactor;
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> m_interactorStyle;

    vtkSmartPointer<vtkPlaneSource> m_planeSource;
    vtkSmartPointer<vtkActor> m_planeActor;
    vtkSmartPointer<vtkPolyDataMapper> m_planeMapper;

    vtkSmartPointer<vtkAxesActor> m_axesActor;
    vtkSmartPointer<vtkOrientationMarkerWidget> m_axesWidget;

    bool m_isInitialized;
};