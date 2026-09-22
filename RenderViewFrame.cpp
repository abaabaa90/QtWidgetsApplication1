#include "RenderViewFrame.h"
#include "VtkRenderWidget.h"
#include "StyleManager.h"
#include "ThemeManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QMouseEvent>

int RenderViewFrame::s_frameCounter = 0;

//-----------------------------------------------------------------------------
RenderViewFrame::RenderViewFrame(const QString& title, QWidget* parent)
    : QWidget(parent)
    , m_frameId(++s_frameCounter)
{
    setupUI();
    setupConnections();
}

//-----------------------------------------------------------------------------
RenderViewFrame::~RenderViewFrame() = default;

//-----------------------------------------------------------------------------
void RenderViewFrame::setupUI()
{
    int unit = StyleManager::baseUnit();
    int titleBarHeight = static_cast<int>(unit * 0.58);
    int btnSize = static_cast<int>(unit * 0.40);

    // --- Outer layout ---
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // --- Title bar ---
    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("renderViewTitleBar");
    m_titleBar->setFixedHeight(titleBarHeight);

    auto* titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(static_cast<int>(unit * 0.12), 0,
                                     static_cast<int>(unit * 0.06), 0);
    titleLayout->setSpacing(static_cast<int>(unit * 0.06));

    m_titleLabel = new QLabel(QString("RenderView%1").arg(m_frameId), m_titleBar);
    m_titleLabel->setObjectName("renderViewTitleLabel");

    m_splitHBtn = new QPushButton("H", m_titleBar);
    m_splitVBtn = new QPushButton("V", m_titleBar);
    m_maximizeBtn = new QPushButton(QStringLiteral("□"), m_titleBar); // White square
    m_closeBtn = new QPushButton(QStringLiteral("×"), m_titleBar);   // Multiplication sign

    m_splitHBtn->setObjectName("renderViewBtn");
    m_splitVBtn->setObjectName("renderViewBtn");
    m_maximizeBtn->setObjectName("renderViewBtn");
    m_closeBtn->setObjectName("renderViewCloseBtn");

    m_splitHBtn->setFixedSize(btnSize, btnSize);
    m_splitVBtn->setFixedSize(btnSize, btnSize);
    m_maximizeBtn->setFixedSize(btnSize, btnSize);
    m_closeBtn->setFixedSize(btnSize, btnSize);

    m_splitHBtn->setToolTip(QStringLiteral("Split horizontally"));
    m_splitVBtn->setToolTip(QStringLiteral("Split vertically"));
    m_maximizeBtn->setToolTip(QStringLiteral("Maximize view"));
    m_closeBtn->setToolTip(QStringLiteral("Close view"));

    titleLayout->addWidget(m_titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(m_splitHBtn);
    titleLayout->addWidget(m_splitVBtn);
    titleLayout->addWidget(m_maximizeBtn);
    titleLayout->addWidget(m_closeBtn);

    outerLayout->addWidget(m_titleBar);

    // --- Content area ---
    m_contentArea = new QWidget(this);
    m_contentArea->setObjectName("renderViewContentArea");
    m_contentArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* contentLayout = new QVBoxLayout(m_contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_vtkWidget = new VtkRenderWidget(m_contentArea);
    contentLayout->addWidget(m_vtkWidget);

    outerLayout->addWidget(m_contentArea, 1);
}

//-----------------------------------------------------------------------------
void RenderViewFrame::setupConnections()
{
    connect(m_splitHBtn, &QPushButton::clicked,
            this, &RenderViewFrame::splitHorizontalRequested);
    connect(m_splitVBtn, &QPushButton::clicked,
            this, &RenderViewFrame::splitVerticalRequested);
    connect(m_closeBtn, &QPushButton::clicked,
            this, &RenderViewFrame::closeRequested);

    connect(m_maximizeBtn, &QPushButton::clicked, this, [this]() {
        if (m_maximized) {
            emit restoreRequested();
        } else {
            emit maximizeRequested();
        }
    });

    // 主题切换时重绘边框（颜色从 ThemeManager 实时读取）
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this](Theme) { update(); });
}

//-----------------------------------------------------------------------------
void RenderViewFrame::setViewTitle(const QString& title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
}

//-----------------------------------------------------------------------------
QString RenderViewFrame::viewTitle() const
{
    return m_titleLabel ? m_titleLabel->text() : QString();
}

//-----------------------------------------------------------------------------
void RenderViewFrame::setActive(bool active)
{
    if (m_active != active) {
        m_active = active;
        update(); // trigger paintEvent to redraw border
    }
}

//-----------------------------------------------------------------------------
bool RenderViewFrame::isActive() const
{
    return m_active;
}

//-----------------------------------------------------------------------------
void RenderViewFrame::setMaximized(bool maximized)
{
    m_maximized = maximized;
    // Toggle the maximize button text
    m_maximizeBtn->setText(maximized ? QStringLiteral("▢")   // Rounded square (restore)
                                     : QStringLiteral("□")); // White square (maximize)
    m_maximizeBtn->setToolTip(maximized ? QStringLiteral("Restore view")
                                        : QStringLiteral("Maximize view"));
}

//-----------------------------------------------------------------------------
bool RenderViewFrame::isMaximized() const
{
    return m_maximized;
}

//-----------------------------------------------------------------------------
VtkRenderWidget* RenderViewFrame::vtkRenderWidget() const
{
    return m_vtkWidget;
}

//-----------------------------------------------------------------------------
void RenderViewFrame::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    if (m_active) {
        painter.setPen(QPen(ThemeManager::color(ColorRole::Accent), 2));
    } else {
        painter.setPen(QPen(ThemeManager::color(ColorRole::Border), 1));
    }

    // Draw border just inside the widget rect
    QRect r = rect().adjusted(0, 0, -1, -1);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(r);
}

//-----------------------------------------------------------------------------
void RenderViewFrame::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit activated();
    }
    QWidget::mousePressEvent(event);
}
