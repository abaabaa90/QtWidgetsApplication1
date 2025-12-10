#include "CylinderExample.h"

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCylinderSource.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkLookupTable.h>
#include <array>
#include <vtkXMLImageDataReader.h>
#include <vtkContourFilter.h>
#include <vtkSphereSource.h>
#include <vtkLight.h>

 void CylinderExample::example()
{
     // 1. 创建数据源
     vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
     sphereSource->SetRadius(1.0);
     sphereSource->SetThetaResolution(30);
     sphereSource->SetPhiResolution(30);

     // 2. 创建映射器
     vtkSmartPointer<vtkPolyDataMapper> mapper =
         vtkSmartPointer<vtkPolyDataMapper>::New();
     mapper->SetInputConnection(sphereSource->GetOutputPort());

     // 3. 创建演员
     vtkSmartPointer<vtkActor> actor =
         vtkSmartPointer<vtkActor>::New();
     actor->SetMapper(mapper);

     // 4. 创建渲染器和窗口
     vtkSmartPointer<vtkRenderer> renderer =
         vtkSmartPointer<vtkRenderer>::New();
     vtkSmartPointer<vtkRenderWindow> renderWindow =
         vtkSmartPointer<vtkRenderWindow>::New();
     renderWindow->AddRenderer(renderer);

     // 5. 创建交互器
     vtkSmartPointer<vtkRenderWindowInteractor> interactor =
         vtkSmartPointer<vtkRenderWindowInteractor>::New();
     interactor->SetRenderWindow(renderWindow);

     // 6. 添加到场景
     renderer->AddActor(actor);
     renderer->SetBackground(0.1, 0.2, 0.3);

     // 7. 开始渲染
     renderWindow->Render();
     interactor->Start();
}

 void  CylinderExample::Cylinderexample()
 {
     vtkNew<vtkNamedColors> colors;
     std::array<unsigned char, 4> bkg{ {26, 51, 102, 255} };
     colors->SetColor("BkgColor", bkg.data());

     vtkNew<vtkCylinderSource> cylinder;
     cylinder->SetResolution(8);
  vtkNew<vtkPolyDataMapper> cylinderMapper;
     cylinderMapper->SetInputConnection(cylinder->GetOutputPort());

     // The actor is a grouping mechanism: besides the geometry (mapper), it
     // also has a property, transformation matrix, and/or texture map.
     // Here we set its color and rotate it around the X and Y axes.
     vtkNew<vtkActor> cylinderActor;
     cylinderActor->SetMapper(cylinderMapper);
     cylinderActor->GetProperty()->SetColor(
         colors->GetColor4d("Tomato").GetData());
     cylinderActor->RotateX(30.0);
     cylinderActor->RotateY(-45.0);

     // The renderer generates the image
     // which is then displayed on the render window.
     // It can be thought of as a scene to which the actor is added
     vtkNew<vtkRenderer> renderer;
     renderer->AddActor(cylinderActor);
     renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());
     // Zoom in a little by accessing the camera and invoking its "Zoom" method.
     renderer->ResetCamera();
     renderer->GetActiveCamera()->Zoom(1.5);
     vtkSmartPointer<vtkLight> mLight = vtkSmartPointer<vtkLight>::New();
     mLight->SetColor(0,9,0);
     mLight->SetPosition(0, 0, 1);
     mLight->SetFocalPoint(renderer->GetActiveCamera()->GetFocalPoint());
     renderer->AddLight(mLight);
     vtkSmartPointer<vtkLight> mLight2 = vtkSmartPointer<vtkLight>::New();
     mLight2->SetColor(0, 0, 1);
     mLight2->SetPosition(0, 0, -1);
     mLight2->SetFocalPoint(renderer->GetActiveCamera()->GetFocalPoint());
     renderer->AddLight(mLight2);

     // The render window is the actual GUI window
     // that appears on the computer screen
     vtkNew<vtkRenderWindow> renderWindow;
     renderWindow->SetSize(300, 300);
     renderWindow->AddRenderer(renderer);
     renderWindow->SetWindowName("Cylinder");

     // The render window interactor captures mouse events
     // and will perform appropriate camera or actor manipulation
     // depending on the nature of the events.
     vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
     renderWindowInteractor->SetRenderWindow(renderWindow);

     // This starts the event loop and as a side effect causes an initial render.
     renderWindow->Render();
     renderWindowInteractor->Start();

     return;
 }