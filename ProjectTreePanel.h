#pragma once
#include "DockPanelBase.h"
#include "DataSet.h"

#include <QHash>
#include <QList>

class QTreeWidget;
class QTreeWidgetItem;
class QMenu;
class QPoint;
class QLineEdit;

/**
 * @brief 工程树面板（替代原"数据浏览器"）
 *
 * 三级结构（类 VS 解决方案资源管理器）：
 *   工程(Project) -> 数据集(Dataset，每个打开的文件一项) -> 变量(Variable)
 *
 * 选中"数据集"或"变量"节点都会发出 dataSetActivated(数据集索引)。
 */
class ProjectTreePanel : public DockPanelBase
{
    Q_OBJECT

public:
    explicit ProjectTreePanel(QWidget* parent = nullptr);

    /// 添加一个数据集节点（UserRole=index），返回新节点
    QTreeWidgetItem* addDataSet(const DataSet& ds, int index);
    /// 清空数据集与变量节点，仅保留工程根
    void clearDataSets();
    /// 按数据集列表重建数据集节点（删除后重排索引用）
    void rebuildFromDatasets(const QList<DataSet>& datasets);
    /// 当前选中节点对应的数据集索引；无则返回 -1
    int currentDataSetIndex() const;
    /// 选中指定数据集节点
    void selectDataSet(int index);

    QTreeWidget* tree() const { return m_tree; }

signals:
    /// 数据集/变量节点被选中，参数为数据集索引
    void dataSetActivated(int index);
    /// 请求打开数据集源文件所在位置
    void openFileLocationRequested(int index);
    /// 数据集被重命名
    void renameRequested(int index, const QString& newName);
    /// 请求删除数据集
    void removeRequested(int index);
    /// 请求导入新数据集（打开文件对话框）
    void importRequested();
    /// 数据集显隐复选框被切换
    void visibilityChanged(int index, bool visible);

protected:
    QWidget* createContent() override;

private:
    void onItemSelectionChanged();
    void showContextMenu(const QPoint& pos);
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onFilterTextChanged(const QString& text);

    QTreeWidget*     m_tree          = nullptr;
    QTreeWidgetItem* m_projectRoot   = nullptr;
    QLineEdit*       m_filterEdit    = nullptr;
    bool             m_restoringTree = false;   // 程序化更新树时抑制 itemChanged
    QString          m_renameOriginal;          // 重命名前的原名（空名时回退）
    QHash<int, Qt::CheckState> m_checkStates;   // 数据集索引 -> 复选框状态（区分勾选/改名）
};
