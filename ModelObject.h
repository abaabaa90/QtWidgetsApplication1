#pragma once
#include <QString>

class vtkActor;

/// 场景中的模型对象（创建出的几何体）
struct ModelObject
{
    int id = 0;
    QString name;
    int type = 0;   // 0=正方体 1=球 2=圆柱 3=圆锥 4=平面
    vtkActor* actor = nullptr;
};
