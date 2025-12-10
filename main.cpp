#pragma once
#include "CylinderExample.h"
#include "BaseWindow.h"
#include "MainWindow.h"
#include "pch.h"
#include <vtkAxesActor.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextActor.h>
#include "DimensionPlane.h"
#include <vtkAutoInit.h>
#pragma execution_character_set("utf-8")

// VTK模块初始化
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
VTK_MODULE_INIT(vtkRenderingContextOpenGL2);

int main(int argc, char* argv[])
{
	// 设置OpenGL表面格式（确保兼容性）
	QSurfaceFormat format;
	format.setRenderableType(QSurfaceFormat::OpenGL);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setVersion(3, 2);
	format.setSamples(8);
	QSurfaceFormat::setDefaultFormat(format);
	QApplication app(argc, argv); 
	DimensionPlane w;
	w.show();

	return app.exec();
}
