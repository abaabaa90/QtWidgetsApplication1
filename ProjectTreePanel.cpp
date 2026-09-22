#include "ProjectTreePanel.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------
ProjectTreePanel::ProjectTreePanel(QWidget* parent)
    : DockPanelBase(QStringLiteral("工程树"), parent)
{
    // 在派生构造体内调用虚函数，避免基类构造期间虚调用
    setContentWidget(createContent());

    connect(m_tree, &QTreeWidget::itemSelectionChanged,
            this, &ProjectTreePanel::onItemSelectionChanged);
    connect(m_tree, &QTreeWidget::customContextMenuRequested,
            this, &ProjectTreePanel::showContextMenu);
    connect(m_tree, &QTreeWidget::itemChanged,
            this, &ProjectTreePanel::onItemChanged);
}

//-----------------------------------------------------------------------------
QWidget* ProjectTreePanel::createContent()
{
    auto* container = new QWidget(this);
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    // 顶部筛选框
    m_filterEdit = new QLineEdit(container);
    m_filterEdit->setObjectName(QStringLiteral("projectTreeFilter"));
    m_filterEdit->setPlaceholderText(QStringLiteral("筛选数据集 / 变量..."));
    m_filterEdit->setClearButtonEnabled(true);
    layout->addWidget(m_filterEdit);
    connect(m_filterEdit, &QLineEdit::textChanged,
            this, &ProjectTreePanel::onFilterTextChanged);

    m_tree = new QTreeWidget(container);
    m_tree->setObjectName(QStringLiteral("projectTree"));
    m_tree->setHeaderHidden(true);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setEditTriggers(QAbstractItemView::EditKeyPressed);   // F2 重命名
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_tree, 1);

    m_projectRoot = new QTreeWidgetItem(m_tree, { QStringLiteral("工程") });
    m_projectRoot->setExpanded(true);

    return container;
}

//-----------------------------------------------------------------------------
QTreeWidgetItem* ProjectTreePanel::addDataSet(const DataSet& ds, int index)
{
    if (!m_tree || !m_projectRoot) return nullptr;

    // 程序化建节点会触发 itemChanged，用标志抑制重命名/勾选信号
    m_restoringTree = true;
    auto* item = new QTreeWidgetItem(m_projectRoot, { ds.name });
    item->setData(0, Qt::UserRole, index);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);   // 显隐复选框
    item->setCheckState(0, Qt::Checked);
    item->setExpanded(true);
    m_checkStates[index] = Qt::Checked;

    // 每个变量一个子节点（同样携带数据集索引，选中变量也能联动属性表）
    for (const auto& var : ds.variables) {
        auto* varItem = new QTreeWidgetItem(item, { var });
        varItem->setData(0, Qt::UserRole, index);
    }
    m_restoringTree = false;

    m_projectRoot->setExpanded(true);
    return item;
}

//-----------------------------------------------------------------------------
void ProjectTreePanel::rebuildFromDatasets(const QList<DataSet>& datasets)
{
    clearDataSets();
    m_checkStates.clear();
    for (int i = 0; i < datasets.size(); ++i) {
        addDataSet(datasets.at(i), i);
    }
}

//-----------------------------------------------------------------------------
void ProjectTreePanel::clearDataSets()
{
    if (!m_tree || !m_projectRoot) return;
    while (m_projectRoot->childCount() > 0) {
        delete m_projectRoot->takeChild(0);
    }
}

//-----------------------------------------------------------------------------
int ProjectTreePanel::currentDataSetIndex() const
{
    if (!m_tree) return -1;
    QTreeWidgetItem* item = m_tree->currentItem();
    if (!item) return -1;

    const int idx = item->data(0, Qt::UserRole).toInt();
    // UserRole 无效（工程根节点等）返回 -1
    return item->data(0, Qt::UserRole).isValid() ? idx : -1;
}

//-----------------------------------------------------------------------------
void ProjectTreePanel::selectDataSet(int index)
{
    if (!m_tree || !m_projectRoot) return;

    for (int i = 0; i < m_projectRoot->childCount(); ++i) {
        QTreeWidgetItem* item = m_projectRoot->child(i);
        if (item->data(0, Qt::UserRole).toInt() == index) {
            m_tree->setCurrentItem(item);
            return;
        }
    }
}

//-----------------------------------------------------------------------------
void ProjectTreePanel::onItemSelectionChanged()
{
    const int idx = currentDataSetIndex();
    if (idx >= 0) {
        emit dataSetActivated(idx);
    }
}

//-----------------------------------------------------------------------------
void ProjectTreePanel::showContextMenu(const QPoint& pos)
{
    QTreeWidgetItem* item = m_tree->itemAt(pos);
    if (!m_tree) return;

    // 根节点或空白处：导入新数据集
    if (!item || item == m_projectRoot) {
        QMenu menu(m_tree);
        QAction* act = menu.addAction(QStringLiteral("导入数据集..."));
        if (menu.exec(m_tree->viewport()->mapToGlobal(pos)) == act) {
            emit importRequested();
        }
        return;
    }

    // 数据集/变量节点：携带数据集索引
    const QVariant v = item->data(0, Qt::UserRole);
    if (!v.isValid()) return;
    const int idx = v.toInt();

    const bool isDataSet = (item->parent() == m_projectRoot);   // 数据集节点

    QMenu menu(m_tree);
    QAction* openLoc = menu.addAction(QStringLiteral("打开文件所在位置"));
    QAction* renameAct = isDataSet ? menu.addAction(QStringLiteral("重命名")) : nullptr;
    QAction* removeAct = menu.addAction(QStringLiteral("删除数据集"));
    if (isDataSet) menu.addSeparator();

    QAction* chosen = menu.exec(m_tree->viewport()->mapToGlobal(pos));
    if (chosen == openLoc) {
        emit openFileLocationRequested(idx);
    } else if (chosen == renameAct) {
        m_renameOriginal = item->text(0);
        m_tree->editItem(item, 0);
    } else if (chosen == removeAct) {
        emit removeRequested(idx);
    }
}

//-----------------------------------------------------------------------------
void ProjectTreePanel::onItemChanged(QTreeWidgetItem* item, int column)
{
    if (m_restoringTree || !item || column != 0) return;
    // 仅数据集节点参与（变量/根节点忽略）
    if (item->parent() != m_projectRoot) return;
    if (!item->data(0, Qt::UserRole).isValid()) return;
    const int idx = item->data(0, Qt::UserRole).toInt();

    // 复选框状态变化 -> 显隐切换
    const Qt::CheckState cs = item->checkState(0);
    if (m_checkStates.value(idx, Qt::Checked) != cs) {
        m_checkStates[idx] = cs;
        emit visibilityChanged(idx, cs == Qt::Checked);
        return;
    }

    // 文本变化 -> 重命名
    const QString newName = item->text(0).trimmed();
    if (newName.isEmpty()) {
        // 空名回退到原名
        item->setText(0, m_renameOriginal);
        return;
    }
    emit renameRequested(idx, newName);
}

//-----------------------------------------------------------------------------
void ProjectTreePanel::onFilterTextChanged(const QString& text)
{
    if (!m_tree || !m_projectRoot) return;
    const QString q = text.trimmed();

    for (int i = 0; i < m_projectRoot->childCount(); ++i) {
        QTreeWidgetItem* ds = m_projectRoot->child(i);
        const bool dsMatch = ds->text(0).contains(q, Qt::CaseInsensitive);

        if (q.isEmpty() || dsMatch) {
            // 数据集名匹配（或空查询）：显示全部子节点
            ds->setHidden(false);
            for (int j = 0; j < ds->childCount(); ++j) {
                ds->child(j)->setHidden(false);
            }
            if (dsMatch) ds->setExpanded(true);
            continue;
        }

        // 数据集名不匹配：看是否有子节点匹配
        bool childMatch = false;
        for (int j = 0; j < ds->childCount(); ++j) {
            QTreeWidgetItem* v = ds->child(j);
            const bool vm = v->text(0).contains(q, Qt::CaseInsensitive);
            v->setHidden(!vm);
            childMatch |= vm;
        }
        ds->setHidden(!childMatch);
        if (childMatch) ds->setExpanded(true);
    }
}
