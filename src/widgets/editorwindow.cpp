// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorwindow.h"
#include "core/capturerequest.h"
#include "utils/globalvalues.h"
#include "utils/colorutils.h"
#include "utils/confighandler.h"
#include "utils/pathinfo.h"
#include "widgets/capture/capturebutton.h"
#include "widgets/capture/capturetoolbutton.h"
#include "widgets/capture/capturewidget.h"
#include "widgets/editcanvas.h"
#include "platform/windowspace.h"

#include <QCursor>
#include <QEvent>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QScreen>
#include <QScrollArea>
#include <QShortcut>
#include <QVBoxLayout>
#include <QWindow>
#include <algorithm>

namespace {
// Toolbar strip metrics in logical points
constexpr int kToolbarPad = 6;
constexpr int kToolbarSpacing = 4;
} // namespace

EditorWindow::EditorWindow(const QString& path, const QPixmap& image)
  : QWidget(nullptr, Qt::Window | Qt::WindowMinMaxButtonsHint)
  , m_image(image)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowTitle(tr("Edit - %1").arg(QFileInfo(path).fileName()));

    // Edits always overwrite the opened file; the image is never resampled
    CaptureRequest req(CaptureRequest::GRAPHICAL_MODE);
    req.setEditImage(image);
    req.addSaveTask(path);
    req.setOverwriteExisting(true);

    m_toolbar = new QWidget(this);
    m_toolbar->setObjectName(QStringLiteral("editorToolbar"));
    m_toolbar->setAttribute(Qt::WA_StyledBackground, true);
    m_toolbar->setStyleSheet(
      QStringLiteral("#editorToolbar { background: #2b2b2b; }"));

    auto* scroll = new QScrollArea(this);
    m_scroll = scroll;
    scroll->setObjectName(QStringLiteral("editorScroll"));
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet(
      QStringLiteral("#editorScroll, #editorScroll > QWidget { background: "
                     "#2b2b2b; }"));
    m_capture = new CaptureWidget(req, false);
    scroll->setWidget(m_capture);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_toolbar);
    layout->addWidget(scroll, 1);

    // Move the tool buttons into the fixed strip
    m_buttons = m_capture->toolbarButtons();
    for (CaptureToolButton* b : m_buttons) {
        b->setParent(m_toolbar);
        b->setFixedSize(GlobalValues::buttonBaseSize(),
                        GlobalValues::buttonBaseSize());
        b->show();
    }
    addExpandButton();
    m_toolbar->setFixedHeight(toolbarHeightFor(width()));
    layoutToolbar();
    dockSidePanel();

    new QShortcut(QKeySequence::Close, this, SLOT(close()));

    // Esc, Save and the other finishing tools close the CaptureWidget
    connect(m_capture, &QObject::destroyed, this, [this]() { close(); });
    connect(m_capture,
            &CaptureWidget::editCanvasResized,
            this,
            &EditorWindow::growToCanvas);
}

// A button in the strip after the tool buttons. It is not a capture tool: it
// calls straight into the edit canvas, so the overlay never sees it.
void EditorWindow::addExpandButton()
{
    const QColor ui = ConfigHandler().uiColor();
    m_expandButton = new CaptureButton(m_toolbar);
    m_expandButton->setColor(ui);
    m_expandButton->setStyleSheet(m_expandButton->styleSheet() +
                                  QStringLiteral(" CaptureButton { padding: "
                                                 "0; }"));
    const int size = GlobalValues::buttonBaseSize();
    m_expandButton->setFixedSize(size, size);
    m_expandButton->setMask(
      QRegion(QRect(-1, -1, size + 2, size + 2), QRegion::Ellipse));
    const QString iconDir = ColorUtils::colorIsDark(ui)
                              ? PathInfo::whiteIconPath()
                              : PathInfo::blackIconPath();
    m_expandButton->setIcon(QIcon(iconDir + QStringLiteral("expand-canvas.svg")));
    m_expandButton->setIconSize(QSize(size, size) * 0.6);
    m_expandButton->setToolTip(tr("Expand canvas"));
    connect(m_expandButton, &QPushButton::clicked, this, [this]() {
        if (m_capture) {
            m_capture->expandEditCanvas();
        }
    });
    m_expandButton->show();
}

void EditorWindow::growToCanvas(const QSize& canvasLogical)
{
    QScreen* screen = windowHandle() ? windowHandle()->screen() : nullptr;
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    const QRect available = screen->availableGeometry();
    const QSize target(
      std::min(canvasLogical.width(), available.width() * 9 / 10),
      std::min(canvasLogical.height() + m_toolbar->height(),
               available.height() * 9 / 10));
    const QSize grown(std::max(width(), target.width()),
                      std::max(height(), target.height()));
    if (grown != size()) {
        resize(grown);
    }
}

EditorWindow::~EditorWindow()
{
    if (m_capture) {
        m_capture->disconnect(this);
    }
}

int EditorWindow::toolbarHeightFor(int width) const
{
    const int size = GlobalValues::buttonBaseSize();
    const int rows = toolbarRows(
      stripCount(), size, kToolbarSpacing, width - 2 * kToolbarPad);
    if (rows == 0) {
        return 0;
    }
    return rows * size + (rows - 1) * kToolbarSpacing + 2 * kToolbarPad;
}

int EditorWindow::stripCount() const
{
    return m_buttons.size() + (m_expandButton ? 1 : 0);
}

// One row of buttons that wraps to further rows when the window is narrow
void EditorWindow::layoutToolbar()
{
    const int size = GlobalValues::buttonBaseSize();
    const int usable = std::max(1, width() - 2 * kToolbarPad);
    const int perRow =
      std::max(1, (usable + kToolbarSpacing) / (size + kToolbarSpacing));
    int i = 0;
    for (CaptureToolButton* b : m_buttons) {
        const int row = i / perRow;
        const int col = i % perRow;
        b->move(kToolbarPad + col * (size + kToolbarSpacing),
                kToolbarPad + row * (size + kToolbarSpacing));
        ++i;
    }
    if (m_expandButton) {
        m_expandButton->move(kToolbarPad + (i % perRow) * (size + kToolbarSpacing),
                             kToolbarPad + (i / perRow) * (size + kToolbarSpacing));
    }
}

// The panel and its toggle become children of the scroll viewport, so they
// stay at the left edge while the canvas scrolls underneath.
void EditorWindow::dockSidePanel()
{
    QWidget* viewport = m_scroll->viewport();
    viewport->installEventFilter(this);
    for (QWidget* w : { m_capture->sidePanel(), m_capture->sidePanelToggle() }) {
        if (w) {
            w->setParent(viewport);
        }
    }
    if (QWidget* toggle = m_capture->sidePanelToggle()) {
        toggle->show();
    }
}

bool EditorWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (m_capture && watched == m_scroll->viewport() &&
        (event->type() == QEvent::Resize || event->type() == QEvent::Show)) {
        QWidget* viewport = m_scroll->viewport();
        QWidget* panel = m_capture->sidePanel();
        QWidget* toggle = m_capture->sidePanelToggle();
        if (panel) {
            panel->setFixedHeight(viewport->height());
            panel->move(0, 0);
            panel->raise();
        }
        if (toggle) {
            toggle->move(0, (viewport->height() - toggle->height()) / 2);
            toggle->raise();
        }
    }
    return QWidget::eventFilter(watched, event);
}

void EditorWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    m_toolbar->setFixedHeight(toolbarHeightFor(width()));
    layoutToolbar();
}

void EditorWindow::present()
{
    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    const QRect available = screen->availableGeometry();
    const qreal dpr = m_image.devicePixelRatio();
    // The toolbar height depends on the window width, so plan twice: once for
    // the width, then again with the strip height added to the window
    const EditCanvas widthPlan =
      planEditCanvas(m_image.size(), dpr, kEditMargin, available);
    const EditCanvas plan =
      planEditCanvas(m_image.size(),
                     dpr,
                     kEditMargin,
                     available,
                     toolbarHeightFor(widthPlan.windowSize.width()));
    resize(plan.windowSize);
    QRect frame(QPoint(0, 0), plan.windowSize);
    frame.moveCenter(available.center());
    move(frame.topLeft());
    showOnActiveSpace(this);
    show();
    raise();
    activateWindow();
    if (m_capture) {
        m_capture->setFocus();
    }
}
