#include "MainWindow.h"
#include "RenderViewContainer.h"
#include "RenderViewFrame.h"
#include "VtkRenderWidget.h"
#include "DockPanelBase.h"
#include "ProjectTreePanel.h"
#include "RibbonBar.h"
#include "RibbonPage.h"
#include "RibbonGroup.h"
#include "ModelProperties.h"
#include "Reader.h"
#include "StyleManager.h"
#include "ThemeManager.h"
#include <vtkActor.h>

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QShortcut>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTableWidget>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------
MainWindow::MainWindow()
    : FrameworkWindow(nullptr)
{
    int unit = StyleManager::baseUnit();

    setWindowTitle(QStringLiteral("3D Visualization Platform"));
    setWindowSize(StyleManager::goldenWidth(unit * 12), unit * 12);
    setMinimumWindowSize(unit * 14, unit * 9);
    setWindowRadius(0);
    setContentMargins(0, 0, 0, 0);

    buildDocks();
    buildMenus();
    buildToolBar();
    buildStatusBar();
    buildRibbon();

    // 恢复上次保存的停靠布局
    restorePersistentState();

    // 恢复上次渲染视图的浮动状态
    if (ThemeManager::appSettings().value(QStringLiteral("layout/renderFloating"), false).toBool()) {
        floatRenderPanel();
    }
}

//-----------------------------------------------------------------------------
void MainWindow::buildDocks()
{
    // 中央：三维渲染视图
    m_renderContainer = new RenderViewContainer(this);
    setCentralWidget(m_renderContainer);
    buildRenderPlaceholder();

    // 左：工程树（DockPanelBase 子类：自定义标题栏 + 可浮动/四边停靠）
    m_projectTree = new ProjectTreePanel(this);
    addDockPanel(m_projectTree, Qt::LeftDockWidgetArea);

    // 右：属性（页0=数据集信息，页1=模型对象属性）
    m_propTable = new QTableWidget(0, 2);
    m_propTable->setHorizontalHeaderLabels({ QStringLiteral("属性"), QStringLiteral("值") });
    m_propTable->verticalHeader()->setVisible(false);
    m_propTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_propTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_modelProps = new ModelProperties;
    m_propStack = new QStackedWidget;
    m_propStack->addWidget(m_propTable);    // 页0：数据集信息
    m_propStack->addWidget(m_modelProps);   // 页1：模型对象属性
    auto* propPanel = new DockPanelBase(QStringLiteral("属性"));
    propPanel->setContentWidget(m_propStack);
    m_dockProperties = propPanel;
    addDockPanel(m_dockProperties, Qt::RightDockWidgetArea);
    connect(m_modelProps, &ModelProperties::edited,
            this, &MainWindow::onModelPropsEdited);

    // 下：输出
    m_logEdit = new QPlainTextEdit;
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumBlockCount(2000);
    m_logEdit->appendPlainText(QStringLiteral("就绪"));
    auto* outputPanel = new DockPanelBase(QStringLiteral("输出"));
    outputPanel->setContentWidget(m_logEdit);
    m_dockOutput = outputPanel;
    addDockPanel(m_dockOutput, Qt::BottomDockWidgetArea);

    // 初始停靠尺寸
    int unit = StyleManager::baseUnit();
    resizeDocks({ m_projectTree }, { StyleManager::goldenSmaller(unit * 12) }, Qt::Horizontal);
    resizeDocks({ m_dockOutput }, { unit * 3 }, Qt::Vertical);

    // 工程树选中数据集/变量 -> 属性表显示元信息
    connect(m_projectTree, &ProjectTreePanel::dataSetActivated,
            this, &MainWindow::onDataSetActivated);
    // 工程树右键菜单动作
    connect(m_projectTree, &ProjectTreePanel::importRequested,
            this, &MainWindow::onOpenFile);
    connect(m_projectTree, &ProjectTreePanel::openFileLocationRequested,
            this, &MainWindow::onOpenFileLocation);
    connect(m_projectTree, &ProjectTreePanel::renameRequested,
            this, &MainWindow::onRenameDataSet);
    connect(m_projectTree, &ProjectTreePanel::removeRequested,
            this, &MainWindow::onRemoveDataSet);
    connect(m_projectTree, &ProjectTreePanel::visibilityChanged,
            this, &MainWindow::onDatasetVisibilityChanged);

    // 3D 视图：点选模型 / gizmo 变换 / 绘制工具
    if (auto* vtk = activeVtk()) {
        connect(vtk, &VtkRenderWidget::actorSelected,
                this, &MainWindow::onModelSelected);
        connect(vtk, &VtkRenderWidget::transformChanged,
                this, &MainWindow::onModelTransformChanged);
        connect(vtk, &VtkRenderWidget::drawToolChanged, this, [this](DrawTool tool) {
            // 视图内切换/退出绘制时同步按钮选中态
            for (QToolButton* b : qAsConst(m_drawToolButtons)) {
                b->setChecked(static_cast<DrawTool>(b->property("drawTool").toInt()) == tool);
            }
        });
    }
}

//-----------------------------------------------------------------------------
void MainWindow::buildRenderPlaceholder()
{
    m_renderPlaceholder = new QWidget(m_shell);
    m_renderPlaceholder->setObjectName(QStringLiteral("renderPlaceholder"));
    auto* layout = new QVBoxLayout(m_renderPlaceholder);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* label = new QLabel(QStringLiteral("渲染视图已浮动为独立窗口\n"
                                            "点击“浮动渲染视图”或关闭浮动窗口即可恢复"),
                             m_renderPlaceholder);
    label->setObjectName(QStringLiteral("renderPlaceholderLabel"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    m_renderPlaceholder->hide();
}

//-----------------------------------------------------------------------------
void MainWindow::buildMenus()
{
    // 文件
    auto* fileMenu = menuBar()->addMenu(QStringLiteral("文件(&F)"));
    QAction* openAct = fileMenu->addAction(QStringLiteral("打开数据(&O)..."));
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &MainWindow::onOpenFile);
    fileMenu->addSeparator();
    QAction* exitAct = fileMenu->addAction(QStringLiteral("退出(&Q)"));
    exitAct->setShortcut(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, this, []() { QApplication::closeAllWindows(); });

    // 视图
    auto* viewMenu = menuBar()->addMenu(QStringLiteral("视图(&V)"));
    m_themeAction = viewMenu->addAction(QStringLiteral("深色主题(&D)"));
    m_themeAction->setCheckable(true);
    m_themeAction->setChecked(ThemeManager::instance().currentTheme() == Theme::Dark);
    connect(m_themeAction, &QAction::toggled, this, &MainWindow::onThemeToggled);

    m_renderFloatAction = viewMenu->addAction(QStringLiteral("浮动渲染视图(&F)"));
    m_renderFloatAction->setCheckable(true);
    // 用 triggered 而不是 toggled，避免内部 setChecked 造成信号回环
    connect(m_renderFloatAction, &QAction::triggered,
            this, &MainWindow::toggleRenderFloat);

    viewMenu->addAction(QStringLiteral("重置视图布局(&R)"), this, &MainWindow::onResetLayout);

    // 窗口
    auto* windowMenu = menuBar()->addMenu(QStringLiteral("窗口(&W)"));
    windowMenu->addAction(m_projectTree->toggleViewAction());
    windowMenu->addAction(m_dockProperties->toggleViewAction());
    windowMenu->addAction(m_dockOutput->toggleViewAction());

    // 帮助
    auto* helpMenu = menuBar()->addMenu(QStringLiteral("帮助(&H)"));
    helpMenu->addAction(QStringLiteral("关于(&A)"), this, &MainWindow::onAbout);
}

//-----------------------------------------------------------------------------
void MainWindow::buildToolBar()
{
    QToolBar* tb = addToolBar(QStringLiteral("主工具栏"));
    tb->setObjectName(QStringLiteral("mainToolBar"));
    tb->setMovable(false);

    tb->addAction(QStringLiteral("打开"), this, &MainWindow::onOpenFile);
    tb->addSeparator();

    m_splitHAction = tb->addAction(QStringLiteral("H分"), this, [this]() {
        if (auto* f = m_renderContainer ? m_renderContainer->activeFrame() : nullptr)
            m_renderContainer->splitHorizontal(f);
    });
    m_splitVAction = tb->addAction(QStringLiteral("V分"), this, [this]() {
        if (auto* f = m_renderContainer ? m_renderContainer->activeFrame() : nullptr)
            m_renderContainer->splitVertical(f);
    });
    m_maximizeAction = tb->addAction(QStringLiteral("最大化"), this, [this]() {
        if (auto* f = m_renderContainer ? m_renderContainer->activeFrame() : nullptr)
            m_renderContainer->maximizeFrame(f);
    });
    tb->addSeparator();

    m_resetCameraAction = tb->addAction(QStringLiteral("重置相机"), this, [this]() {
        if (auto* f = m_renderContainer ? m_renderContainer->activeFrame() : nullptr)
            f->vtkRenderWidget()->resetCamera();
    });
    tb->addSeparator();
    tb->addAction(m_renderFloatAction);   // 主题切换在标题栏 ☀ 按钮，工具栏不再重复
}

//-----------------------------------------------------------------------------
void MainWindow::buildStatusBar()
{
    statusBar()->showMessage(QStringLiteral("就绪"));

    auto* themeLabel = new QLabel(
        QStringLiteral("主题：%1").arg(ThemeManager::themeName(ThemeManager::instance().currentTheme())),
        statusBar());
    statusBar()->addPermanentWidget(themeLabel);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [themeLabel](Theme t) {
                themeLabel->setText(QStringLiteral("主题：%1").arg(ThemeManager::themeName(t)));
            });
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &MainWindow::syncThemeAction);
}

//-----------------------------------------------------------------------------
void MainWindow::buildRibbon()
{
    // 功能区放在顶部聚合栏之下、宿主内容区之上
    m_ribbon = new RibbonBar(this);
    m_contentLayout->insertWidget(m_contentLayout->indexOf(m_shell), m_ribbon);

    // 页签：几何（先只做一个栏目，后续可扩展 开始/插入/视图...）
    RibbonPage* geoPage = m_ribbon->addPage(QStringLiteral("几何"));

    // 创建组：基本体素
    RibbonGroup* primGroup = geoPage->addGroup(QStringLiteral("创建"));
    connect(primGroup->addTool(QStringLiteral("正方体"), QStringLiteral("▣")),
            &QToolButton::clicked, this, [this]() { createModel(0); });
    connect(primGroup->addTool(QStringLiteral("球体"), QStringLiteral("●")),
            &QToolButton::clicked, this, [this]() { createModel(1); });
    connect(primGroup->addTool(QStringLiteral("圆柱"), QStringLiteral("▯")),
            &QToolButton::clicked, this, [this]() { createModel(2); });
    connect(primGroup->addTool(QStringLiteral("圆锥"), QStringLiteral("▲")),
            &QToolButton::clicked, this, [this]() { createModel(3); });
    connect(primGroup->addTool(QStringLiteral("平面"), QStringLiteral("▭")),
            &QToolButton::clicked, this, [this]() { createModel(4); });

    // 编辑组
    RibbonGroup* editGroup = geoPage->addGroup(QStringLiteral("编辑"));
    connect(editGroup->addTool(QStringLiteral("删除选中"), QStringLiteral("✕")),
            &QToolButton::clicked, this, &MainWindow::onDeleteSelectedModel);
    connect(editGroup->addTool(QStringLiteral("清除全部"), QStringLiteral("🗑")),
            &QToolButton::clicked, this, &MainWindow::onClearModels);

    // Delete 键删除选中
    auto* delShortcut = new QShortcut(QKeySequence::Delete, this);
    connect(delShortcut, &QShortcut::activated, this, &MainWindow::onDeleteSelectedModel);

    // 工具表方式追加建模工具组（绘制/修改/注释/图层/选择...），易扩展
    addGeometryToolGroups(geoPage);
}

//-----------------------------------------------------------------------------
namespace {

QString modelTypeName(int type)
{
    switch (type) {
    case 0: return QStringLiteral("正方体");
    case 1: return QStringLiteral("球体");
    case 2: return QStringLiteral("圆柱");
    case 3: return QStringLiteral("圆锥");
    case 4: return QStringLiteral("平面");
    default: return QStringLiteral("未知");
    }
}

// ---- 几何页签工具表（可扩展：加一个工具 = 加一行）----
enum class GeoToolCmd {
    None,                       // 仅前端占位
    DrawLine, DrawPolyline, DrawCircle, DrawArc,   // 交互式绘制
};
struct ToolDef {
    const char* name;
    const char* glyph;
    GeoToolCmd cmd = GeoToolCmd::None;
};
struct ToolGroupDef {
    const char* title;
    QVector<ToolDef> tools;
};

const QVector<ToolGroupDef> kGeometryTools = {
    { "绘制", {
        { "直线", "╱", GeoToolCmd::DrawLine },
        { "多段线", "∿", GeoToolCmd::DrawPolyline },
        { "圆", "○", GeoToolCmd::DrawCircle },
        { "圆弧", "⌒", GeoToolCmd::DrawArc },
    }},
    { "修改", {
        { "移动", "⇔" }, { "旋转", "⟳" }, { "缩放", "⤢" }, { "复制", "⧉" },
        { "镜像", "◧" }, { "修剪", "✂" }, { "圆角", "⌓" }, { "拉伸", "⇕" },
    }},
    { "注释", {
        { "文字", "A" }, { "标注", "⇄" },
    }},
    { "图层", {
        { "图层", "☰" },
    }},
    { "选择", {
        { "选择", "☑" },
    }},
};

DrawTool drawToolForCmd(GeoToolCmd cmd)
{
    switch (cmd) {
    case GeoToolCmd::DrawLine:     return DrawTool::Line;
    case GeoToolCmd::DrawPolyline: return DrawTool::Polyline;
    case GeoToolCmd::DrawCircle:   return DrawTool::Circle;
    case GeoToolCmd::DrawArc:      return DrawTool::Arc;
    default:                       return DrawTool::None;
    }
}

} // namespace

//-----------------------------------------------------------------------------
void MainWindow::addGeometryToolGroups(RibbonPage* page)
{
    if (!page) return;

    for (const ToolGroupDef& g : kGeometryTools) {
        RibbonGroup* group = page->addGroup(QString::fromUtf8(g.title));
        for (const ToolDef& t : g.tools) {
            const QString name = QString::fromUtf8(t.name);
            QToolButton* btn = group->addTool(name, QString::fromUtf8(t.glyph));

            switch (t.cmd) {
            case GeoToolCmd::DrawLine:
            case GeoToolCmd::DrawPolyline:
            case GeoToolCmd::DrawCircle:
            case GeoToolCmd::DrawArc: {
                // 交互式绘制工具：可选中、排他激活
                btn->setCheckable(true);
                btn->setProperty("drawTool", int(drawToolForCmd(t.cmd)));
                m_drawToolButtons.append(btn);
                connect(btn, &QToolButton::clicked, this, [this, btn]() {
                    onDrawToolButtonClicked(btn);
                });
                break;
            }
            default:
                // 前端按钮已就位；功能暂未实现，点击仅记录
                connect(btn, &QToolButton::clicked, this, [this, name]() {
                    logMessage(QStringLiteral("[几何] %1（功能待实现）").arg(name));
                });
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------------
void MainWindow::onDrawToolButtonClicked(QToolButton* btn)
{
    if (!btn) return;

    // 排他：只保留当前按钮的选中态
    for (QToolButton* b : qAsConst(m_drawToolButtons)) {
        if (b != btn) b->setChecked(false);
    }

    const DrawTool tool = static_cast<DrawTool>(btn->property("drawTool").toInt());
    if (auto* vtk = activeVtk()) {
        // 再次点击已激活按钮 -> 取消
        vtk->setDrawTool(btn->isChecked() ? tool : DrawTool::None);
    }
}

//-----------------------------------------------------------------------------
void MainWindow::createModel(int type)
{
    auto* vtk = activeVtk();
    if (!vtk) {
        logMessage(QStringLiteral("[几何] 没有可用的三维视图"));
        return;
    }

    static const double palette[][3] = {
        { 0.95, 0.45, 0.35 }, { 0.40, 0.80, 0.45 }, { 0.95, 0.70, 0.25 },
        { 0.70, 0.45, 0.90 }, { 0.30, 0.80, 0.80 }, { 0.90, 0.35, 0.70 },
    };

    // 首次创建模型时清掉默认演示场景，得到干净工作区
    if (!m_defaultSceneCleared) {
        vtk->clearAll();
        m_defaultSceneCleared = true;
    }

    ModelObject obj;
    obj.id = ++m_modelCounter;
    obj.name = QStringLiteral("几何体%1").arg(obj.id);
    obj.type = type;
    obj.actor = vtk->createModel(type, palette[(obj.id - 1) % 6]);
    if (!obj.actor) return;

    m_modelObjects.append(obj);
    selectModel(m_modelObjects.size() - 1);
    vtk->resetCamera();
    logMessage(QStringLiteral("[几何] 创建 %1").arg(obj.name));
}

//-----------------------------------------------------------------------------
int MainWindow::selectedModelIndex() const
{
    for (int i = 0; i < m_modelObjects.size(); ++i) {
        if (m_modelObjects.at(i).id == m_selectedModelId) return i;
    }
    return -1;
}

//-----------------------------------------------------------------------------
void MainWindow::onModelSelected(vtkActor* actor)
{
    if (!actor) {
        clearModelSelection();
        return;
    }
    for (int i = 0; i < m_modelObjects.size(); ++i) {
        if (m_modelObjects.at(i).actor == actor) {
            selectModel(i);
            return;
        }
    }
    // 非模型 actor（gizmo 手柄 / 点云）忽略，不取消选择
}

//-----------------------------------------------------------------------------
void MainWindow::selectModel(int index)
{
    if (index < 0 || index >= m_modelObjects.size()) return;
    if (m_modelObjects.at(index).id == m_selectedModelId) return;

    clearModelSelection();
    m_selectedModelId = m_modelObjects.at(index).id;

    auto* vtk = activeVtk();
    if (vtk) {
        vtk->setActorHighlighted(m_modelObjects.at(index).actor, true);
        vtk->attachTransformGizmo(m_modelObjects.at(index).actor);
    }
    showModelProps();
}

//-----------------------------------------------------------------------------
void MainWindow::clearModelSelection()
{
    if (m_selectedModelId < 0) return;
    for (const auto& obj : m_modelObjects) {
        if (obj.id == m_selectedModelId) {
            if (auto* vtk = activeVtk()) {
                vtk->setActorHighlighted(obj.actor, false);
                vtk->detachTransformGizmo();
            }
            break;
        }
    }
    m_selectedModelId = -1;
    if (m_propStack) m_propStack->setCurrentIndex(0);   // 属性区切回数据集页
}

//-----------------------------------------------------------------------------
void MainWindow::showModelProps()
{
    if (!m_modelProps || !m_propStack) return;
    const int idx = selectedModelIndex();
    if (idx < 0) return;

    const ModelObject& obj = m_modelObjects.at(idx);
    double p[3], o[3], s[3];
    obj.actor->GetPosition(p);
    obj.actor->GetOrientation(o);
    obj.actor->GetScale(s);
    m_modelProps->setValues(obj.name, modelTypeName(obj.type),
                            p[0], p[1], p[2], o[0], o[1], o[2], s[0], s[1], s[2]);
    m_propStack->setCurrentIndex(1);
}

//-----------------------------------------------------------------------------
void MainWindow::onModelTransformChanged()
{
    showModelProps();   // gizmo 松手后从 actor 重新读取（setValues 抑制 edited，无回环）
}

//-----------------------------------------------------------------------------
void MainWindow::onModelPropsEdited()
{
    const int idx = selectedModelIndex();
    if (idx < 0 || !m_modelProps) return;

    ModelObject& obj = m_modelObjects[idx];
    if (auto* vtk = activeVtk()) {
        vtk->setActorTransform(obj.actor,
            m_modelProps->posX(), m_modelProps->posY(), m_modelProps->posZ(),
            m_modelProps->rotX(), m_modelProps->rotY(), m_modelProps->rotZ(),
            m_modelProps->scaleX(), m_modelProps->scaleY(), m_modelProps->scaleZ());
    }
    if (obj.name != m_modelProps->name()) {
        obj.name = m_modelProps->name();
        logMessage(QStringLiteral("[几何] 重命名 → %1").arg(obj.name));
    }
}

//-----------------------------------------------------------------------------
void MainWindow::onDeleteSelectedModel()
{
    const int idx = selectedModelIndex();
    if (idx < 0) return;

    const ModelObject obj = m_modelObjects.takeAt(idx);
    if (auto* vtk = activeVtk()) {
        vtk->setActorHighlighted(obj.actor, false);
        vtk->removeActor(obj.actor);
    }
    m_selectedModelId = -1;
    if (m_propStack) m_propStack->setCurrentIndex(0);
    logMessage(QStringLiteral("[几何] 删除 %1").arg(obj.name));
}

//-----------------------------------------------------------------------------
void MainWindow::onClearModels()
{
    if (auto* vtk = activeVtk()) {
        for (const auto& obj : qAsConst(m_modelObjects)) {
            vtk->removeActor(obj.actor);
        }
        vtk->clearSketches();   // 一并清除已绘制的草图
    }
    m_modelObjects.clear();
    m_selectedModelId = -1;
    if (m_propStack) m_propStack->setCurrentIndex(0);
    logMessage(QStringLiteral("[几何] 已清除全部模型"));
}

//-----------------------------------------------------------------------------
void MainWindow::onOpenFile()
{
    const QString file = QFileDialog::getOpenFileName(
        this, QStringLiteral("打开数据文件"), QString(),
        QStringLiteral("Tecplot 数据文件 (*.dat *.txt);;所有文件 (*.*)"));
    if (file.isEmpty()) return;

    // 解析文件
    Reader reader;
    if (!reader.ReadFile(file.toLocal8Bit().constData())) {
        logMessage(QStringLiteral("[错误] 无法解析文件：%1").arg(file));
        showStatusMessage(QStringLiteral("打开失败：%1").arg(file));
        return;
    }

    // 组装数据集
    DataSet ds;
    ds.name = QStringLiteral("dataset%1").arg(++m_datasetCounter);
    ds.filePath = file;
    ds.title = QString::fromStdString(reader.DataTitle);
    for (const auto& v : reader.Variables) {
        ds.variables.append(QString::fromStdString(v));
    }
    ds.points = std::move(reader.Points);

    // 记录数据集
    const int idx = m_datasets.size();
    m_datasets.append(std::move(ds));
    const DataSet& stored = m_datasets.last();

    // 工程树加一项（数据集 + 变量子节点）
    m_projectTree->addDataSet(stored, idx);
    m_projectTree->selectDataSet(idx);

    // 首次加载时清掉默认的圆锥/球/圆柱演示场景
    if (!m_defaultSceneCleared) {
        if (auto* vtk = activeVtk()) vtk->clearAll();
        m_defaultSceneCleared = true;
    }
    m_datasetVisible[idx] = true;
    refreshRender();   // 重建所有数据集 actor（多数据集同屏）

    // 日志 + 属性 + 状态栏
    logMessage(QStringLiteral("[打开] %1").arg(file));
    logMessage(QStringLiteral("  标题：%1").arg(stored.title));
    logMessage(QStringLiteral("  变量：%1").arg(stored.variables.join(QStringLiteral("  "))));
    logMessage(QStringLiteral("  点数：%1，维度：%2")
                   .arg(stored.pointCount()).arg(stored.dimension()));
    showDataSetInfo(stored);
    showStatusMessage(QStringLiteral("已打开：%1（%2 个点）")
                          .arg(file).arg(stored.pointCount()));
}

//-----------------------------------------------------------------------------
void MainWindow::logMessage(const QString& msg)
{
    if (!m_logEdit) return;
    const QString line = QStringLiteral("[%1] %2")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss")), msg);
    m_logEdit->appendPlainText(line);
    QTextCursor cursor = m_logEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_logEdit->setTextCursor(cursor);
}

//-----------------------------------------------------------------------------
void MainWindow::showDataSetInfo(const DataSet& ds)
{
    if (!m_propTable) return;
    if (m_propStack) m_propStack->setCurrentIndex(0);   // 属性区切到数据集页
    m_propTable->setRowCount(5);
    m_propTable->setItem(0, 0, new QTableWidgetItem(QStringLiteral("名称")));
    m_propTable->setItem(0, 1, new QTableWidgetItem(ds.name));
    m_propTable->setItem(1, 0, new QTableWidgetItem(QStringLiteral("标题")));
    m_propTable->setItem(1, 1, new QTableWidgetItem(ds.title));
    m_propTable->setItem(2, 0, new QTableWidgetItem(QStringLiteral("变量")));
    m_propTable->setItem(2, 1, new QTableWidgetItem(ds.variables.join(QStringLiteral(", "))));
    m_propTable->setItem(3, 0, new QTableWidgetItem(QStringLiteral("点数")));
    m_propTable->setItem(3, 1, new QTableWidgetItem(QString::number(static_cast<qlonglong>(ds.pointCount()))));
    m_propTable->setItem(4, 0, new QTableWidgetItem(QStringLiteral("维度")));
    m_propTable->setItem(4, 1, new QTableWidgetItem(QString::number(static_cast<qlonglong>(ds.dimension()))));
}

//-----------------------------------------------------------------------------
void MainWindow::onDataSetActivated(int idx)
{
    if (idx >= 0 && idx < m_datasets.size()) {
        showDataSetInfo(m_datasets.at(idx));
    }
}

//-----------------------------------------------------------------------------
namespace {
// 数据集调色板：按索引取一个区分度高的颜色
void colorForIndex(int idx, double out[3])
{
    static const double palette[][3] = {
        { 0.30, 0.65, 0.95 },   // 蓝
        { 0.95, 0.45, 0.35 },   // 红
        { 0.40, 0.80, 0.45 },   // 绿
        { 0.95, 0.70, 0.25 },   // 橙
        { 0.70, 0.45, 0.90 },   // 紫
        { 0.30, 0.80, 0.80 },   // 青
        { 0.90, 0.35, 0.70 },   // 粉
        { 0.60, 0.60, 0.60 },   // 灰
    };
    const double(&c)[3] = palette[idx % 8];
    out[0] = c[0]; out[1] = c[1]; out[2] = c[2];
}
} // namespace

//-----------------------------------------------------------------------------
VtkRenderWidget* MainWindow::activeVtk() const
{
    if (!m_renderContainer) return nullptr;
    RenderViewFrame* frame = m_renderContainer->activeFrame();
    return frame ? frame->vtkRenderWidget() : nullptr;
}

//-----------------------------------------------------------------------------
void MainWindow::refreshRender()
{
    VtkRenderWidget* vtk = activeVtk();
    if (!vtk) return;

    vtk->clearDatasets();
    m_actorIds.clear();
    for (int i = 0; i < m_datasets.size(); ++i) {
        double c[3];
        colorForIndex(i, c);
        const int did = vtk->addDataset(m_datasets.at(i).points, c);
        m_actorIds[i] = did;
        vtk->setDatasetVisible(did, m_datasetVisible.value(i, true));
    }
    vtk->resetCamera();
}

//-----------------------------------------------------------------------------
void MainWindow::onDatasetVisibilityChanged(int idx, bool visible)
{
    if (idx < 0 || idx >= m_datasets.size()) return;
    m_datasetVisible[idx] = visible;
    if (auto* vtk = activeVtk()) {
        if (m_actorIds.contains(idx)) {
            vtk->setDatasetVisible(m_actorIds.value(idx), visible);
        }
    }
    logMessage(QStringLiteral("[显隐] %1 → %2").arg(m_datasets.at(idx).name,
                                                     visible ? QStringLiteral("显示") : QStringLiteral("隐藏")));
}

//-----------------------------------------------------------------------------
void MainWindow::onOpenFileLocation(int idx)
{
    if (idx < 0 || idx >= m_datasets.size()) return;
    const QString path = m_datasets.at(idx).filePath;
    if (path.isEmpty()) return;

    // 打开所在目录并选中该文件
    QProcess::startDetached(QStringLiteral("explorer.exe"),
                            QStringList() << QStringLiteral("/select,")
                                          + QDir::toNativeSeparators(path));
}

//-----------------------------------------------------------------------------
void MainWindow::onRenameDataSet(int idx, const QString& newName)
{
    if (idx < 0 || idx >= m_datasets.size()) return;
    m_datasets[idx].name = newName;
    logMessage(QStringLiteral("[重命名] %1").arg(newName));
}

//-----------------------------------------------------------------------------
void MainWindow::onRemoveDataSet(int idx)
{
    if (idx < 0 || idx >= m_datasets.size()) return;

    const QString name = m_datasets.at(idx).name;
    const auto ret = QMessageBox::question(
        this, QStringLiteral("删除数据集"),
        QStringLiteral("确定删除数据集 \"%1\" 吗？").arg(name),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    m_datasets.removeAt(idx);
    m_projectTree->rebuildFromDatasets(m_datasets);
    m_datasetVisible.remove(idx);
    logMessage(QStringLiteral("[删除] %1").arg(name));

    if (!m_datasets.isEmpty()) {
        // 重建渲染与选中，保持一致
        refreshRender();
        m_projectTree->selectDataSet(m_datasets.size() - 1);
    } else {
        if (auto* vtk = activeVtk()) { vtk->clearDatasets(); vtk->resetCamera(); }
        if (m_propTable) m_propTable->clearContents();
    }
}

//-----------------------------------------------------------------------------
void MainWindow::onAbout()
{
    QMessageBox::about(this, QStringLiteral("关于"),
        QStringLiteral("3D Visualization Platform\nVersion 1.0\n\n"
                       "ParaView 风格框架窗口 + 黑白主题 + 可浮动工程树"));
}

//-----------------------------------------------------------------------------
void MainWindow::onResetLayout()
{
    // 若渲染视图处于浮动状态，先停靠回中央
    if (m_renderFloating) dockRenderPanel();

    // 把三个面板恢复到默认停靠位置
    m_shell->removeDockWidget(m_projectTree);
    m_shell->removeDockWidget(m_dockProperties);
    m_shell->removeDockWidget(m_dockOutput);
    m_shell->addDockWidget(Qt::LeftDockWidgetArea, m_projectTree);
    m_shell->addDockWidget(Qt::RightDockWidgetArea, m_dockProperties);
    m_shell->addDockWidget(Qt::BottomDockWidgetArea, m_dockOutput);
    m_shell->setCentralWidget(m_renderContainer);
    showStatusMessage(QStringLiteral("视图布局已重置"));
}

//-----------------------------------------------------------------------------
void MainWindow::onThemeToggled(bool dark)
{
    if (m_syncingTheme) return;
    ThemeManager::instance().setTheme(dark ? Theme::Dark : Theme::Light);
}

//-----------------------------------------------------------------------------
void MainWindow::syncThemeAction(Theme theme)
{
    if (!m_themeAction) return;
    m_syncingTheme = true;
    m_themeAction->setChecked(theme == Theme::Dark);
    m_syncingTheme = false;
}

//-----------------------------------------------------------------------------
void MainWindow::toggleRenderFloat()
{
    if (m_renderFloating) {
        dockRenderPanel();
    } else {
        floatRenderPanel();
    }
}

//-----------------------------------------------------------------------------
void MainWindow::floatRenderPanel()
{
    if (m_renderFloating || !m_renderContainer) return;

    // 1) 摘除中央槽位：takeCentralWidget 不会 delete，只是 setParent(0)
    QWidget* render = m_shell->takeCentralWidget();
    if (render && render != m_renderContainer) render->setParent(nullptr);

    // 2) 作为顶层浮动窗口显示
    m_renderContainer->setWindowFlag(Qt::Window, true);
    m_renderContainer->setWindowTitle(QStringLiteral("渲染视图"));
    m_renderContainer->resize(qMax(m_renderContainer->width(), 900),
                              qMax(m_renderContainer->height(), 640));
    m_renderContainer->show();

    // 3) 中央换占位
    m_shell->setCentralWidget(m_renderPlaceholder);
    m_renderPlaceholder->show();

    m_renderFloating = true;
    if (m_renderFloatAction) m_renderFloatAction->setChecked(true);
    ThemeManager::appSettings().setValue(QStringLiteral("layout/renderFloating"), true);
}

//-----------------------------------------------------------------------------
void MainWindow::dockRenderPanel()
{
    if (!m_renderFloating || !m_renderContainer) return;

    // 1) 移除占位（takeCentralWidget 不会 delete）
    QWidget* ph = m_shell->takeCentralWidget();
    if (ph && ph != m_renderContainer) ph->hide();

    // 2) 恢复为子控件并放回中央
    m_renderContainer->setWindowFlag(Qt::Window, false);
    m_renderContainer->setParent(m_shell);
    m_renderContainer->show();
    m_shell->setCentralWidget(m_renderContainer);

    // 3) 重挂载后强制 VTK 渲染（原生窗口重建）
    if (auto* frame = m_renderContainer->activeFrame()) {
        if (auto* vtk = frame->vtkRenderWidget()) {
            if (auto* rw = vtk->getRenderWindow()) rw->Render();
        }
    }

    m_renderFloating = false;
    if (m_renderFloatAction) m_renderFloatAction->setChecked(false);
    ThemeManager::appSettings().setValue(QStringLiteral("layout/renderFloating"), false);
}

//-----------------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent* event)
{
    // 关闭主窗口前先停靠回渲染视图，避免留下孤立的浮动窗口
    if (m_renderFloating) dockRenderPanel();
    FrameworkWindow::closeEvent(event);
}
