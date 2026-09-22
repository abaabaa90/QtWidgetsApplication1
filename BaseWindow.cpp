#include "BaseWindow.h"
#include "StyleManager.h"
#include "ThemeManager.h"

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
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
    setAttribute(Qt::WA_TranslucentBackground);

    initMembers();
    initUI();
    initStyle();
    initConnections();

    int unit = StyleManager::baseUnit();
    setWindowSize(StyleManager::goldenWidth(unit * 10), unit * 10);

    m_closeButton->setVisible(showCloseButton);
}

BaseWindow::~BaseWindow()
{
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
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(m_shadowBlurRadius, m_shadowBlurRadius,
        m_shadowBlurRadius, m_shadowBlurRadius);

    setupTitleBar();
    setupContentArea();
    updateWindowStyle();
}

void BaseWindow::setupTitleBar()
{
    int unit = StyleManager::baseUnit();
    int btnSize = static_cast<int>(unit * 0.55);

    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("titleBar");
    m_titleBar->setFixedHeight(static_cast<int>(unit * 0.78));
    m_titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_titleLayout = new QHBoxLayout(m_titleBar);
    m_titleLayout->setSpacing(static_cast<int>(unit * 0.2));
    m_titleLayout->setContentsMargins(static_cast<int>(unit * 0.3), 0,
                                       static_cast<int>(unit * 0.2), 0);

    m_titleLabel = new QLabel("Base Window", m_titleBar);
    m_titleLabel->setObjectName("titleLabel");
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(static_cast<int>(unit * 0.2));
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);

    m_minButton = new QPushButton(QStringLiteral("─"), m_titleBar);
    m_maxButton = new QPushButton(QStringLiteral("□"), m_titleBar);
    m_closeButton = new QPushButton(QStringLiteral("×"), m_titleBar);

    m_minButton->setObjectName("minButton");
    m_maxButton->setObjectName("maxButton");
    m_closeButton->setObjectName("closeButton");

    m_minButton->setFixedSize(btnSize, btnSize);
    m_maxButton->setFixedSize(btnSize, btnSize);
    m_closeButton->setFixedSize(btnSize, btnSize);

    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_minButton);
    m_titleLayout->addWidget(m_maxButton);
    m_titleLayout->addWidget(m_closeButton);

    m_mainLayout->addWidget(m_titleBar);
}

void BaseWindow::setupContentArea()
{
    m_contentWidget = new QWidget(this);
    m_contentWidget->setObjectName("contentWidget");
    m_contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    m_mainLayout->addWidget(m_contentWidget, 1);
}

void BaseWindow::initStyle()
{
    QPalette palette;
    palette.setColor(QPalette::Window, m_backgroundColor);
    m_contentWidget->setPalette(palette);
    m_contentWidget->setAutoFillBackground(true);

    QPalette titlePalette;
    titlePalette.setColor(QPalette::Window, m_titleBarColor);
    m_titleBar->setPalette(titlePalette);
    m_titleBar->setAutoFillBackground(true);

    QPalette titleLabelPalette;
    titleLabelPalette.setColor(QPalette::WindowText, Qt::white);
    m_titleLabel->setPalette(titleLabelPalette);
}

void BaseWindow::initConnections()
{
    connect(m_closeButton, &QPushButton::clicked, this, &BaseWindow::onCloseClicked);
    connect(m_minButton, &QPushButton::clicked, this, &BaseWindow::onMinClicked);
    connect(m_maxButton, &QPushButton::clicked, this, &BaseWindow::onMaxClicked);

    // 主题切换时按当前主题刷新外壳颜色
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &BaseWindow::applyCurrentTheme);
}

QPushButton* BaseWindow::addTitleBarButton(const QString& text, const QString& objectName)
{
    auto* btn = new QPushButton(text, m_titleBar);
    btn->setObjectName(objectName);
    btn->setFixedSize(m_minButton->size());
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFocusPolicy(Qt::NoFocus);

    // 插到最小化按钮之前（位于 min/max/close 的左边）
    m_titleLayout->insertWidget(m_titleLayout->indexOf(m_minButton), btn);
    return btn;
}

void BaseWindow::addTitleBarWidget(QWidget* widget)
{
    if (!widget || !m_titleLayout || !m_titleLabel) return;
    // 插到标题文字之后、弹性空间之前（菜单栏/工具栏等并入顶部栏）
    m_titleLayout->insertWidget(m_titleLayout->indexOf(m_titleLabel) + 1, widget);
}

void BaseWindow::applyCurrentTheme()
{
    setBackgroundColor(ThemeManager::color(ColorRole::Window));
    setTitleBarColor(ThemeManager::color(ColorRole::TitleBar));

    // 标题栏文字颜色跟随主题（浅色主题下为深色文字）
    QPalette titleLabelPalette;
    titleLabelPalette.setColor(QPalette::WindowText, ThemeManager::color(ColorRole::TitleText));
    m_titleLabel->setPalette(titleLabelPalette);

    updateWindowStyle();
    update();
}

void BaseWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (m_shadowEnabled) {
        drawWindowShadow(painter);
    }

    QRect windowRect = rect().adjusted(m_shadowBlurRadius, m_shadowBlurRadius,
        -m_shadowBlurRadius, -m_shadowBlurRadius);
    drawRoundedRect(painter, windowRect, m_windowRadius);

    painter.setBrush(m_backgroundColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(windowRect, m_windowRadius, m_windowRadius);
}

void BaseWindow::drawWindowShadow(QPainter& painter)
{
    for (int i = 0; i < m_shadowBlurRadius; ++i) {
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
    if (event->button() == Qt::LeftButton) {
        if (m_titleBar->geometry().contains(event->pos()) ||
            event->pos().y() <= m_titleBar->height() + m_shadowBlurRadius) {
            m_isDragging = true;
            m_dragStartPosition = event->globalPos() - frameGeometry().topLeft();
            event->accept();
        }
    }
}

void BaseWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
        QPoint newPos = event->globalPos() - m_dragStartPosition;
        move(newPos);
        event->accept();
    }
}

void BaseWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
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
    if (m_titleLabel) {
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
    if (m_mainLayout) {
        m_mainLayout->setContentsMargins(
            enabled ? blurRadius : 0, enabled ? blurRadius : 0,
            enabled ? blurRadius : 0, enabled ? blurRadius : 0);
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
    if (m_contentLayout) {
        m_contentLayout->setContentsMargins(left, top, right, bottom);
    }
}

void BaseWindow::showCentered()
{
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

    int oldRadius = m_windowRadius;
    bool oldShadow = m_shadowEnabled;

    m_windowRadius = 0;
    m_shadowEnabled = false;
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget::showMaximized();

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
    if (m_shadowEnabled) {
        m_mainLayout->setContentsMargins(m_shadowBlurRadius, m_shadowBlurRadius,
            m_shadowBlurRadius, m_shadowBlurRadius);
    }
    QWidget::showNormal();
    if (!m_normalGeometry.isNull()) {
        setGeometry(m_normalGeometry);
    }
}

QWidget* BaseWindow::contentWidget() const
{
    return m_contentWidget;
}

void BaseWindow::updateWindowStyle()
{
    // 颜色由 ThemeManager / theme.qss 统一管理，这里只生成标题栏/内容区的圆角规则
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
    if (m_isMaximized) {
        showNormal();
        m_maxButton->setText(QStringLiteral("□"));
    } else {
        showMaximized();
        m_maxButton->setText(QStringLiteral("❐"));
    }
}
