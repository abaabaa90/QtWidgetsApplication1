#ifndef VTKRENDERWIDGET_H
#define VTKRENDERWIDGET_H

#include <QWidget>
#include <QVTKOpenGLNativeWidget.h>

// VTK前向声明（减少编译依赖）
class vtkConeSource;
class vtkSphereSource;
class vtkCylinderSource;
class vtkPolyDataMapper;
class vtkActor;
class vtkRenderer;
class vtkRenderWindow;
class vtkCamera;
class vtkLight;

/**
 * @brief 可重用的VTK渲染部件
 * 封装了VTK渲染功能，可以在任何Qt窗口中使用
 */
class VtkRenderWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父部件
     */
    explicit VtkRenderWidget(QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~VtkRenderWidget();

    /**
     * @brief 添加圆锥体到场景
     * @param height 高度
     * @param radius 半径
     * @param resolution 分辨率
     * @param color RGB颜色，范围0-1
     * @param position 位置坐标
     */
    void addCone(double height = 3.0, double radius = 1.0, int resolution = 20,
        const double color[3] = nullptr, const double position[3] = nullptr);

    /**
     * @brief 添加球体到场景
     * @param radius 半径
     * @param thetaResolution 经度分辨率
     * @param phiResolution 纬度分辨率
     * @param color RGB颜色，范围0-1
     * @param position 位置坐标
     */
    void addSphere(double radius = 1.0, int thetaResolution = 20, int phiResolution = 20,
        const double color[3] = nullptr, const double position[3] = nullptr);

    /**
     * @brief 添加圆柱体到场景
     * @param height 高度
     * @param radius 半径
     * @param resolution 分辨率
     * @param color RGB颜色，范围0-1
     * @param position 位置坐标
     */
    void addCylinder(double height = 3.0, double radius = 1.0, int resolution = 20,
        const double color[3] = nullptr, const double position[3] = nullptr);

    /**
     * @brief 设置背景颜色
     * @param r 红色分量，范围0-1
     * @param g 绿色分量，范围0-1
     * @param b 蓝色分量，范围0-1
     */
    void setBackgroundColor(double r, double g, double b);

    /**
     * @brief 重置相机到默认视图
     */
    void resetCamera();

    /**
     * @brief 获取VTK渲染窗口（用于高级操作）
     * @return VTK渲染窗口指针
     */
    vtkRenderWindow* getRenderWindow();

    /**
     * @brief 获取VTK渲染器（用于高级操作）
     * @return VTK渲染器指针
     */
    vtkRenderer* getRenderer();

    /**
     * @brief 显示坐标轴
     * @param show 是否显示
     */
    void showAxes(bool show);

    /**
     * @brief 设置交互模式
     * @param mode 0=旋转，1=平移，2=缩放
     */
    void setInteractionMode(int mode);

signals:
    /**
     * @brief 物体被选中的信号
     * @param actor 被选中的actor
     */
    void actorSelected(vtkActor* actor);

    /**
     * @brief 渲染完成信号
     */
    void renderingFinished();

protected:
    /**
     * @brief 初始化VTK环境
     */
    void initializeVtk();

    /**
     * @brief 创建默认场景
     */
    void createDefaultScene();

    /**
     * @brief 添加光源
     */
    void addLights();

private:
    QVTKOpenGLNativeWidget* m_vtkWidget;  // VTK OpenGL部件
    vtkRenderer* m_renderer;               // VTK渲染器
    std::vector<vtkActor*> m_actors;       // 存储所有actor便于管理
    bool m_initialized;                    // 初始化标志

    // 私有辅助方法
    vtkActor* createActor(vtkPolyDataMapper* mapper, const double color[3], const double position[3]);
};

#endif // VTKRENDERWIDGET_H