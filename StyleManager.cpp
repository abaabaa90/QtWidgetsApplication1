#include "StyleManager.h"
#include <QApplication>
#include <QScreen>
#include <QFile>
#include <algorithm>

StyleManager& StyleManager::instance()
{
    static StyleManager inst;
    return inst;
}

void StyleManager::loadFromResource(const QString& resourcePath)
{
    QFile file(resourcePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_styleSheet = QString::fromUtf8(file.readAll());
        file.close();
    }
}

void StyleManager::applyToApp()
{
    qApp->setStyleSheet(qApp->styleSheet() + m_styleSheet);
}

int StyleManager::baseUnit()
{
    QScreen* screen = QApplication::primaryScreen();
    if (!screen) return 50;
    QSize sz = screen->availableSize();
    int minDim = std::min(sz.width(), sz.height());
    return std::max(minDim / 20, 32);  // floor at 32px
}

int StyleManager::goldenSmaller(int total)
{
    return static_cast<int>(total / (PHI + 1.0));
}

int StyleManager::goldenLarger(int total)
{
    return total - goldenSmaller(total);
}

int StyleManager::goldenWidth(int height)
{
    return static_cast<int>(height * PHI);
}

int StyleManager::goldenHeight(int width)
{
    return static_cast<int>(width / PHI);
}
