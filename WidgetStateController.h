#pragma once
#include "pch.h"
class WidgetStateController
{
public:
	WidgetStateController();
	~WidgetStateController();
	void save(QWidget* p);
	void load(QWidget* p);
	QMap<QWidget*, QVariantMap> QMapState;//或者需要根据ObjectName来进行保存到文件当中比较好?
	QMap<std::string, QVariantMap>QMapState_string;

};

