#include "BaseWindow.h"

BaseWindow::BaseWindow(QWidget* parent, bool showCloseButton)
    : QWidget(parent)
    , m_isDragging(false)
    , m_isMaximized(false)
    , m_backgroundColor(QColor(247, 247, 247))
    , m_titleBarColor(QColor(53, 53, 53))
    , m_windowRadius(8)
    , m_shadowEnabled(true)
    , m_shadowBlurRadius(10)
{
    // 设置窗口标志
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 初始化成员
    initMembers();

    // 初始化UI
    initUI();

    // 初始化样式
    initStyle();

    // 初始化连接
    initConnections();

    // 默认大小
    setWindowSize(800, 600);

    // 根据参数设置是否显示关闭按钮
    m_closeButton->setVisible(showCloseButton);
}

BaseWindow::~BaseWindow()
{
    // 清理资源
}

void BaseWindow::initMembers()
{
    m_titleBar = nullptr;
    m_contentWidget = nullptr;
    m_titleLabel = nullptr;
    m_closeButton = nullptr;
    m_minButton = nullptr;
    m_maxButton = nullptr;
    m_mainLayout = nullptr;
    m_titleLayout = nullptr;
    m_contentLayout = nullptr;

    m_isDragging = false;
    m_isMaximized = false;

    m_normalGeometry = QRect(0, 0, 800, 600);
}

void BaseWindow::initUI()
{
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(m_shadowBlurRadius, m_shadowBlurRadius,
        m_shadowBlurRadius, m_shadowBlurRadius);

    // 设置标题栏
    setupTitleBar();

    // 设置内容区域
    setupContentArea();

    // 更新窗口样式
    updateWindowStyle();
}

void BaseWindow::setupTitleBar()
{
    // 创建标题栏
    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("titleBar");
    m_titleBar->setFixedHeight(40);
    m_titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 创建标题栏布局
    m_titleLayout = new QHBoxLayout(m_titleBar);
    m_titleLayout->setSpacing(10);
    m_titleLayout->setContentsMargins(15, 0, 10, 0);

    // 创建标题标签
    m_titleLabel = new QLabel("Base Window", m_titleBar);
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // 设置标题字体
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(10);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);

    // 创建按钮
    m_minButton = new QPushButton("─", m_titleBar);
    m_maxButton = new QPushButton("□", m_titleBar);
    m_closeButton = new QPushButton("×", m_titleBar);

    // 设置按钮对象名
    m_minButton->setObjectName("minButton");
    m_maxButton->setObjectName("maxButton");
    m_closeButton->setObjectName("closeButton");

    // 设置按钮固定大小
    m_minButton->setFixedSize(30, 30);
    m_maxButton->setFixedSize(30, 30);
    m_closeButton->setFixedSize(30, 30);

    // 设置按钮样式
    QString buttonStyle =
        "QPushButton {"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(255, 255, 255, 30);"
        "}";

    m_minButton->setStyleSheet(buttonStyle);
    m_maxButton->setStyleSheet(buttonStyle);

    // 关闭按钮特殊样式
    m_closeButton->setStyleSheet(
        "QPushButton {"
        "   border: none;"
        "   border-radius: 4px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #E81123;"
        "   color: white;"
        "}");

    // 将部件添加到标题栏布局
    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_minButton);
    m_titleLayout->addWidget(m_maxButton);
    m_titleLayout->addWidget(m_closeButton);

    // 将标题栏添加到主布局
    m_mainLayout->addWidget(m_titleBar);
}

void BaseWindow::setupContentArea()
{
    // 创建内容区域
    m_contentWidget = new QWidget(this);
    m_contentWidget->setObjectName("contentWidget");
    m_contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 创建内容布局
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    // 将内容区域添加到主布局
    m_mainLayout->addWidget(m_contentWidget, 1); // 1 表示可伸缩
}

void BaseWindow::initStyle()
{
    // 设置默认背景色
    QPalette palette;
    palette.setColor(QPalette::Window, m_backgroundColor);
    m_contentWidget->setPalette(palette);
    m_contentWidget->setAutoFillBackground(true);

    // 设置标题栏颜色
    QPalette titlePalette;
    titlePalette.setColor(QPalette::Window, m_titleBarColor);
    m_titleBar->setPalette(titlePalette);
    m_titleBar->setAutoFillBackground(true);

    // 设置标题文字颜色
    QPalette titleLabelPalette;
    titleLabelPalette.setColor(QPalette::WindowText, Qt::white);
    m_titleLabel->setPalette(titleLabelPalette);
}

void BaseWindow::initConnections()
{
    // 连接按钮信号
    connect(m_closeButton, &QPushButton::clicked, this, &BaseWindow::onCloseClicked);
    connect(m_minButton, &QPushButton::clicked, this, &BaseWindow::onMinClicked);
    connect(m_maxButton, &QPushButton::clicked, this, &BaseWindow::onMaxClicked);
}

void BaseWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 绘制窗口阴影
    if (m_shadowEnabled)
    {
        drawWindowShadow(painter);
    }

    // 绘制窗口背景
    QRect windowRect = rect().adjusted(m_shadowBlurRadius, m_shadowBlurRadius,
        -m_shadowBlurRadius, -m_shadowBlurRadius);
    drawRoundedRect(painter, windowRect, m_windowRadius);

    // 填充背景色
    painter.setBrush(m_backgroundColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(windowRect, m_windowRadius, m_windowRadius);
}

void BaseWindow::drawWindowShadow(QPainter& painter)
{
    // 简单的阴影效果
    for (int i = 0; i < m_shadowBlurRadius; ++i)
    {
        QRect shadowRect = rect().adjusted(i, i, -i, -i);
        QColor shadowColor = QColor(0, 0, 0, 50 - i * 5);
        painter.setPen(QPen(shadowColor, 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(shadowRect, m_windowRadius + i, m_windowRadius + i);
    }
}

void BaseWindow::drawRoundedRect(QPainter& painter, const QRect& rect, int radius)
{
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect, radius, radius);
}

void BaseWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        // 如果点击在标题栏区域，开始拖动
        if (m_titleBar->geometry().contains(event->pos()) ||
            event->pos().y() <= m_titleBar->height() + m_shadowBlurRadius)
        {
            m_isDragging = true;
            m_dragStartPosition = event->globalPos() - frameGeometry().topLeft();
            event->accept();
        }
    }
}

void BaseWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDragging && (event->buttons() & Qt::LeftButton))
    {
        QPoint newPos = event->globalPos() - m_dragStartPosition;
        move(newPos);
        event->accept();
    }
}

void BaseWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isDragging = false;
        event->accept();
    }
}

void BaseWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update();
}

void BaseWindow::setWindowTitle(const QString& title)
{
    if (m_titleLabel)
    {
        m_titleLabel->setText(title);
    }
    QWidget::setWindowTitle(title);
}

void BaseWindow::setWindowSize(int width, int height)
{
    resize(width, height);
}

void BaseWindow::setWindowSize(const QSize& size)
{
    resize(size);
}

void BaseWindow::setFixedWindowSize(int width, int height)
{
    setFixedSize(width, height);
}

void BaseWindow::setFixedWindowSize(const QSize& size)
{
    setFixedSize(size);
}

void BaseWindow::setMinimumWindowSize(int width, int height)
{
    setMinimumSize(width, height);
}

void BaseWindow::setMaximumWindowSize(int width, int height)
{
    setMaximumSize(width, height);
}

void BaseWindow::setBackgroundColor(const QColor& color)
{
    m_backgroundColor = color;

    QPalette palette;
    palette.setColor(QPalette::Window, m_backgroundColor);
    m_contentWidget->setPalette(palette);
    m_contentWidget->setAutoFillBackground(true);

    update();
}

void BaseWindow::setWindowRadius(int radius)
{
    m_windowRadius = radius;
    update();
}

void BaseWindow::setWindowShadow(bool enabled, int blurRadius)
{
    m_shadowEnabled = enabled;
    m_shadowBlurRadius = blurRadius;

    // 更新边距
    if (m_mainLayout)
    {
        if (enabled)
        {
            m_mainLayout->setContentsMargins(blurRadius, blurRadius, blurRadius, blurRadius);
        }
        else
        {
            m_mainLayout->setContentsMargins(0, 0, 0, 0);
        }
    }

    update();
}

void BaseWindow::setTitleBarVisible(bool visible)
{
    m_titleBar->setVisible(visible);
}

void BaseWindow::setTitleBarHeight(int height)
{
    m_titleBar->setFixedHeight(height);
}

void BaseWindow::setTitleBarColor(const QColor& color)
{
    m_titleBarColor = color;

    QPalette palette;
    palette.setColor(QPalette::Window, m_titleBarColor);
    m_titleBar->setPalette(palette);
    m_titleBar->setAutoFillBackground(true);

    update();
}

void BaseWindow::setContentMargins(int left, int top, int right, int bottom)
{
    if (m_contentLayout)
    {
        m_contentLayout->setContentsMargins(left, top, right, bottom);
    }
}

void BaseWindow::showCentered()
{
    // 居中显示
    QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    show();
}

void BaseWindow::showMaximized()
{
    m_isMaximized = true;
    m_normalGeometry = geometry();

    // 最大化时移除圆角和阴影
    int oldRadius = m_windowRadius;
    bool oldShadow = m_shadowEnabled;

    m_windowRadius = 0;
    m_shadowEnabled = false;
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget::showMaximized();

    // 恢复设置（用于下次显示）
    m_windowRadius = oldRadius;
    m_shadowEnabled = oldShadow;
}

void BaseWindow::showMinimized()
{
    QWidget::showMinimized();
}

void BaseWindow::showNormal()
{
    m_isMaximized = false;

    // 恢复圆角和阴影
    if (m_shadowEnabled)
    {
        m_mainLayout->setContentsMargins(m_shadowBlurRadius, m_shadowBlurRadius,
            m_shadowBlurRadius, m_shadowBlurRadius);
    }

    QWidget::showNormal();

    // 恢复之前的大小和位置
    if (!m_normalGeometry.isNull())
    {
        setGeometry(m_normalGeometry);
    }
}

QWidget* BaseWindow::contentWidget() const
{
    return m_contentWidget;
}

void BaseWindow::updateWindowStyle()
{
    // 更新样式表
    QString styleSheet = QString(
        "QWidget#contentWidget {"
        "   background-color: %1;"
        "   border-bottom-left-radius: %2px;"
        "   border-bottom-right-radius: %2px;"
        "}"
        "QWidget#titleBar {"
        "   background-color: %3;"
        "   border-top-left-radius: %2px;"
        "   border-top-right-radius: %2px;"
        "}"
    ).arg(m_backgroundColor.name())
        .arg(m_windowRadius)
        .arg(m_titleBarColor.name());

    setStyleSheet(styleSheet);
}

void BaseWindow::onCloseClicked()
{
    close();
}

void BaseWindow::onMinClicked()
{
    showMinimized();
}

void BaseWindow::onMaxClicked()
{
    if (m_isMaximized)
    {
        showNormal();
        m_maxButton->setText("□");
    }
    else
    {
        showMaximized();
        m_maxButton->setText("❐");
    }
}