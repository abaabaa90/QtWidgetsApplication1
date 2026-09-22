#pragma once

#include <QWidget>

class VtkRenderWidget;
class QLabel;
class QPushButton;

/**
 * @brief A framed VTK render view with a ParaView-style title bar.
 *
 * Each frame wraps a VtkRenderWidget inside a stylized container
 * with a dark title bar and buttons for split, maximize, and close.
 * An active frame draws a highlighted blue border.
 */
class RenderViewFrame : public QWidget
{
    Q_OBJECT

public:
    explicit RenderViewFrame(const QString& title, QWidget* parent = nullptr);
    ~RenderViewFrame() override;

    void setViewTitle(const QString& title);
    QString viewTitle() const;

    void setActive(bool active);
    bool isActive() const;

    void setMaximized(bool maximized);
    bool isMaximized() const;

    /// Access the internal VTK render widget for advanced use.
    VtkRenderWidget* vtkRenderWidget() const;

signals:
    void splitHorizontalRequested();
    void splitVerticalRequested();
    void maximizeRequested();
    void restoreRequested();
    void closeRequested();
    void activated();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void setupUI();
    void setupConnections();

    // --- UI elements ---
    QWidget*         m_titleBar     = nullptr;
    QLabel*          m_titleLabel   = nullptr;
    QPushButton*     m_splitHBtn    = nullptr;
    QPushButton*     m_splitVBtn    = nullptr;
    QPushButton*     m_maximizeBtn  = nullptr;
    QPushButton*     m_closeBtn     = nullptr;
    QWidget*         m_contentArea  = nullptr;
    VtkRenderWidget* m_vtkWidget    = nullptr;

    // --- State ---
    bool m_active     = false;
    bool m_maximized  = false;
    int  m_frameId    = 0;

    static int s_frameCounter;
};
