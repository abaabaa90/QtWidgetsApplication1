#pragma once

#include <QWidget>
#include <QList>

class QVBoxLayout;
class RenderViewFrame;
class QStackedWidget;
class QSplitter;

/**
 * @brief Manages a recursive QSplitter-based layout of RenderViewFrame widgets.
 *
 * This is the core container that replaces the placeholder 3D page.
 * It supports:
 *  - Splitting any frame horizontally or vertically (recursive QSplitter tree)
 *  - Maximizing a single frame (hides all others via setVisible)
 *  - Restoring from maximized state
 *  - Closing frames (with automatic splitter collapse)
 *  - Active-frame tracking with visual border highlight
 */
class RenderViewContainer : public QWidget
{
    Q_OBJECT

public:
    explicit RenderViewContainer(QWidget* parent = nullptr);
    ~RenderViewContainer() override;

    /// Return the currently active frame, or nullptr.
    RenderViewFrame* activeFrame() const;

    /// Return all frames managed by this container.
    QList<RenderViewFrame*> allFrames() const;

public slots:
    void splitHorizontal(RenderViewFrame* frame);
    void splitVertical(RenderViewFrame* frame);
    void maximizeFrame(RenderViewFrame* frame);
    void restoreFrame(RenderViewFrame* frame);
    void closeFrame(RenderViewFrame* frame);

private:
    void setupUI();
    void connectFrame(RenderViewFrame* frame);
    void disconnectFrame(RenderViewFrame* frame);
    void setActiveFrame(RenderViewFrame* frame);

    // Split helpers
    QSplitter* findParentSplitter(QWidget* widget) const;
    void replaceInLayout(QWidget* oldWidget, QWidget* newWidget);

    // Collapse logic
    void collapseRedundantSplitter(QSplitter* splitter);

    // Factory
    RenderViewFrame* createFrame();

    // Layout structure
    QVBoxLayout*    m_mainLayout    = nullptr;
    QStackedWidget* m_stackWidget   = nullptr;
    QWidget*        m_normalPage    = nullptr;  // page 0: normal split layout
    QWidget*        m_maxPage       = nullptr;  // page 1: maximized view

    // m_rootWidget is the single widget in m_normalPage's layout.
    // It is either a RenderViewFrame (no splits) or a QSplitter (has splits).
    QWidget*        m_rootWidget    = nullptr;

    // State
    RenderViewFrame* m_activeFrame     = nullptr;
    RenderViewFrame* m_maximizedFrame  = nullptr;
    QList<RenderViewFrame*> m_frames;
    int m_frameCounter = 0;
};
