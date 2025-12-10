#pragma once
#include "pch.h"
#include "TitleBar.h"
class MainWindow : public QWidget
{
public:
	MainWindow();
	~MainWindow();
	
private:
	QHBoxLayout* TitleBar_MainWindow_Layout = nullptr;
	TitleBar* TitleBar_MainWindow = nullptr;
	


};
