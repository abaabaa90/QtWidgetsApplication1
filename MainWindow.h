#pragma once
#include "FrameworkWindow.h"
#include "DataSet.h"
#include "ModelObject.h"

#include <QHash>
#include <QList>

class RenderViewContainer;
class ProjectTreePanel;
class VtkRenderWidget;
class RibbonBar;
class RibbonPage;
class ModelProperties;
class QDockWidget;
class QAction;
class QTableWidget;
class QPlainTextEdit;
class QStackedWidget;
class QCloseEvent;
class QToolButton;
class vtkActor;

/**
 * @brief 主窗口：FrameworkWindow 的具体实例
 *
 * 演示 ParaView 风格框架的完整骨架：
 *  - 中央：三维渲染视图（RenderViewContainer，可浮出为独立窗口）
 *  - 左停靠：工程树（ProjectTreePanel，可浮动/拖到任意边停靠）
 *  - 右停靠：属性；下停靠：输出
 *  - 菜单栏 / 工具栏 / 状态栏，主题切换联动
 */
class MainWindow : public FrameworkWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void onOpenFile();       // 打开数据文件并解析显示
    void onAbout();          // 关于对话框
    void onResetLayout();    // 重置停靠布局
    void onThemeToggled(bool dark);   // 菜单主题动作
    void syncThemeAction(Theme theme); // 标题栏按钮切换后同步菜单勾选
    void onDataSetActivated(int index); // 工程树选中数据集

    // 工程树右键菜单
    void onOpenFileLocation(int index);                  // 打开源文件所在位置
    void onRenameDataSet(int index, const QString& name); // 数据集重命名
    void onRemoveDataSet(int index);                     // 删除数据集
    void onDatasetVisibilityChanged(int index, bool visible); // 工程树勾选显隐

    // 功能区（Ribbon）/ 建模
    void createModel(int type);             // 创建模型对象（0=正方体 1=球 2=圆柱 3=圆锥 4=平面）
    void onModelSelected(vtkActor* actor);  // 视图内点选模型
    void onModelTransformChanged();         // gizmo 变换结束
    void onModelPropsEdited();              // 属性面板编辑
    void onDeleteSelectedModel();           // 删除选中模型
    void onClearModels();                   // 清除全部模型
    void onDrawToolButtonClicked(QToolButton* btn);   // 绘制工具按钮（排他激活）

    // 渲染视图浮动
    void toggleRenderFloat();   // 菜单/工具栏触发
    void floatRenderPanel();    // 渲染视图 -> 顶层浮动窗口
    void dockRenderPanel();     // 浮动窗口 -> 停靠回中央

protected:
    void closeEvent(QCloseEvent* event) override;  // 关闭前停靠回浮动中的渲染视图

private:
    void buildDocks();
    void buildMenus();
    void buildToolBar();
    void buildStatusBar();
    void buildRibbon();
    void addGeometryToolGroups(RibbonPage* page);   // 几何页签工具表构建（可扩展）
    void logMessage(const QString& msg);     // 输出 dock 打印日志
    void showDataSetInfo(const DataSet& ds); // 属性表显示数据集元信息
    VtkRenderWidget* activeVtk() const;      // 活动 3D 帧的 VTK 控件
    void refreshRender();                    // 按 m_datasets 重建所有数据集 actor
    void buildRenderPlaceholder();           // 渲染视图浮动时的中央占位

    // 模型对象
    void selectModel(int index);
    void clearModelSelection();
    void showModelProps();                  // 属性面板填充选中模型
    int  selectedModelIndex() const;

    RenderViewContainer* m_renderContainer = nullptr;   // 中央三维视图

    // 停靠面板
    ProjectTreePanel* m_projectTree = nullptr; // 左：工程树
    QDockWidget* m_dockProperties = nullptr;   // 右：属性
    QDockWidget* m_dockOutput = nullptr;       // 下：输出

    // 菜单/工具栏动作
    QAction* m_themeAction = nullptr;
    QAction* m_splitHAction = nullptr;
    QAction* m_splitVAction = nullptr;
    QAction* m_maximizeAction = nullptr;
    QAction* m_resetCameraAction = nullptr;
    QAction* m_renderFloatAction = nullptr;

    // 已加载的数据集
    QList<DataSet> m_datasets;
    int m_datasetCounter = 0;
    QTableWidget*  m_propTable = nullptr;  // 属性表
    QPlainTextEdit* m_logEdit = nullptr;   // 输出日志

    // 渲染视图浮动状态
    QWidget* m_renderPlaceholder = nullptr; // 浮动时占据中央
    bool     m_renderFloating    = false;

    // 功能区（Ribbon）
    RibbonBar* m_ribbon = nullptr;

    // 模型对象（3D 建模）
    QList<ModelObject> m_modelObjects;
    int  m_modelCounter    = 0;
    int  m_selectedModelId = -1;
    ModelProperties* m_modelProps = nullptr;   // 模型属性面板（属性停靠区页1）
    QStackedWidget*  m_propStack   = nullptr;  // 属性停靠区：页0=数据集 页1=模型
    QVector<QToolButton*> m_drawToolButtons;   // 绘制工具按钮（排他激活）

    // 多数据集显示
    QHash<int, bool> m_datasetVisible;      // 数据集索引 -> 是否显示
    QHash<int, int>  m_actorIds;            // 数据集索引 -> vtk 数据集 id
    bool             m_defaultSceneCleared = false;  // 是否已清掉默认场景

    bool m_syncingTheme = false;  // 防止菜单勾选与主题信号互相循环触发
};
