#include "CustomLogTicker.h"
#include <cmath>
#include <QDebug>

FixedPowerTicker::FixedPowerTicker() :
    m_startExponent(0),
    m_exponentStep(2),
    m_maxTickCount(10)
{
}

void FixedPowerTicker::setStartExponent(int exponent)
{
    m_startExponent = exponent;
}

int FixedPowerTicker::startExponent() const
{
    return m_startExponent;
}

void FixedPowerTicker::setExponentStep(int step)
{
    if (step > 0)
        m_exponentStep = step;
}

int FixedPowerTicker::exponentStep() const
{
    return m_exponentStep;
}

void FixedPowerTicker::setMaxTickCount(int count)
{
    if (count > 0)
        m_maxTickCount = count;
}

int FixedPowerTicker::maxTickCount() const
{
    return m_maxTickCount;
}

QVector<double> FixedPowerTicker::createTickVector(double tickStep, const QCPRange& range)
{
    Q_UNUSED(tickStep);
    QVector<double> result;

    // 永远固定从设置的起始指数开始，不管数据范围！
    int exponent = m_startExponent;
    double candidate = std::pow(10.0, exponent);

    // 生成固定数量的刻度，不考虑数据范围
    for (int i = 0; i < m_maxTickCount; ++i)
    {
        result.append(candidate);
        exponent += m_exponentStep;
        candidate = std::pow(10.0, exponent);
    }

    qDebug() << "FixedPowerTicker: 固定刻度数量 =" << result.size();
    for (int i = 0; i < qMin(5, result.size()); ++i) {
        //qDebug() << " 刻度" << i << ": 10^" << (m_startExponent + i * m_exponentStep)<< " = " << result[i];
    }

    return result;
}

int FixedPowerTicker::getSubTickCount(double tickStep)
{
    Q_UNUSED(tickStep);
    return 0;
}

QString FixedPowerTicker::getTickLabel(double tick, const QLocale& locale,
    QChar formatChar, int precision)
{
    // 自定义标签格式：显示为10的指数形式
    double exponent = std::log10(tick);

    // 如果指数是整数，显示为10^n形式
    double intPart;
    if (std::modf(exponent, &intPart) < 1e-10) {
        if (intPart == 0) {
            return "10^0";
        }
        else {
            return QString("10^{%1}").arg(static_cast<int>(intPart));
        }
    }

    // 否则使用默认格式
    return QCPAxisTicker::getTickLabel(tick, locale, formatChar, precision);
}


FixedIntervalFromMinTicker::FixedIntervalFromMinTicker() :
    m_exponentStep(2),
    m_forceAllTicks(true)
{
}

void FixedIntervalFromMinTicker::setExponentStep(int step)
{
    if (step > 0)
        m_exponentStep = step;
}

int FixedIntervalFromMinTicker::exponentStep() const
{
    return m_exponentStep;
}

void FixedIntervalFromMinTicker::setForceAllTicks(bool force)
{
    m_forceAllTicks = force;
}

bool FixedIntervalFromMinTicker::forceAllTicks() const
{
    return m_forceAllTicks;
}

QVector<double> FixedIntervalFromMinTicker::createTickVector(double tickStep, const QCPRange& range)
{
    Q_UNUSED(tickStep);
    QVector<double> result;

    if (range.lower > 0 && range.upper > 0)
    {
        double min = range.lower;
        double max = range.upper;

        // 找到最小值的对数，并向下取整到最接近的偶数
        double logMin = std::log10(min);
        int startExponent = static_cast<int>(std::floor(logMin));

        // 调整起始指数到m_exponentStep的倍数
        int remainder = startExponent % m_exponentStep;
        if (remainder != 0) {
            startExponent = startExponent - remainder + m_exponentStep;
        }

        // 如果起始指数对应的值小于最小值，增加一个步长
        double candidate = std::pow(10.0, startExponent);
        if (candidate < min) {
            startExponent += m_exponentStep;
            candidate = std::pow(10.0, startExponent);
        }

        // 生成刻度
        while ((m_forceAllTicks || candidate <= max) &&
            result.size() < 20) // 防止无限循环
        {
            result.append(candidate);
            startExponent += m_exponentStep;
            candidate = std::pow(10.0, startExponent);
        }

        qDebug() << "FixedIntervalFromMinTicker: 范围 [" << min << "," << max << "]";
        qDebug() << "FixedIntervalFromMinTicker: 起始指数 =" << startExponent - m_exponentStep * (result.size() - 1);
        qDebug() << "FixedIntervalFromMinTicker: 刻度数量 =" << result.size();
        qDebug() << "FixedIntervalFromMinTicker: 刻度序列:";
        for (double tick : result) {
            qDebug() << "  " << tick << " (10^" << std::log10(tick) << ")";
        }
    }

    return result;
}

int FixedIntervalFromMinTicker::getSubTickCount(double tickStep)
{
    Q_UNUSED(tickStep);
    return 0;
}
