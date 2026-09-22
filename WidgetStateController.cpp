#include "WidgetStateController.h"

WidgetStateController::WidgetStateController()
{
}

WidgetStateController::~WidgetStateController()
{
}

void WidgetStateController::save(QWidget* p)
{
	const auto children = p->findChildren<QWidget*>();
	for (QWidget* w : children)
	{

	}
	auto save = [&](QWidget* t) -> void {
		if (!t) { return; }
		QVariantMap state;
		if (auto* T = qobject_cast<QPushButton*>(t))
		{

		}
		else if (auto* T = qobject_cast<QRadioButton*>(t))
		{

		}
		else if (auto* T = qobject_cast<QLabel*>(t))
		{

		}
		else if (auto* T = qobject_cast<QGroupBox*>(t))
		{

		}
		else if (auto* T = qobject_cast<QLineEdit*>(t))
		{

		}
		else if (auto* T = qobject_cast<QComboBox*>(t))
		{

		}
		else if (auto* T = qobject_cast<QTabWidget*>(t))
		{

		}
		else if (auto* T = qobject_cast<QAbstractButton*>(t))
		{

		}
		else if (auto* T = qobject_cast<QCheckBox*>(t))
		{

		}
	};
}

void WidgetStateController::load(QWidget* p)
{

}
