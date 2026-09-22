#include "RenderViewContainer.h"
#include "RenderViewFrame.h"

#include <QVBoxLayout>
#include <QSplitter>
#include <QStackedWidget>

//-----------------------------------------------------------------------------
RenderViewContainer::RenderViewContainer(QWidget* parent)
    : QWidget(parent)
{
    setupUI();

    // Create the initial frame.
    RenderViewFrame* initialFrame = createFrame();
    m_normalPage->layout()->addWidget(initialFrame);
    m_rootWidget = initialFrame;
    setActiveFrame(initialFrame);
}

//-----------------------------------------------------------------------------
RenderViewContainer::~RenderViewContainer() = default;

//-----------------------------------------------------------------------------
void RenderViewContainer::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    m_stackWidget = new QStackedWidget(this);

    // --- Page 0: normal split layout ---
    m_normalPage = new QWidget(m_stackWidget);
    m_normalPage->setObjectName("renderViewNormalPage");
    auto* normalLayout = new QVBoxLayout(m_normalPage);
    normalLayout->setContentsMargins(0, 0, 0, 0);
    normalLayout->setSpacing(0);

    // --- Page 1: maximized view ---
    m_maxPage = new QWidget(m_stackWidget);
    m_maxPage->setObjectName("renderViewMaxPage");
    auto* maxLayout = new QVBoxLayout(m_maxPage);
    maxLayout->setContentsMargins(0, 0, 0, 0);
    maxLayout->setSpacing(0);

    m_stackWidget->addWidget(m_normalPage);  // index 0
    m_stackWidget->addWidget(m_maxPage);     // index 1
    m_stackWidget->setCurrentIndex(0);

    m_mainLayout->addWidget(m_stackWidget);
}

//-----------------------------------------------------------------------------
RenderViewFrame* RenderViewContainer::createFrame()
{
    ++m_frameCounter;
    QString title = QString("RenderView%1").arg(m_frameCounter);
    auto* frame = new RenderViewFrame(title, this);
    m_frames.append(frame);
    connectFrame(frame);
    return frame;
}

//-----------------------------------------------------------------------------
void RenderViewContainer::connectFrame(RenderViewFrame* frame)
{
    connect(frame, &RenderViewFrame::splitHorizontalRequested,
            this, [this, frame]() { splitHorizontal(frame); });
    connect(frame, &RenderViewFrame::splitVerticalRequested,
            this, [this, frame]() { splitVertical(frame); });
    connect(frame, &RenderViewFrame::maximizeRequested,
            this, [this, frame]() { maximizeFrame(frame); });
    connect(frame, &RenderViewFrame::restoreRequested,
            this, [this, frame]() { restoreFrame(frame); });
    connect(frame, &RenderViewFrame::closeRequested,
            this, [this, frame]() { closeFrame(frame); });
    connect(frame, &RenderViewFrame::activated,
            this, [this, frame]() { setActiveFrame(frame); });
}

//-----------------------------------------------------------------------------
void RenderViewContainer::disconnectFrame(RenderViewFrame* frame)
{
    disconnect(frame, nullptr, this, nullptr);
}

//-----------------------------------------------------------------------------
void RenderViewContainer::setActiveFrame(RenderViewFrame* frame)
{
    if (m_activeFrame == frame) return;

    if (m_activeFrame) {
        m_activeFrame->setActive(false);
    }

    m_activeFrame = frame;

    if (m_activeFrame) {
        m_activeFrame->setActive(true);
    }
}

//-----------------------------------------------------------------------------
RenderViewFrame* RenderViewContainer::activeFrame() const
{
    return m_activeFrame;
}

//-----------------------------------------------------------------------------
QList<RenderViewFrame*> RenderViewContainer::allFrames() const
{
    return m_frames;
}

//-----------------------------------------------------------------------------
QSplitter* RenderViewContainer::findParentSplitter(QWidget* widget) const
{
    if (!widget || widget == m_rootWidget) return nullptr;
    QWidget* p = widget->parentWidget();
    // Walk up: a QSplitter handle's direct parent is the QSplitter itself.
    // The widget's parent could be the QSplitter directly.
    if (auto* splitter = qobject_cast<QSplitter*>(p)) {
        return splitter;
    }
    // If the widget is inside a container inside the splitter, walk up further.
    while (p && p != m_normalPage) {
        if (auto* splitter = qobject_cast<QSplitter*>(p)) {
            return splitter;
        }
        p = p->parentWidget();
    }
    return nullptr;
}

//-----------------------------------------------------------------------------
void RenderViewContainer::replaceInLayout(QWidget* oldWidget, QWidget* newWidget)
{
    if (!oldWidget || !newWidget) return;

    QSplitter* parentSplitter = findParentSplitter(oldWidget);

    if (parentSplitter) {
        // Replace inside the splitter
        int idx = parentSplitter->indexOf(oldWidget);
        if (idx >= 0) {
            // Block signals to avoid intermediate layout recalculations
            parentSplitter->insertWidget(idx, newWidget);
            oldWidget->setParent(nullptr);
        }
    } else if (oldWidget == m_rootWidget) {
        // Replace the root widget
        QLayout* layout = m_normalPage->layout();
        layout->removeWidget(oldWidget);
        oldWidget->setParent(nullptr);
        layout->addWidget(newWidget);
        m_rootWidget = newWidget;
    }
}

//-----------------------------------------------------------------------------
void RenderViewContainer::splitHorizontal(RenderViewFrame* frame)
{
    auto* newFrame = createFrame();
    auto* oldParent = findParentSplitter(frame);

    if (!oldParent) {
        // Case 1: frame is the root (unsplit). Create first splitter.
        auto* splitter = new QSplitter(Qt::Horizontal, this);
        splitter->setObjectName("renderViewSplitter");
        splitter->setChildrenCollapsible(false);
        splitter->setHandleWidth(4);

        // Remove frame from root layout, add splitter instead
        QLayout* layout = m_normalPage->layout();
        layout->removeWidget(frame);
        layout->addWidget(splitter);

        splitter->addWidget(frame);
        splitter->addWidget(newFrame);
        splitter->setSizes({1, 1});

        m_rootWidget = splitter;
    }
    else if (oldParent->orientation() == Qt::Horizontal) {
        // Case 2: parent has same orientation — add to existing splitter
        oldParent->addWidget(newFrame);
        // Re-distribute sizes equally
        int count = oldParent->count();
        QList<int> sizes;
        for (int i = 0; i < count; ++i) sizes.append(1);
        oldParent->setSizes(sizes);
    }
    else {
        // Case 3: parent has different orientation — nest a new splitter
        auto* splitter = new QSplitter(Qt::Horizontal, this);
        splitter->setObjectName("renderViewSplitter");
        splitter->setChildrenCollapsible(false);
        splitter->setHandleWidth(4);

        int idx = oldParent->indexOf(frame);
        oldParent->insertWidget(idx, splitter);
        frame->setParent(nullptr);
        splitter->addWidget(frame);
        splitter->addWidget(newFrame);
        splitter->setSizes({1, 1});
    }

    setActiveFrame(newFrame);
}

//-----------------------------------------------------------------------------
void RenderViewContainer::splitVertical(RenderViewFrame* frame)
{
    auto* newFrame = createFrame();
    auto* oldParent = findParentSplitter(frame);

    if (!oldParent) {
        // Case 1: frame is the root — create first splitter
        auto* splitter = new QSplitter(Qt::Vertical, this);
        splitter->setObjectName("renderViewSplitter");
        splitter->setChildrenCollapsible(false);
        splitter->setHandleWidth(4);

        QLayout* layout = m_normalPage->layout();
        layout->removeWidget(frame);
        layout->addWidget(splitter);

        splitter->addWidget(frame);
        splitter->addWidget(newFrame);
        splitter->setSizes({1, 1});

        m_rootWidget = splitter;
    }
    else if (oldParent->orientation() == Qt::Vertical) {
        // Case 2: parent has same orientation — add to existing splitter
        oldParent->addWidget(newFrame);
        int count = oldParent->count();
        QList<int> sizes;
        for (int i = 0; i < count; ++i) sizes.append(1);
        oldParent->setSizes(sizes);
    }
    else {
        // Case 3: parent has different orientation — nest a new splitter
        auto* splitter = new QSplitter(Qt::Vertical, this);
        splitter->setObjectName("renderViewSplitter");
        splitter->setChildrenCollapsible(false);
        splitter->setHandleWidth(4);

        int idx = oldParent->indexOf(frame);
        oldParent->insertWidget(idx, splitter);
        frame->setParent(nullptr);
        splitter->addWidget(frame);
        splitter->addWidget(newFrame);
        splitter->setSizes({1, 1});
    }

    setActiveFrame(newFrame);
}

//-----------------------------------------------------------------------------
void RenderViewContainer::maximizeFrame(RenderViewFrame* frame)
{
    if (m_maximizedFrame) return; // already maximized

    m_maximizedFrame = frame;

    // Hide all OTHER frames. QSplitter auto-collapses hidden children.
    for (auto* f : m_frames) {
        if (f != frame) {
            f->setVisible(false);
        }
    }

    frame->setMaximized(true);

    // Force layout update so splitters adjust to the single visible frame
    if (auto* rootSplitter = qobject_cast<QSplitter*>(m_rootWidget)) {
        rootSplitter->updateGeometry();
    }
}

//-----------------------------------------------------------------------------
void RenderViewContainer::restoreFrame(RenderViewFrame* frame)
{
    if (!m_maximizedFrame || m_maximizedFrame != frame) return;

    // Show all frames again
    for (auto* f : m_frames) {
        f->setVisible(true);
    }

    frame->setMaximized(false);
    m_maximizedFrame = nullptr;
}

//-----------------------------------------------------------------------------
void RenderViewContainer::closeFrame(RenderViewFrame* frame)
{
    if (!frame) return;
    if (m_frames.size() <= 1) return; // keep at least one frame

    // If closing the active frame, pick another
    if (m_activeFrame == frame) {
        setActiveFrame(nullptr);
        for (auto* f : m_frames) {
            if (f != frame) {
                setActiveFrame(f);
                break;
            }
        }
    }

    // If closing the maximized frame, restore first
    if (m_maximizedFrame == frame) {
        restoreFrame(frame);
    }

    // Remove from frame list
    m_frames.removeOne(frame);
    disconnectFrame(frame);

    // Find the splitter parent
    QSplitter* parentSplitter = findParentSplitter(frame);

    if (!parentSplitter) {
        // Frame is the root widget (should only happen if the only frame is
        // being closed, which is guarded above). Create a new empty frame.
        QLayout* layout = m_normalPage->layout();
        layout->removeWidget(frame);
        frame->deleteLater();

        auto* newFrame = createFrame();
        layout->addWidget(newFrame);
        m_rootWidget = newFrame;
        setActiveFrame(newFrame);
        return;
    }

    // Remove frame from its splitter
    frame->setParent(nullptr);
    frame->deleteLater();

    // Collapse the splitter if it now has 0 or 1 children
    collapseRedundantSplitter(parentSplitter);
}

//-----------------------------------------------------------------------------
void RenderViewContainer::collapseRedundantSplitter(QSplitter* splitter)
{
    if (!splitter) return;

    int count = splitter->count();

    if (count == 0) {
        // Splitter is empty — remove it entirely
        if (splitter == m_rootWidget) {
            // Shouldn't happen due to the guard in closeFrame, but handle gracefully
            QLayout* layout = m_normalPage->layout();
            layout->removeWidget(splitter);
            splitter->deleteLater();
            auto* newFrame = createFrame();
            layout->addWidget(newFrame);
            m_rootWidget = newFrame;
            setActiveFrame(newFrame);
        } else {
            QSplitter* grandparent = findParentSplitter(splitter);
            if (grandparent) {
                splitter->setParent(nullptr);
                splitter->deleteLater();
                collapseRedundantSplitter(grandparent);
            }
        }
        return;
    }

    if (count == 1) {
        // Only one child — pull it up to replace this splitter
        QWidget* remaining = splitter->widget(0);

        if (splitter == m_rootWidget) {
            QLayout* layout = m_normalPage->layout();
            layout->removeWidget(splitter);
            splitter->deleteLater();
            layout->addWidget(remaining);
            m_rootWidget = remaining;
        } else {
            QSplitter* grandparent = findParentSplitter(splitter);
            if (grandparent) {
                int idx = grandparent->indexOf(splitter);
                grandparent->insertWidget(idx, remaining);
                splitter->setParent(nullptr);
                splitter->deleteLater();
            }
        }
    }
    // If count >= 2, nothing to collapse
}
