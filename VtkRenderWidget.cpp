#include "VtkRenderWidget.h"
#include "ThemeManager.h"

#include <QVBoxLayout>
#include <vtkProperty.h>
#include <vtkConeSource.h>
#include <vtkSphereSource.h>
#include <vtkCylinderSource.h>
#include <vtkCubeSource.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkLight.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkCellPicker.h>
#include <vtkBoxWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkTransform.h>
#include <vtkNew.h>
#include <vtkCommand.h>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace {
const double kPI = 3.14159265358979323846;   // MSVC 默认不定义 M_PI
}

VtkRenderWidget::VtkRenderWidget(QWidget* parent)
    : QWidget(parent)
    , m_vtkWidget(nullptr)
    , m_renderer(nullptr)
    , m_initialized(false)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    layout->addWidget(m_vtkWidget);

    initializeVtk();
    addLights();
    createDefaultScene();

    // 视口背景跟随主题
    applyThemeBackground(ThemeManager::instance().currentTheme());
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &VtkRenderWidget::applyThemeBackground);
}

VtkRenderWidget::~VtkRenderWidget()
{
}

void VtkRenderWidget::initializeVtk()
{
    if (m_initialized) return;

    m_renderer = vtkRenderer::New();
    m_renderer->SetBackground(0.12, 0.12, 0.18);

    m_vtkWidget->renderWindow()->AddRenderer(m_renderer);

    // ---- 点选拾取：左键单击（按下→松开移动<5px）选择对象 ----
    m_picker = vtkSmartPointer<vtkCellPicker>::New();
    m_picker->SetTolerance(0.005);

    m_pickCB = vtkSmartPointer<vtkCallbackCommand>::New();
    m_pickCB->SetCallback(VtkRenderWidget::OnPickCallback);
    m_pickCB->SetClientData(this);

    // 绘制预览 / 键盘
    m_moveCB = vtkSmartPointer<vtkCallbackCommand>::New();
    m_moveCB->SetCallback(VtkRenderWidget::OnMoveCallback);
    m_moveCB->SetClientData(this);

    m_keyCB = vtkSmartPointer<vtkCallbackCommand>::New();
    m_keyCB->SetCallback(VtkRenderWidget::OnKeyCallback);
    m_keyCB->SetClientData(this);

    if (auto* iren = m_vtkWidget->renderWindow()->GetInteractor()) {
        iren->SetPicker(m_picker);
        iren->AddObserver(vtkCommand::LeftButtonPressEvent, m_pickCB);
        iren->AddObserver(vtkCommand::LeftButtonReleaseEvent, m_pickCB);
        iren->AddObserver(vtkCommand::RightButtonPressEvent, m_pickCB);
        iren->AddObserver(vtkCommand::MouseMoveEvent, m_moveCB);
        iren->AddObserver(vtkCommand::KeyPressEvent, m_keyCB);
    }

    // ---- 变换 gizmo 回调 ----
    m_boxCB = vtkSmartPointer<vtkCallbackCommand>::New();
    m_boxCB->SetCallback(VtkRenderWidget::OnBoxCallback);
    m_boxCB->SetClientData(this);

    m_initialized = true;
}

void VtkRenderWidget::createDefaultScene()
{
    addCone(3.0, 1.0, 20, new double[3]{ 0.2, 0.6, 1.0 }, new double[3]{ -3.0, 0.0, 0.0 });
    addSphere(1.2, 20, 20, new double[3]{ 1.0, 0.4, 0.3 }, new double[3]{ 0.0, 0.0, 0.0 });
    addCylinder(3.0, 0.8, 20, new double[3]{ 0.3, 0.8, 0.4 }, new double[3]{ 3.0, 0.0, 0.0 });

    resetCamera();
}

void VtkRenderWidget::addLights()
{
    auto* light = vtkLight::New();
    light->SetLightTypeToSceneLight();
    light->SetPosition(1.0, 2.0, 4.0);
    light->SetIntensity(0.8);
    m_renderer->AddLight(light);
    light->Delete();
}

void VtkRenderWidget::addCone(double height, double radius, int resolution,
    const double color[3], const double position[3])
{
    auto* source = vtkConeSource::New();
    source->SetHeight(height);
    source->SetRadius(radius);
    source->SetResolution(resolution);

    auto* mapper = vtkPolyDataMapper::New();
    mapper->SetInputConnection(source->GetOutputPort());

    double defaultColor[3] = { 0.5, 0.5, 0.5 };
    double defaultPos[3] = { 0, 0, 0 };

    auto* actor = createActor(mapper,
        color ? color : defaultColor,
        position ? position : defaultPos);

    source->Delete();
    mapper->Delete();

    m_actors.push_back(actor);
}

void VtkRenderWidget::addSphere(double radius, int thetaResolution, int phiResolution,
    const double color[3], const double position[3])
{
    auto* source = vtkSphereSource::New();
    source->SetRadius(radius);
    source->SetThetaResolution(thetaResolution);
    source->SetPhiResolution(phiResolution);

    auto* mapper = vtkPolyDataMapper::New();
    mapper->SetInputConnection(source->GetOutputPort());

    double defaultColor[3] = { 0.5, 0.5, 0.5 };
    double defaultPos[3] = { 0, 0, 0 };

    auto* actor = createActor(mapper,
        color ? color : defaultColor,
        position ? position : defaultPos);

    source->Delete();
    mapper->Delete();

    m_actors.push_back(actor);
}

void VtkRenderWidget::addCylinder(double height, double radius, int resolution,
    const double color[3], const double position[3])
{
    auto* source = vtkCylinderSource::New();
    source->SetHeight(height);
    source->SetRadius(radius);
    source->SetResolution(resolution);

    auto* mapper = vtkPolyDataMapper::New();
    mapper->SetInputConnection(source->GetOutputPort());

    double defaultColor[3] = { 0.5, 0.5, 0.5 };
    double defaultPos[3] = { 0, 0, 0 };

    auto* actor = createActor(mapper,
        color ? color : defaultColor,
        position ? position : defaultPos);

    source->Delete();
    mapper->Delete();

    m_actors.push_back(actor);
}

void VtkRenderWidget::addCube(double xLength, double yLength, double zLength,
    const double color[3], const double position[3])
{
    auto* source = vtkCubeSource::New();
    source->SetXLength(xLength);
    source->SetYLength(yLength);
    source->SetZLength(zLength);

    auto* mapper = vtkPolyDataMapper::New();
    mapper->SetInputConnection(source->GetOutputPort());

    double defaultColor[3] = { 0.5, 0.5, 0.5 };
    double defaultPos[3] = { 0, 0, 0 };

    auto* actor = createActor(mapper,
        color ? color : defaultColor,
        position ? position : defaultPos);

    source->Delete();
    mapper->Delete();

    m_actors.push_back(actor);
}

void VtkRenderWidget::addPlane(double width, double depth,
    const double color[3], const double position[3])
{
    auto* source = vtkPlaneSource::New();
    source->SetXResolution(1);
    source->SetYResolution(1);
    source->SetOrigin(-width / 2, -depth / 2, 0);
    source->SetPoint1(width / 2, -depth / 2, 0);
    source->SetPoint2(-width / 2, depth / 2, 0);

    auto* mapper = vtkPolyDataMapper::New();
    mapper->SetInputConnection(source->GetOutputPort());

    double defaultColor[3] = { 0.5, 0.5, 0.5 };
    double defaultPos[3] = { 0, 0, 0 };

    auto* actor = createActor(mapper,
        color ? color : defaultColor,
        position ? position : defaultPos);

    source->Delete();
    mapper->Delete();

    m_actors.push_back(actor);
}

vtkActor* VtkRenderWidget::createActor(vtkPolyDataMapper* mapper,
    const double color[3], const double position[3])
{
    // 空指针防护：color/position 都可能为 nullptr（如 createModel 不传位置时）
    static const double kDefaultColor[3] = { 0.5, 0.5, 0.5 };
    static const double kDefaultPos[3]   = { 0.0, 0.0, 0.0 };
    const double* col = color ? color : kDefaultColor;
    const double* pos = position ? position : kDefaultPos;

    auto* actor = vtkActor::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(col[0], col[1], col[2]);
    actor->SetPosition(pos[0], pos[1], pos[2]);
    actor->GetProperty()->SetSpecular(0.3);
    actor->GetProperty()->SetSpecularPower(20);
    actor->GetProperty()->SetDiffuse(0.8);
    actor->GetProperty()->SetAmbient(0.2);

    m_renderer->AddActor(actor);
    return actor;
}

void VtkRenderWidget::setBackgroundColor(double r, double g, double b)
{
    if (m_renderer)
    {
        m_renderer->SetBackground(r, g, b);
        m_vtkWidget->renderWindow()->Render();
    }
}

void VtkRenderWidget::applyThemeBackground(Theme theme)
{
    if (!m_renderer) return;

    if (theme == Theme::Dark) {
        m_renderer->SetBackground(0.12, 0.12, 0.18);   // 深色主题：近黑视口
    } else {
        m_renderer->SetBackground(0.75, 0.75, 0.75);   // 浅色主题：浅灰视口
    }

    if (m_vtkWidget) {
        m_vtkWidget->renderWindow()->Render();
    }
}

void VtkRenderWidget::clearAll()
{
    for (vtkActor* actor : m_actors) {
        m_renderer->RemoveActor(actor);
        actor->Delete();
    }
    m_actors.clear();

    clearDatasets();
}

vtkActor* VtkRenderWidget::createPointCloudActor(
    const std::vector<std::vector<double>>& points, const double color[3])
{
    // 建 vtkPoints：前 3 个变量作为 x/y/z，不足补 0
    auto* vtkPts = vtkPoints::New();
    vtkPts->SetDataTypeToDouble();
    for (const auto& row : points) {
        double p[3] = { 0.0, 0.0, 0.0 };
        if (row.size() >= 1) p[0] = row[0];
        if (row.size() >= 2) p[1] = row[1];
        if (row.size() >= 3) p[2] = row[2];
        vtkPts->InsertNextPoint(p);
    }

    auto* poly = vtkPolyData::New();
    poly->SetPoints(vtkPts);
    vtkPts->Delete();

    // 纯点集要转成 vertex cell 才能被 mapper 渲染
    auto* glyph = vtkVertexGlyphFilter::New();
    glyph->SetInputData(poly);
    poly->Delete();

    auto* mapper = vtkPolyDataMapper::New();
    mapper->SetInputConnection(glyph->GetOutputPort());
    glyph->Delete();

    double defaultColor[3] = { 0.3, 0.6, 0.9 };
    auto* actor = createActor(mapper, color ? color : defaultColor, nullptr);
    actor->GetProperty()->SetPointSize(5);
    mapper->Delete();
    return actor;
}

vtkActor* VtkRenderWidget::addPointCloud(const std::vector<std::vector<double>>& points,
                                         const double color[3])
{
    auto* actor = createPointCloudActor(points, color);
    m_actors.push_back(actor);
    return actor;
}

int VtkRenderWidget::addDataset(const std::vector<std::vector<double>>& points,
                                const double color[3])
{
    const int id = m_nextDatasetId++;
    auto* actor = createPointCloudActor(points, color);
    m_datasetActors.insert(id, actor);
    return id;
}

void VtkRenderWidget::removeDataset(int id)
{
    vtkActor* actor = m_datasetActors.take(id);
    if (actor) {
        m_renderer->RemoveActor(actor);
        actor->Delete();
    }
}

void VtkRenderWidget::setDatasetVisible(int id, bool visible)
{
    vtkActor* actor = m_datasetActors.value(id);
    if (actor) {
        actor->SetVisibility(visible ? 1 : 0);
        if (m_vtkWidget) m_vtkWidget->renderWindow()->Render();
    }
}

void VtkRenderWidget::clearDatasets()
{
    for (vtkActor* actor : qAsConst(m_datasetActors)) {
        m_renderer->RemoveActor(actor);
        actor->Delete();
    }
    m_datasetActors.clear();
}

//-----------------------------------------------------------------------------
vtkActor* VtkRenderWidget::createModel(int type, const double color[3],
                                       const double position[3])
{
    vtkPolyDataAlgorithm* source = nullptr;
    switch (type) {
    case 0: { auto* s = vtkCubeSource::New(); s->SetXLength(2.0); s->SetYLength(2.0); s->SetZLength(2.0); source = s; } break;
    case 1: { auto* s = vtkSphereSource::New(); s->SetRadius(1.2); s->SetThetaResolution(20); s->SetPhiResolution(20); source = s; } break;
    case 2: { auto* s = vtkCylinderSource::New(); s->SetHeight(2.4); s->SetRadius(0.8); s->SetResolution(20); source = s; } break;
    case 3: { auto* s = vtkConeSource::New(); s->SetHeight(2.4); s->SetRadius(1.0); s->SetResolution(20); source = s; } break;
    case 4: { auto* s = vtkPlaneSource::New(); s->SetXResolution(1); s->SetYResolution(1);
              s->SetOrigin(-1.2, -1.2, 0); s->SetPoint1(1.2, -1.2, 0); s->SetPoint2(-1.2, 1.2, 0); source = s; } break;
    default: return nullptr;
    }

    auto* mapper = vtkPolyDataMapper::New();
    mapper->SetInputConnection(source->GetOutputPort());

    double defaultColor[3] = { 0.5, 0.6, 0.8 };
    auto* actor = createActor(mapper, color ? color : defaultColor, position);

    mapper->Delete();
    source->Delete();

    // 模型对象不加入 m_actors，避免 clearAll（首次加载数据集）误删模型
    return actor;
}

//-----------------------------------------------------------------------------
void VtkRenderWidget::removeActor(vtkActor* actor)
{
    if (!actor) return;
    if (m_gizmoActor == actor) detachTransformGizmo();

    m_renderer->RemoveActor(actor);
    m_actors.erase(std::remove(m_actors.begin(), m_actors.end(), actor), m_actors.end());
    actor->Delete();
}

//-----------------------------------------------------------------------------
void VtkRenderWidget::setActorHighlighted(vtkActor* actor, bool highlighted)
{
    if (!actor) return;
    if (highlighted) {
        const QColor accent = ThemeManager::color(ColorRole::Accent);
        actor->GetProperty()->SetEdgeVisibility(1);
        actor->GetProperty()->SetEdgeColor(accent.redF(), accent.greenF(), accent.blueF());
        actor->GetProperty()->SetLineWidth(2.0);
    } else {
        actor->GetProperty()->SetEdgeVisibility(0);
    }
    if (m_vtkWidget) m_vtkWidget->renderWindow()->Render();
}

//-----------------------------------------------------------------------------
void VtkRenderWidget::attachTransformGizmo(vtkActor* actor)
{
    detachTransformGizmo();
    if (!actor) return;
    auto* iren = m_vtkWidget ? m_vtkWidget->renderWindow()->GetInteractor() : nullptr;
    if (!iren) return;

    m_gizmoActor = actor;
    m_boxWidget = vtkSmartPointer<vtkBoxWidget>::New();
    m_boxWidget->SetInteractor(iren);
    m_boxWidget->SetProp3D(actor);
    m_boxWidget->SetPlaceFactor(1.0);
    m_boxWidget->PlaceWidget(actor->GetBounds());
    m_boxWidget->SetTranslationEnabled(1);
    m_boxWidget->SetScalingEnabled(1);
    m_boxWidget->SetRotationEnabled(1);
    m_boxWidget->HandlesOn();
    m_boxWidget->AddObserver(vtkCommand::InteractionEvent, m_boxCB);
    m_boxWidget->AddObserver(vtkCommand::EndInteractionEvent, m_boxCB);
    m_boxWidget->SetEnabled(1);
    if (m_vtkWidget) m_vtkWidget->renderWindow()->Render();
}

//-----------------------------------------------------------------------------
void VtkRenderWidget::detachTransformGizmo()
{
    if (m_boxWidget) {
        m_boxWidget->SetEnabled(0);
        m_boxWidget = nullptr;
    }
    m_gizmoActor = nullptr;
    if (m_vtkWidget) m_vtkWidget->renderWindow()->Render();
}

//-----------------------------------------------------------------------------
void VtkRenderWidget::setActorTransform(vtkActor* actor,
    double px, double py, double pz,
    double rx, double ry, double rz,
    double sx, double sy, double sz)
{
    if (!actor) return;
    // 清除 gizmo 阶段可能残留的 user transform，统一用位置/旋转/缩放
    actor->SetUserTransform(nullptr);
    actor->SetPosition(px, py, pz);
    actor->SetOrientation(rx, ry, rz);
    actor->SetScale(sx, sy, sz);

    if (m_gizmoActor == actor && m_boxWidget) {
        m_boxWidget->PlaceWidget(actor->GetBounds());
    }
    if (m_vtkWidget) m_vtkWidget->renderWindow()->Render();
}

//-----------------------------------------------------------------------------
// 静态回调：左键单击拾取 / 绘制取点 / 右键取消
void VtkRenderWidget::OnPickCallback(vtkObject* caller, unsigned long eid,
                                     void* clientData, void*)
{
    auto* self = static_cast<VtkRenderWidget*>(clientData);
    auto* iren = static_cast<vtkRenderWindowInteractor*>(caller);
    if (!self || !iren) return;

    if (eid == vtkCommand::LeftButtonPressEvent) {
        self->m_pressX = iren->GetEventPosition()[0];
        self->m_pressY = iren->GetEventPosition()[1];
    } else if (eid == vtkCommand::LeftButtonReleaseEvent) {
        const int* xy = iren->GetEventPosition();
        const int dx = xy[0] - self->m_pressX;
        const int dy = xy[1] - self->m_pressY;
        if (dx * dx + dy * dy <= 25) {   // 按下→松开移动<5px 视为单击
            if (self->m_drawTool != DrawTool::None) {
                // 绘制模式：把点击投影到 z=0 平面喂给状态机
                self->advanceDraw(self->worldPointOnPlane(xy[0], xy[1]));
            } else if (vtkRenderer* ren = iren->FindPokedRenderer(xy[0], xy[1])) {
                self->m_picker->Pick(xy[0], xy[1], 0, ren);
                emit self->actorSelected(self->m_picker->GetActor());
            }
        }
    } else if (eid == vtkCommand::RightButtonPressEvent) {
        // 右键：取消当前绘制
        if (self->m_drawTool != DrawTool::None) self->resetDraw();
    }
}

//-----------------------------------------------------------------------------
// 静态回调：鼠标移动 -> 更新预览
void VtkRenderWidget::OnMoveCallback(vtkObject* caller, unsigned long,
                                     void* clientData, void*)
{
    auto* self = static_cast<VtkRenderWidget*>(clientData);
    auto* iren = static_cast<vtkRenderWindowInteractor*>(caller);
    if (!self || !iren || self->m_drawTool == DrawTool::None) return;
    const int* xy = iren->GetEventPosition();
    self->m_cursorPt = self->worldPointOnPlane(xy[0], xy[1]);
    self->updatePreview();
}

//-----------------------------------------------------------------------------
// 静态回调：键盘（Enter 完成多段线，Esc 退出绘制）
void VtkRenderWidget::OnKeyCallback(vtkObject* caller, unsigned long,
                                    void* clientData, void*)
{
    auto* self = static_cast<VtkRenderWidget*>(clientData);
    auto* iren = static_cast<vtkRenderWindowInteractor*>(caller);
    if (!self || !iren || self->m_drawTool == DrawTool::None) return;
    const char* key = iren->GetKeySym();
    if (!key) return;
    if (std::strcmp(key, "Return") == 0) {
        self->finishPolyline();
    } else if (std::strcmp(key, "Escape") == 0) {
        self->setDrawTool(DrawTool::None);
    }
}

//-----------------------------------------------------------------------------
// 静态回调：gizmo 变换
void VtkRenderWidget::OnBoxCallback(vtkObject* caller, unsigned long eid,
                                    void* clientData, void*)
{
    auto* self = static_cast<VtkRenderWidget*>(clientData);
    auto* box = static_cast<vtkBoxWidget*>(caller);
    if (!self || !box || !self->m_gizmoActor) return;

    if (eid == vtkCommand::InteractionEvent) {
        // 拖拽中：用 user transform 让 actor 跟随（视觉实时）
        vtkNew<vtkTransform> t;
        box->GetTransform(t);
        self->m_gizmoActor->SetUserTransform(t);
    } else if (eid == vtkCommand::EndInteractionEvent) {
        // 结束：把变换烘焙进 actor 的位置/旋转/缩放，便于属性面板读取
        vtkNew<vtkTransform> t;
        box->GetTransform(t);
        double p[3], o[3], s[3];
        t->GetPosition(p);
        t->GetOrientation(o);
        t->GetScale(s);
        self->m_gizmoActor->SetUserTransform(nullptr);
        self->m_gizmoActor->SetPosition(p);
        self->m_gizmoActor->SetOrientation(o);
        self->m_gizmoActor->SetScale(s);
        // 重放 gizmo 到新 bounds
        box->PlaceWidget(self->m_gizmoActor->GetBounds());
        emit self->transformChanged(self->m_gizmoActor);
    }
    if (self->m_vtkWidget) self->m_vtkWidget->renderWindow()->Render();
}

//-----------------------------------------------------------------------------
void VtkRenderWidget::setDrawTool(DrawTool tool)
{
    resetDraw();
    clearPreview();
    m_drawTool = tool;
    // 绘制模式下隐藏 gizmo
    if (m_drawTool != DrawTool::None && m_gizmoActor) {
        detachTransformGizmo();
    }
    emit drawToolChanged(m_drawTool);
}

//-----------------------------------------------------------------------------
void VtkRenderWidget::resetDraw()
{
    m_drawPts.clear();
    clearPreview();
}

//-----------------------------------------------------------------------------
void VtkRenderWidget::clearPreview()
{
    if (m_previewActor) {
        m_renderer->RemoveActor(m_previewActor);
        m_previewActor->Delete();
        m_previewActor = nullptr;
    }
    if (m_previewMapper) {
        m_previewMapper->Delete();
        m_previewMapper = nullptr;
    }
}

//-----------------------------------------------------------------------------
void VtkRenderWidget::clearSketches()
{
    for (vtkActor* a : m_sketchActors) {
        m_renderer->RemoveActor(a);
        a->Delete();
    }
    m_sketchActors.clear();
    clearPreview();
}

//-----------------------------------------------------------------------------
std::array<double, 3> VtkRenderWidget::worldPointOnPlane(int x, int y)
{
    std::array<double, 3> res = { 0.0, 0.0, 0.0 };
    if (!m_renderer) return res;

    // 显示坐标 -> 两个深度求射线，再与 z=0 平面求交
    double w[4], d[3];
    d[0] = double(x); d[1] = double(y); d[2] = 0.0;
    m_renderer->SetDisplayPoint(d);
    m_renderer->DisplayToWorld();
    m_renderer->GetWorldPoint(w);
    double w1[3] = { w[0] / w[3], w[1] / w[3], w[2] / w[3] };

    d[2] = 1.0;
    m_renderer->SetDisplayPoint(d);
    m_renderer->DisplayToWorld();
    m_renderer->GetWorldPoint(w);
    double w2[3] = { w[0] / w[3], w[1] / w[3], w[2] / w[3] };

    const double dz = w2[2] - w1[2];
    if (std::fabs(dz) < 1e-9) {
        res[0] = w1[0]; res[1] = w1[1];
        return res;
    }
    const double t = -w1[2] / dz;
    res[0] = w1[0] + t * (w2[0] - w1[0]);
    res[1] = w1[1] + t * (w2[1] - w1[1]);
    return res;
}

//-----------------------------------------------------------------------------
vtkActor* VtkRenderWidget::makePolylineActor(
    const std::vector<std::array<double, 3>>& pts, const double color[3], double width)
{
    vtkNew<vtkPoints> points;
    points->SetNumberOfPoints(int(pts.size()));
    for (int i = 0; i < int(pts.size()); ++i) points->SetPoint(i, pts[i].data());

    vtkNew<vtkPolyLine> pl;
    pl->GetPointIds()->SetNumberOfIds(int(pts.size()));
    for (int i = 0; i < int(pts.size()); ++i) pl->GetPointIds()->SetId(i, i);

    vtkNew<vtkCellArray> cells;
    cells->InsertNextCell(pl);

    vtkNew<vtkPolyData> poly;
    poly->SetPoints(points);
    poly->SetLines(cells);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(poly);

    auto* actor = vtkActor::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(color[0], color[1], color[2]);
    actor->GetProperty()->SetLineWidth(width);
    m_renderer->AddActor(actor);
    return actor;
}

//-----------------------------------------------------------------------------
std::vector<std::array<double, 3>> VtkRenderWidget::circlePoints(
    const std::array<double, 3>& c, double r, int n)
{
    std::vector<std::array<double, 3>> pts;
    pts.reserve(n);
    for (int i = 0; i < n; ++i) {
        const double a = 2.0 * kPI * i / n;
        pts.push_back({ c[0] + r * std::cos(a), c[1] + r * std::sin(a), 0.0 });
    }
    return pts;
}

//-----------------------------------------------------------------------------
std::vector<std::array<double, 3>> VtkRenderWidget::arcPoints(
    const std::array<double, 3>& p1, const std::array<double, 3>& p2,
    const std::array<double, 3>& p3, int n)
{
    // 三点外心（在 z=0 平面）
    const double ax = p1[0], ay = p1[1], bx = p2[0], by = p2[1], cx = p3[0], cy = p3[1];
    const double det = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
    std::array<double, 3> center;
    if (std::fabs(det) < 1e-9) {
        center = { (ax + cx) / 2.0, (ay + cy) / 2.0, 0.0 };
    } else {
        const double ux = ((ax*ax + ay*ay) * (by - cy) + (bx*bx + by*by) * (cy - ay) + (cx*cx + cy*cy) * (ay - by)) / det;
        const double uy = ((ax*ax + ay*ay) * (cx - bx) + (bx*bx + by*by) * (ax - cx) + (cx*cx + cy*cy) * (bx - ax)) / det;
        center = { ux, uy, 0.0 };
    }
    const double r = std::hypot(center[0] - p1[0], center[1] - p1[1]);
    const double a1 = std::atan2(p1[1] - center[1], p1[0] - center[0]);
    const double a2 = std::atan2(p2[1] - center[1], p2[0] - center[0]);
    const double a3 = std::atan2(p3[1] - center[1], p3[0] - center[0]);

    // 从 a1 出发，选择经过 a2 的弧方向到达 a3
    const double db = std::fmod(a2 - a1 + 2.0 * kPI, 2.0 * kPI);   // [0,2π)
    const double da = std::fmod(a3 - a1 + 2.0 * kPI, 2.0 * kPI);   // [0,2π)
    const double sweep = (db > 1e-6 && db < da) ? da : da - 2.0 * kPI;

    const int m = std::max(2, n);
    std::vector<std::array<double, 3>> pts;
    pts.reserve(m + 1);
    for (int i = 0; i <= m; ++i) {
        const double a = a1 + sweep * i / m;
        pts.push_back({ center[0] + r * std::cos(a), center[1] + r * std::sin(a), 0.0 });
    }
    return pts;
}

//-----------------------------------------------------------------------------
vtkActor* VtkRenderWidget::addSketchLine(const std::array<double, 3>& p1,
                                         const std::array<double, 3>& p2)
{
    static const double color[3] = { 0.25, 0.78, 0.92 };
    std::vector<std::array<double, 3>> pts = { p1, p2 };
    vtkActor* a = makePolylineActor(pts, color, 2.0);
    m_sketchActors.push_back(a);
    return a;
}

//-----------------------------------------------------------------------------
vtkActor* VtkRenderWidget::addSketchCircle(const std::array<double, 3>& c, double radius)
{
    static const double color[3] = { 0.25, 0.78, 0.92 };
    vtkActor* a = makePolylineActor(circlePoints(c, radius, 64), color, 2.0);
    m_sketchActors.push_back(a);
    return a;
}

//-----------------------------------------------------------------------------
vtkActor* VtkRenderWidget::addSketchArc(const std::array<double, 3>& p1,
                                        const std::array<double, 3>& p2,
                                        const std::array<double, 3>& p3)
{
    static const double color[3] = { 0.25, 0.78, 0.92 };
    vtkActor* a = makePolylineActor(arcPoints(p1, p2, p3, 64), color, 2.0);
    m_sketchActors.push_back(a);
    return a;
}

//-----------------------------------------------------------------------------
void VtkRenderWidget::advanceDraw(const std::array<double, 3>& p)
{
    if (m_drawTool == DrawTool::None) return;

    switch (m_drawTool) {
    case DrawTool::Line:
        if (m_drawPts.empty()) {
            m_drawPts.push_back(p);
        } else {
            addSketchLine(m_drawPts[0], p);
            resetDraw();
        }
        break;
    case DrawTool::Circle:
        if (m_drawPts.empty()) {
            m_drawPts.push_back(p);
        } else {
            const double r = std::hypot(p[0] - m_drawPts[0][0], p[1] - m_drawPts[0][1]);
            addSketchCircle(m_drawPts[0], r);
            resetDraw();
        }
        break;
    case DrawTool::Arc:
        m_drawPts.push_back(p);
        if (m_drawPts.size() >= 3) {
            addSketchArc(m_drawPts[0], m_drawPts[1], m_drawPts[2]);
            resetDraw();
        }
        break;
    case DrawTool::Polyline:
        m_drawPts.push_back(p);
        break;
    default:
        break;
    }
    updatePreview();
}

//-----------------------------------------------------------------------------
void VtkRenderWidget::finishPolyline()
{
    if (m_drawTool != DrawTool::Polyline) return;
    if (m_drawPts.size() >= 2) {
        static const double color[3] = { 0.25, 0.78, 0.92 };
        vtkActor* a = makePolylineActor(m_drawPts, color, 2.0);
        m_sketchActors.push_back(a);
    }
    resetDraw();
}

//-----------------------------------------------------------------------------
void VtkRenderWidget::updatePreview()
{
    if (m_drawTool == DrawTool::None || m_drawPts.empty()) {
        clearPreview();
        return;
    }

    std::vector<std::array<double, 3>> pts;
    switch (m_drawTool) {
    case DrawTool::Line:
        pts = { m_drawPts[0], m_cursorPt };
        break;
    case DrawTool::Circle: {
        const double r = std::hypot(m_cursorPt[0] - m_drawPts[0][0],
                                    m_cursorPt[1] - m_drawPts[0][1]);
        pts = circlePoints(m_drawPts[0], r, 64);
        break;
    }
    case DrawTool::Arc:
        if (m_drawPts.size() >= 2) pts = arcPoints(m_drawPts[0], m_drawPts[1], m_cursorPt, 64);
        else pts = { m_drawPts[0], m_cursorPt };
        break;
    case DrawTool::Polyline:
        pts = m_drawPts;
        if (!pts.empty()) pts.push_back(m_cursorPt);
        break;
    default:
        break;
    }
    if (pts.size() < 2) return;

    vtkNew<vtkPoints> points;
    points->SetNumberOfPoints(int(pts.size()));
    for (int i = 0; i < int(pts.size()); ++i) points->SetPoint(i, pts[i].data());
    vtkNew<vtkPolyLine> pl;
    pl->GetPointIds()->SetNumberOfIds(int(pts.size()));
    for (int i = 0; i < int(pts.size()); ++i) pl->GetPointIds()->SetId(i, i);
    vtkNew<vtkCellArray> cells;
    cells->InsertNextCell(pl);
    vtkNew<vtkPolyData> poly;
    poly->SetPoints(points);
    poly->SetLines(cells);

    if (!m_previewActor) {
        m_previewMapper = vtkPolyDataMapper::New();
        m_previewActor = vtkActor::New();
        m_previewActor->SetMapper(m_previewMapper);
        m_previewActor->GetProperty()->SetColor(0.95, 0.85, 0.2);
        m_previewActor->GetProperty()->SetLineWidth(2.0);
        m_previewActor->GetProperty()->SetOpacity(0.85);
        m_renderer->AddActor(m_previewActor);
    }
    m_previewMapper->SetInputData(poly);
    if (m_vtkWidget) m_vtkWidget->renderWindow()->Render();
}

void VtkRenderWidget::resetCamera()
{
    if (m_renderer)
    {
        m_renderer->ResetCamera();
        m_renderer->GetActiveCamera()->Zoom(1.2);
        m_vtkWidget->renderWindow()->Render();
    }
}

vtkRenderWindow* VtkRenderWidget::getRenderWindow()
{
    return m_vtkWidget ? m_vtkWidget->renderWindow() : nullptr;
}

vtkRenderer* VtkRenderWidget::getRenderer()
{
    return m_renderer;
}

void VtkRenderWidget::showAxes(bool show)
{
    Q_UNUSED(show);
    // Orientation marker widget would be implemented here
}

void VtkRenderWidget::setInteractionMode(int mode)
{
    Q_UNUSED(mode);
}
