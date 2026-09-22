#pragma once
#include <QWidget>

class QLineEdit;
class QLabel;
class QDoubleSpinBox;

/**
 * @brief 模型对象属性编辑器（名称 + 位置/旋转/缩放）
 *
 * 用于右侧"属性"停靠区，选中模型对象时显示并精确编辑其变换。
 * 位置/旋转单位为度。setValues 填充期间抑制 edited 信号，避免双向同步回环。
 */
class ModelProperties : public QWidget
{
    Q_OBJECT

public:
    explicit ModelProperties(QWidget* parent = nullptr);

    /// 填充数据（name/type + 变换值）
    void setValues(const QString& name, const QString& type,
                   double px, double py, double pz,
                   double rx, double ry, double rz,
                   double sx, double sy, double sz);

    QString name() const;
    double posX() const;  double posY() const;  double posZ() const;
    double rotX() const;  double rotY() const;  double rotZ() const;
    double scaleX() const; double scaleY() const; double scaleZ() const;

signals:
    /// 用户编辑了任一字段（名称/位置/旋转/缩放）
    void edited();

private:
    void setupUI();

    QLineEdit*      m_nameEdit  = nullptr;
    QLabel*         m_typeLabel = nullptr;
    QDoubleSpinBox* m_posX = nullptr; QDoubleSpinBox* m_posY = nullptr; QDoubleSpinBox* m_posZ = nullptr;
    QDoubleSpinBox* m_rotX = nullptr; QDoubleSpinBox* m_rotY = nullptr; QDoubleSpinBox* m_rotZ = nullptr;
    QDoubleSpinBox* m_scaleX = nullptr; QDoubleSpinBox* m_scaleY = nullptr; QDoubleSpinBox* m_scaleZ = nullptr;
    bool m_suppress = false;
};
