#pragma once

#include <QMainWindow>
#include <QHBoxLayout>

class VTKWidget;

class DimensionPlane : public QMainWindow
{
    Q_OBJECT

public:
    DimensionPlane(QWidget* parent = nullptr);
    ~DimensionPlane();

private:
    void setupUI();

    VTKWidget* m_vtkWidget;
    QHBoxLayout* m_mainLayout;
};