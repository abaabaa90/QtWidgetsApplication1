#pragma once
#include <QString>
#include <QWidget>

class StyleManager
{
public:
    static StyleManager& instance();

    void loadFromResource(const QString& resourcePath);
    void applyToApp();

    // Golden ratio
    static constexpr double PHI = 1.618033988749895;

    // Screen-proportional base unit = min(screenW, screenH) / 20
    static int baseUnit();

    // Golden-section split of a total: returns the SMALLER part
    static int goldenSmaller(int total);

    // Golden-section split: returns the LARGER part
    static int goldenLarger(int total);

    // Width from height using golden ratio
    static int goldenWidth(int height);

    // Height from width using golden ratio
    static int goldenHeight(int width);

private:
    StyleManager() = default;
    QString m_styleSheet;
};
