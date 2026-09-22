#include "ModelProperties.h"

#include <QLineEdit>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------
ModelProperties::ModelProperties(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("modelProps"));
    setupUI();
}

//-----------------------------------------------------------------------------
void ModelProperties::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(4);

    // 名称 / 类型
    auto* nameRow = new QHBoxLayout;
    nameRow->addWidget(new QLabel(QStringLiteral("名称"), this));
    m_nameEdit = new QLineEdit(this);
    nameRow->addWidget(m_nameEdit, 1);
    layout->addLayout(nameRow);

    m_typeLabel = new QLabel(this);
    m_typeLabel->setObjectName(QStringLiteral("modelPropsType"));
    layout->addWidget(m_typeLabel);

    auto makeSpin = [this](double min, double max, double step, double val) {
        auto* s = new QDoubleSpinBox(this);
        s->setRange(min, max);
        s->setSingleStep(step);
        s->setDecimals(2);
        s->setValue(val);
        return s;
    };

    // 位置
    auto* posRow = new QHBoxLayout;
    posRow->addWidget(new QLabel(QStringLiteral("位置"), this));
    m_posX = makeSpin(-9999, 9999, 0.1, 0);
    m_posY = makeSpin(-9999, 9999, 0.1, 0);
    m_posZ = makeSpin(-9999, 9999, 0.1, 0);
    for (QDoubleSpinBox* s : { m_posX, m_posY, m_posZ }) posRow->addWidget(s);
    layout->addLayout(posRow);

    // 旋转（度）
    auto* rotRow = new QHBoxLayout;
    rotRow->addWidget(new QLabel(QStringLiteral("旋转"), this));
    m_rotX = makeSpin(-3600, 3600, 1.0, 0);
    m_rotY = makeSpin(-3600, 3600, 1.0, 0);
    m_rotZ = makeSpin(-3600, 3600, 1.0, 0);
    for (QDoubleSpinBox* s : { m_rotX, m_rotY, m_rotZ }) rotRow->addWidget(s);
    layout->addLayout(rotRow);

    // 缩放
    auto* scaleRow = new QHBoxLayout;
    scaleRow->addWidget(new QLabel(QStringLiteral("缩放"), this));
    m_scaleX = makeSpin(0.01, 1000, 0.05, 1);
    m_scaleY = makeSpin(0.01, 1000, 0.05, 1);
    m_scaleZ = makeSpin(0.01, 1000, 0.05, 1);
    for (QDoubleSpinBox* s : { m_scaleX, m_scaleY, m_scaleZ }) scaleRow->addWidget(s);
    layout->addLayout(scaleRow);

    layout->addStretch();

    // 任一字段变化 -> edited
    connect(m_nameEdit, &QLineEdit::textChanged, this, [this]() {
        if (!m_suppress) emit edited();
    });
    auto hook = [this](QDoubleSpinBox* s) {
        connect(s, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double) {
            if (!m_suppress) emit edited();
        });
    };
    for (QDoubleSpinBox* s : { m_posX, m_posY, m_posZ, m_rotX, m_rotY, m_rotZ, m_scaleX, m_scaleY, m_scaleZ }) {
        hook(s);
    }
}

//-----------------------------------------------------------------------------
void ModelProperties::setValues(const QString& name, const QString& type,
                                double px, double py, double pz,
                                double rx, double ry, double rz,
                                double sx, double sy, double sz)
{
    m_suppress = true;
    m_nameEdit->setText(name);
    m_typeLabel->setText(type);
    m_posX->setValue(px);  m_posY->setValue(py);  m_posZ->setValue(pz);
    m_rotX->setValue(rx);  m_rotY->setValue(ry);  m_rotZ->setValue(rz);
    m_scaleX->setValue(sx); m_scaleY->setValue(sy); m_scaleZ->setValue(sz);
    m_suppress = false;
}

//-----------------------------------------------------------------------------
QString ModelProperties::name() const { return m_nameEdit ? m_nameEdit->text() : QString(); }
double ModelProperties::posX() const { return m_posX->value(); }
double ModelProperties::posY() const { return m_posY->value(); }
double ModelProperties::posZ() const { return m_posZ->value(); }
double ModelProperties::rotX() const { return m_rotX->value(); }
double ModelProperties::rotY() const { return m_rotY->value(); }
double ModelProperties::rotZ() const { return m_rotZ->value(); }
double ModelProperties::scaleX() const { return m_scaleX->value(); }
double ModelProperties::scaleY() const { return m_scaleY->value(); }
double ModelProperties::scaleZ() const { return m_scaleZ->value(); }
