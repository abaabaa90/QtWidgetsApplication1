#pragma once
#include <QString>
#include <QStringList>
#include <vector>

/**
 * @brief 已加载的数据集模型
 *
 * 目前用于承载 Reader 解析出的 Tecplot 数据，后续阶段可扩展
 * 标量数组、颜色映射等字段。前 3 个变量视为三维坐标（不足补 0）。
 */
struct DataSet
{
    QString name;                          // 显示名，如 "dataset1"
    QString filePath;                      // 源文件路径
    QString title;                         // 文件内 Title
    QStringList variables;                 // 变量名（X[m]/Y[m]/...）
    std::vector<std::vector<double>> points; // 每行一个点

    size_t pointCount() const { return points.size(); }
    size_t dimension() const { return points.empty() ? 0 : points[0].size(); }
};
