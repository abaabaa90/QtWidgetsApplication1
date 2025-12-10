#include "MainWindow.h"
MainWindow::MainWindow()
{
	QWidget* a = new QWidget();
	TitleBar_MainWindow = new TitleBar(a);
	a->show();
}

MainWindow::~MainWindow()
{
}