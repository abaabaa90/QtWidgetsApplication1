#ifndef FIXEDPOWERTICKER_H
#define FIXEDPOWERTICKER_H

#include <QSharedPointer>
#include "qcustomplot.h"

class FixedPowerTicker : public QCPAxisTicker
{
public:
    FixedPowerTicker();

    // 设置起始指数（默认0，即10^0=1开始）
    void setStartExponent(int exponent);
    int startExponent() const;

    // 设置指数步长（默认2，即10^0, 10^2, 10^4...）
    void setExponentStep(int step);
    int exponentStep() const;

    // 设置最大刻度数量（默认10）
    void setMaxTickCount(int count);
    int maxTickCount() const;

protected:
    // 生成刻度向量
    QVector<double> createTickVector(double tickStep, const QCPRange& range) override;

    // 子刻度数量
    int getSubTickCount(double tickStep) override;

    // 刻度标签格式化
    QString getTickLabel(double tick, const QLocale& locale,
        QChar formatChar, int precision) override;

private:
    int m_startExponent;
    int m_exponentStep;
    int m_maxTickCount;
};

#endif // FIXEDPOWERTICKER_H
#ifndef FIXEDINTERVALFROMMINTICKER_H
#define FIXEDINTERVALFROMMINTICKER_H

#include <QSharedPointer>
#include "qcustomplot.h"

class FixedIntervalFromMinTicker : public QCPAxisTicker
{
public:
    FixedIntervalFromMinTicker();

    // 设置指数步长（默认2，即10^2间隔）
    void setExponentStep(int step);
    int exponentStep() const;

    // 设置是否强制包含所有刻度（默认true）
    void setForceAllTicks(bool force);
    bool forceAllTicks() const;

protected:
    QVector<double> createTickVector(double tickStep, const QCPRange& range) override;
    int getSubTickCount(double tickStep) override;

private:
    int m_exponentStep;
    bool m_forceAllTicks;
};

#endif // FIXEDINTERVALFROMMINTICKER_H
