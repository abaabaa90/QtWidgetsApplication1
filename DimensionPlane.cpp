#include "DimensionPlane.h"
#include "vtkWidget.h"

#include <QVBoxLayout>
#include <QWidget>

DimensionPlane::DimensionPlane(QWidget* parent)
    : QMainWindow(parent)
{
    setupUI();
    resize(1200, 800);
    setWindowTitle("VTK 3D Viewer");
}

DimensionPlane::~DimensionPlane()
{
}

void DimensionPlane::setupUI()
{
    // �������벿��
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // ����������
    m_mainLayout = new QHBoxLayout(centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    // ����VTK����
    m_vtkWidget = new VTKWidget(centralWidget);
    m_mainLayout->addWidget(m_vtkWidget);
}