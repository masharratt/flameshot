// SPDX-License-Identifier: GPL-3.0-or-later

#include "scrollcapturebar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
const QSize kPreviewSize(40, 64);
const int kStatusWidth = 230;
} // namespace

ScrollCaptureBar::ScrollCaptureBar(QWidget* parent)
  : QWidget(parent,
            Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
              Qt::WindowDoesNotAcceptFocus)
{
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_QuitOnClose, false);
    setFocusPolicy(Qt::NoFocus);
    setObjectName(QStringLiteral("ScrollCaptureBar"));
    // Same look as the recording control
    setStyleSheet(QStringLiteral(
      "#ScrollCaptureBar { background-color: #2b2b2b; border: 1px solid #555;"
      " border-radius: 8px; }"
      "QLabel { color: #eee; }"
      "QLabel#Preview { background: #1e1e1e; border: 1px solid #444; }"
      "QLabel#Height { color: #aaa; }"
      "QPushButton { color: #eee; background: #3c3c3c; border: none;"
      " border-radius: 4px; padding: 3px 8px; }"
      "QPushButton:hover { background: #505050; }"
      "QPushButton:disabled { color: #888; }"));

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 6, 10, 6);
    layout->setSpacing(8);

    m_preview = new QLabel(this);
    m_preview->setObjectName(QStringLiteral("Preview"));
    m_preview->setFixedSize(kPreviewSize);
    m_preview->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    layout->addWidget(m_preview);

    auto* text = new QVBoxLayout();
    text->setSpacing(2);
    m_status = new QLabel(tr("Getting ready..."), this);
    m_status->setWordWrap(true);
    m_status->setFixedWidth(kStatusWidth);
    m_status->setMinimumHeight(m_status->fontMetrics().lineSpacing() * 2);
    text->addWidget(m_status);
    m_height = new QLabel(this);
    m_height->setObjectName(QStringLiteral("Height"));
    text->addWidget(m_height);
    layout->addLayout(text);

    const auto makeButton = [this, layout](const QString& label) {
        auto* b = new QPushButton(label, this);
        b->setFocusPolicy(Qt::NoFocus);
        b->setEnabled(false);
        layout->addWidget(b);
        return b;
    };
    m_auto = makeButton(tr("Stop auto"));
    // Wide enough for both labels, so the bar never changes size
    m_auto->setMinimumWidth(m_auto->sizeHint().width());
    m_auto->setText(tr("Auto"));
    m_auto->setToolTip(
      tr("Scroll the content automatically until it ends (needs "
         "Accessibility permission)"));
    m_done = makeButton(tr("Done"));
    m_cancel = makeButton(tr("Cancel"));
    // Cancel works even before grabbing is ready
    m_cancel->setEnabled(true);

    connect(m_auto, &QPushButton::clicked, this, &ScrollCaptureBar::autoRequested);
    connect(m_done, &QPushButton::clicked, this, &ScrollCaptureBar::doneRequested);
    connect(
      m_cancel, &QPushButton::clicked, this, &ScrollCaptureBar::cancelRequested);

    setStitched(0, QImage());
    setFixedSize(sizeHint());
}

void ScrollCaptureBar::setStatus(const QString& text)
{
    m_status->setText(text);
}

void ScrollCaptureBar::setStitched(int heightPx, const QImage& preview)
{
    m_height->setText(heightPx > 0 ? tr("%1 px tall").arg(heightPx)
                                   : QString());
    if (!preview.isNull()) {
        m_preview->setPixmap(QPixmap::fromImage(preview));
    }
}

void ScrollCaptureBar::setAutoRunning(bool running)
{
    m_auto->setText(running ? tr("Stop auto") : tr("Auto"));
}

void ScrollCaptureBar::setReady(bool ready)
{
    m_auto->setEnabled(ready);
    m_done->setEnabled(ready);
}
