#include "MainWindow.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QStringLiteral("3D Visualization Platform"));
    a.setApplicationVersion(QStringLiteral("1.0.0"));

    // Set application-wide font
    QFont appFont = a.font();
    appFont.setPointSize(10);
    a.setFont(appFont);

    // Set global style
    a.setStyle(QStringLiteral("Fusion"));

    // Create and show main window
    MainWindow w;
    w.showCentered();

    return a.exec();
}
