#include "pch.h"
// �����ص�������������¼�
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
                std::cout << "���õ�ǰ��ͼ�����" << std::endl;
            }
        }
        else if (key == "a" || key == "A")
        {
            // ����������ͼ
            for (auto renderer : renderers)
            {
                if (renderer) renderer->ResetCamera();
            }
            std::cout << "����������ͼ�����" << std::endl;
        }
        else if (key == "1")
        {
            // �л���������ͼ����
            std::cout << "������ͼ (XY) - ��Ƭ: " << mapperXY->GetSliceNumber() << std::endl;
        }
        else if (key == "2")
        {
            // �л���ʸ״��ͼ����
            std::cout << "ʸ״��ͼ (YZ) - ��Ƭ: " << mapperYZ->GetSliceNumber() << std::endl;
        }
        else if (key == "3")
        {
            // �л�����״��ͼ����
            std::cout << "��״��ͼ (XZ) - ��Ƭ: " << mapperXZ->GetSliceNumber() << std::endl;
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
        //std::cerr << "����: �޷���ȡDICOM����" << std::endl;
        return -1;
    }

    int dims[3];
    imageData->GetDimensions(dims);

    std::cout << "DICOM������Ϣ:" << std::endl;
    std::cout << "  ͼ��ߴ�: " << dims[0] << " �� " << dims[1] << " �� " << dims[2] << std::endl;
    std::cout << "  ��Ƭ����: " << dims[2] << std::endl;
    std::cout << "  ���Բ�ͬ��Ƭλ��..." << std::endl;

    // ������Ⱦ���ںͽ�����
    vtkNew<vtkRenderWindow> renderWindow;
    renderWindow->SetSize(1200, 800);
    renderWindow->SetWindowName("DICOM ��ƽ��鿴��");

    vtkNew<vtkRenderWindowInteractor> interactor;
    interactor->SetRenderWindow(renderWindow);

    // ������Ⱦ���б�
    std::vector<vtkRenderer*> renderers;

    // ������ͼ (XY) - ���Ͻ�
    vtkNew<vtkImageSliceMapper> mapperXY;
    mapperXY->SetInputConnection(reader->GetOutputPort());
    mapperXY->SetSliceNumber(dims[2] / 4);  // ����1/4λ��
    mapperXY->SetOrientationToZ();

    vtkNew<vtkImageSlice> sliceXY;
    sliceXY->SetMapper(mapperXY);

    vtkNew<vtkRenderer> rendererXY;
    rendererXY->AddViewProp(sliceXY);
    rendererXY->SetViewport(0.0, 0.5, 0.5, 1.0);
    rendererXY->SetBackground(0.2, 0.2, 0.3);
    renderWindow->AddRenderer(rendererXY);
    renderers.push_back(rendererXY);

    // ʸ״��ͼ (YZ) - ���Ͻ�
    vtkNew<vtkImageSliceMapper> mapperYZ;
    mapperYZ->SetInputConnection(reader->GetOutputPort());
    mapperYZ->SetSliceNumber(dims[0] / 4);  // ����1/4λ��
    mapperYZ->SetOrientationToX();

    vtkNew<vtkImageSlice> sliceYZ;
    sliceYZ->SetMapper(mapperYZ);

    vtkNew<vtkRenderer> rendererYZ;
    rendererYZ->AddViewProp(sliceYZ);
    rendererYZ->SetViewport(0.5, 0.5, 1.0, 1.0);
    rendererYZ->SetBackground(0.2, 0.3, 0.2);
    renderWindow->AddRenderer(rendererYZ);
    renderers.push_back(rendererYZ);

    // ��״��ͼ (XZ) - ���½�
    vtkNew<vtkImageSliceMapper> mapperXZ;
    mapperXZ->SetInputConnection(reader->GetOutputPort());
    mapperXZ->SetSliceNumber(dims[1] / 4);  // ����1/4λ��
    mapperXZ->SetOrientationToY();

    vtkNew<vtkImageSlice> sliceXZ;
    sliceXZ->SetMapper(mapperXZ);

    vtkNew<vtkRenderer> rendererXZ;
    rendererXZ->AddViewProp(sliceXZ);
    rendererXZ->SetViewport(0.0, 0.0, 0.5, 0.5);
    rendererXZ->SetBackground(0.3, 0.2, 0.2);
    renderWindow->AddRenderer(rendererXZ);
    renderers.push_back(rendererXZ);

    // ��Ϣ��� - ���½�
    vtkNew<vtkRenderer> rendererInfo;
    rendererInfo->SetViewport(0.5, 0.0, 1.0, 0.5);
    rendererInfo->SetBackground(0.1, 0.1, 0.1);
    renderWindow->AddRenderer(rendererInfo);
    renderers.push_back(rendererInfo);

    // ���ý�����ʽ
    vtkNew<vtkInteractorStyleImage> style;
    interactor->SetInteractorStyle(style);

    // ��Ӽ��̻ص�
    vtkNew<KeyPressCallback> keyPressCallback;
    keyPressCallback->mapperXY = mapperXY;
    keyPressCallback->mapperYZ = mapperYZ;
    keyPressCallback->mapperXZ = mapperXZ;
    keyPressCallback->SetRenderers(renderers);
    interactor->AddObserver(vtkCommand::KeyPressEvent, keyPressCallback);

    // ��ӡʹ��˵��
    std::cout << "\nʹ��˵��:" << std::endl;
    std::cout << "  1. �����������һ����ͼ" << std::endl;
    std::cout << "  2. ������: �ڼ������ͼ���л���Ƭ" << std::endl;
    std::cout << "  3. �������϶�: ƽ��ͼ��" << std::endl;
    std::cout << "  4. ����Ҽ��϶�: ����ͼ��" << std::endl;
    std::cout << "  5. �� 'r' ��: ���õ�ǰ��ͼ" << std::endl;
    std::cout << "  6. �� 'a' ��: ����������ͼ" << std::endl;
    std::cout << "  7. �� '1', '2', '3' ��: �鿴��ǰ��Ƭλ��" << std::endl;

    // �����������
    for (auto renderer : renderers)
    {
        renderer->ResetCamera();
    }

    // ��Ⱦ
    renderWindow->Render();
    interactor->Start();
    return 0;
}