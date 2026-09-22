#ifndef VTKRENDERWIDGET_H
#define VTKRENDERWIDGET_H

#include <QWidget>
#include <QHash>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>
#include <array>
#include <vector>

// 主题枚举前向声明（定义在 ThemeManager.h）
enum class Theme;

// VTKǰ�����������ٱ���������
class vtkConeSource;
class vtkSphereSource;
class vtkCylinderSource;
class vtkPolyDataMapper;
class vtkActor;
class vtkRenderer;
class vtkRenderWindow;
class vtkCamera;
class vtkLight;
class vtkCellPicker;
class vtkBoxWidget;
class vtkCallbackCommand;
class vtkObject;

/// 交互式绘制工具
enum class DrawTool {
    None,
    Line,       // 直线：两点
    Polyline,   // 多段线：多点，Enter 完成
    Circle,     // 圆：圆心 + 半径点
    Arc,        // 圆弧：三点
};

/**
 * @brief �����õ�VTK��Ⱦ����
 * ��װ��VTK��Ⱦ���ܣ��������κ�Qt������ʹ��
 */
class VtkRenderWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief ���캯��
     * @param parent ������
     */
    explicit VtkRenderWidget(QWidget* parent = nullptr);

    /**
     * @brief ��������
     */
    ~VtkRenderWidget();

    /**
     * @brief ���Բ׶�嵽����
     * @param height �߶�
     * @param radius �뾶
     * @param resolution �ֱ���
     * @param color RGB��ɫ����Χ0-1
     * @param position λ������
     */
    void addCone(double height = 3.0, double radius = 1.0, int resolution = 20,
        const double color[3] = nullptr, const double position[3] = nullptr);

    /**
     * @brief ������嵽����
     * @param radius �뾶
     * @param thetaResolution ���ȷֱ���
     * @param phiResolution γ�ȷֱ���
     * @param color RGB��ɫ����Χ0-1
     * @param position λ������
     */
    void addSphere(double radius = 1.0, int thetaResolution = 20, int phiResolution = 20,
        const double color[3] = nullptr, const double position[3] = nullptr);

    /**
     * @brief ���Բ���嵽����
     * @param height �߶�
     * @param radius �뾶
     * @param resolution �ֱ���
     * @param color RGB��ɫ����Χ0-1
     * @param position λ������
     */
    void addCylinder(double height = 3.0, double radius = 1.0, int resolution = 20,
        const double color[3] = nullptr, const double position[3] = nullptr);

    /**
     * @brief 添加正方体到场景
     */
    void addCube(double xLength = 2.0, double yLength = 2.0, double zLength = 2.0,
        const double color[3] = nullptr, const double position[3] = nullptr);

    /**
     * @brief 添加平面（XY 平面矩形）到场景
     */
    void addPlane(double width = 2.0, double depth = 2.0,
        const double color[3] = nullptr, const double position[3] = nullptr);

    /**
     * @brief ��ȥ��ǰ���е�actor�����ڼ���������ǰ������������
     */
    void clearAll();

    /**
     * @brief ���㼯��Ϊ����������ӵ�����������ǰ 3 ������Ϊ x/y/z���������䣩
     * @param points ÿ�е�ǰ 3 ��ֵ��Ϊ����
     * @param color RGB��ɫ����Χ0-1������ΪĬ��ɫ
     * @return �½�����actor
     */
    vtkActor* addPointCloud(const std::vector<std::vector<double>>& points,
        const double color[3] = nullptr);

    /**
     * @brief 以数据集为单位添加一个点云（支持多个数据集同屏显示）
     * @param points 每行前 3 个值作为坐标
     * @param color RGB 颜色（范围 0-1），空则用内置调色板
     * @return 数据集 id（后续 removeDataset/setDatasetVisible 用）
     */
    int addDataset(const std::vector<std::vector<double>>& points,
                   const double color[3] = nullptr);
    /// 移除一个数据集 actor
    void removeDataset(int id);
    /// 显示/隐藏一个数据集 actor
    void setDatasetVisible(int id, bool visible);
    /// 移除全部数据集 actor（保留默认场景）
    void clearDatasets();

    /**
     * @brief 创建模型对象（0=正方体 1=球 2=圆柱 3=圆锥 4=平面），返回 actor
     */
    vtkActor* createModel(int type, const double color[3] = nullptr,
                          const double position[3] = nullptr);
    /// 从场景移除并删除一个 actor
    void removeActor(vtkActor* actor);
    /// 高亮/取消高亮一个 actor（边缘描边）
    void setActorHighlighted(vtkActor* actor, bool highlighted);
    /// 为指定 actor 挂载 3D 变换 gizmo（vtkBoxWidget）
    void attachTransformGizmo(vtkActor* actor);
    /// 卸载变换 gizmo
    void detachTransformGizmo();
    /// 直接设置 actor 的位置/旋转(度)/缩放并重放 gizmo
    void setActorTransform(vtkActor* actor,
        double px, double py, double pz,
        double rx, double ry, double rz,
        double sx, double sy, double sz);

    /// 激活/取消交互式绘制工具（直线/多段线/圆/圆弧），None=退出绘制
    void setDrawTool(DrawTool tool);
    /// 当前绘制工具
    DrawTool drawTool() const { return m_drawTool; }
    /// 清除所有已绘制的草图（直线/圆/圆弧/多段线）
    void clearSketches();

    /**
     * @brief ���ñ�����ɫ
     * @param r ��ɫ��������Χ0-1
     * @param g ��ɫ��������Χ0-1
     * @param b ��ɫ��������Χ0-1
     */
    void setBackgroundColor(double r, double g, double b);

    /**
     * @brief ���������Ĭ����ͼ
     */
    void resetCamera();

    /**
     * @brief ��ȡVTK��Ⱦ���ڣ����ڸ߼�������
     * @return VTK��Ⱦ����ָ��
     */
    vtkRenderWindow* getRenderWindow();

    /**
     * @brief ��ȡVTK��Ⱦ�������ڸ߼�������
     * @return VTK��Ⱦ��ָ��
     */
    vtkRenderer* getRenderer();

    /**
     * @brief ��ʾ������
     * @param show �Ƿ���ʾ
     */
    void showAxes(bool show);

    /**
     * @brief ���ý���ģʽ
     * @param mode 0=��ת��1=ƽ�ƣ�2=����
     */
    void setInteractionMode(int mode);

signals:
    /**
     * @brief ���屻ѡ�е��ź�
     * @param actor ��ѡ�е�actor（nullptr 表示点到空白）
     */
    void actorSelected(vtkActor* actor);

    /**
     * @brief ��Ⱦ����ź�
     */
    void renderingFinished();

    /// 变换 gizmo 交互结束，actor 变换已更新
    void transformChanged(vtkActor* actor);

    /// 绘制工具切换（含视图内 Esc 退出）
    void drawToolChanged(DrawTool tool);

protected:
    /**
     * @brief ��ʼ��VTK����
     */
    void initializeVtk();

    /**
     * @brief ����Ĭ�ϳ���
     */
    void createDefaultScene();

    /**
     * @brief ��ӹ�Դ
     */
    void addLights();

private:
    QVTKOpenGLNativeWidget* m_vtkWidget;  // VTK OpenGL����
    vtkRenderer* m_renderer;               // VTK��Ⱦ��
    std::vector<vtkActor*> m_actors;       // �洢����actor���ڹ���
    QHash<int, vtkActor*> m_datasetActors; // 数据集 id -> actor（多数据集同屏）
    int m_nextDatasetId = 1;               // 数据集 id 自增
    bool m_initialized;                    // ��ʼ����־

    // ˽�и�������
    vtkActor* createActor(vtkPolyDataMapper* mapper, const double color[3], const double position[3]);
    vtkActor* createPointCloudActor(const std::vector<std::vector<double>>& points,
                                    const double color[3]);

    // 建模：拾取 / gizmo 静态回调
    static void OnPickCallback(vtkObject* caller, unsigned long eid, void* clientData, void*);
    static void OnBoxCallback(vtkObject* caller, unsigned long eid, void* clientData, void*);
    static void OnMoveCallback(vtkObject* caller, unsigned long eid, void* clientData, void*);
    static void OnKeyCallback(vtkObject* caller, unsigned long eid, void* clientData, void*);

    // 交互式绘制
    void advanceDraw(const std::array<double, 3>& p);   // 喂一个确认点
    void finishPolyline();
    void resetDraw();
    void updatePreview();
    void clearPreview();
    std::array<double, 3> worldPointOnPlane(int x, int y);   // 点击 -> z=0 平面世界坐标
    vtkActor* makePolylineActor(const std::vector<std::array<double, 3>>& pts,
                                const double color[3], double width);
    vtkActor* addSketchLine(const std::array<double, 3>& p1, const std::array<double, 3>& p2);
    vtkActor* addSketchCircle(const std::array<double, 3>& c, double radius);
    vtkActor* addSketchArc(const std::array<double, 3>& p1, const std::array<double, 3>& p2,
                           const std::array<double, 3>& p3);
    static std::vector<std::array<double, 3>> circlePoints(const std::array<double, 3>& c,
                                                           double r, int n);
    static std::vector<std::array<double, 3>> arcPoints(const std::array<double, 3>& p1,
                                                        const std::array<double, 3>& p2,
                                                        const std::array<double, 3>& p3, int n);

    // 按主题设置渲染视口背景色
    void applyThemeBackground(Theme theme);

    vtkSmartPointer<vtkCellPicker>     m_picker;     // 点选拾取器
    vtkSmartPointer<vtkCallbackCommand> m_pickCB;    // 拾取观察者
    vtkSmartPointer<vtkCallbackCommand> m_moveCB;    // 鼠标移动观察者（预览）
    vtkSmartPointer<vtkCallbackCommand> m_keyCB;     // 键盘观察者（Enter/Esc）
    vtkSmartPointer<vtkBoxWidget>      m_boxWidget;  // 变换 gizmo
    vtkSmartPointer<vtkCallbackCommand> m_boxCB;     // gizmo 观察者
    vtkActor* m_gizmoActor = nullptr;                // 当前挂 gizmo 的 actor
    int m_pressX = -1;                               // 拾取按下点
    int m_pressY = -1;

    // 绘制状态
    DrawTool m_drawTool = DrawTool::None;
    std::vector<std::array<double, 3>> m_drawPts;    // 已确认点
    std::array<double, 3> m_cursorPt = { 0.0, 0.0, 0.0 };  // 光标世界点（预览用）
    vtkActor* m_previewActor  = nullptr;             // 橡皮筋预览
    vtkPolyDataMapper* m_previewMapper = nullptr;
    std::vector<vtkActor*> m_sketchActors;           // 已绘制的草图
};

#endif // VTKRENDERWIDGET_H