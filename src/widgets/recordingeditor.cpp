// SPDX-License-Identifier: GPL-3.0-or-later

#include "recordingeditor.h"
#include "core/flameshot.h"
#include "core/recordingcontroller.h"
#include "platform/gifencoder.h"
#include "platform/videoedit.h"
#include "platform/videopreview.h"
#include "platform/windowspace.h"
#include "utils/abstractlogger.h"
#include "utils/confighandler.h"
#include "widgets/trimtimeline.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QPointer>
#include <QPushButton>
#include <QStandardPaths>
#include <QVBoxLayout>

namespace {

const QString kMp4 = QStringLiteral("mp4");
const QString kGif = QStringLiteral("gif");
const int kTickMs = 33;
// Playback loops back to the start this close to the end handle
const double kLoopSlack = 0.02;

QString seconds(double s)
{
    return QString::number(s, 'f', 2);
}

} // namespace

void RecordingEditor::open(const QString& path,
                           const QString& kind,
                           const QString& editSource)
{
    const VideoInfo info = probeVideo(editSource);
    if (editSource.isEmpty() || !info.ok) {
        AbstractLogger::error()
          << tr("Could not open the recording for editing: %1").arg(path);
        RecordingController::discardEditSource(editSource, path);
        return;
    }
    auto* editor = new RecordingEditor(path, kind, editSource, info.duration);
    showOnActiveSpace(editor);
    editor->show();
    editor->raise();
    editor->activateWindow();
}

RecordingEditor::RecordingEditor(const QString& path,
                                 const QString& kind,
                                 const QString& editSource,
                                 double duration)
  : QWidget(nullptr, Qt::Window)
  , m_path(path)
  , m_kind(kind)
  , m_source(editSource)
  , m_duration(duration)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowTitle(tr("Edit recording"));

    auto* layout = new QVBoxLayout(this);
    m_preview = createVideoPreview(m_source, this);
    m_preview->widget()->setMinimumSize(480, 270);
    layout->addWidget(m_preview->widget(), 1);

    m_timeline = new TrimTimeline(this);
    m_timeline->setDuration(m_duration);
    m_timeline->setRange(fullTrim(m_duration));
    layout->addWidget(m_timeline);

    auto* info = new QHBoxLayout();
    m_play = new QPushButton(tr("Play"), this);
    connect(m_play, &QPushButton::clicked, this, &RecordingEditor::togglePlay);
    info->addWidget(m_play);
    m_times = new QLabel(this);
    info->addWidget(m_times, 1);
    m_beautify = new QCheckBox(tr("Beautify"), this);
    m_beautify->setToolTip(
      tr("Frame the video with the background, padding and shadow set on the "
         "Workflows tab"));
    m_beautify->setChecked(ConfigHandler().beautifyAlways());
    info->addWidget(m_beautify);
    layout->addLayout(info);

    auto* buttons = new QHBoxLayout();
    m_status = new QLabel(this);
    m_status->setWordWrap(true);
    buttons->addWidget(m_status, 1);
    m_cancel = new QPushButton(tr("Cancel"), this);
    connect(m_cancel, &QPushButton::clicked, this, &QWidget::close);
    buttons->addWidget(m_cancel);
    m_exportMp4 = new QPushButton(tr("Export MP4"), this);
    connect(
      m_exportMp4, &QPushButton::clicked, this, [this]() { exportAs(kMp4); });
    buttons->addWidget(m_exportMp4);
    if (m_kind == kGif) {
        m_exportGif = new QPushButton(tr("Export GIF"), this);
        connect(
          m_exportGif, &QPushButton::clicked, this, [this]() { exportAs(kGif); });
        buttons->addWidget(m_exportGif);
        m_exportGif->setDefault(true);
    } else {
        m_exportMp4->setDefault(true);
    }
    layout->addLayout(buttons);

    connect(m_timeline,
            &TrimTimeline::rangeChanged,
            this,
            &RecordingEditor::onRangeChanged);
    connect(m_timeline, &TrimTimeline::seekRequested, this, [this](double t) {
        m_preview->seek(t);
        m_timeline->setPosition(t);
    });
    m_tick.setInterval(kTickMs);
    connect(&m_tick, &QTimer::timeout, this, &RecordingEditor::tick);
    m_tick.start();
    updateControls();
}

RecordingEditor::~RecordingEditor() = default;

void RecordingEditor::closeEvent(QCloseEvent* event)
{
    // An export in flight still needs the source and this window
    if (!m_machine.transition(EditState::Closed)) {
        event->ignore();
        return;
    }
    m_tick.stop();
    m_preview->pause();
    RecordingController::discardEditSource(m_source, m_path);
    QWidget::closeEvent(event);
}

void RecordingEditor::togglePlay()
{
    if (m_preview->isPlaying()) {
        m_preview->pause();
    } else {
        const TrimRange r = m_timeline->range();
        const double pos = m_preview->position();
        if (pos < r.start || pos >= r.end - kLoopSlack) {
            m_preview->seek(r.start);
        }
        m_preview->play();
    }
    updateControls();
}

void RecordingEditor::tick()
{
    const double pos = m_preview->position();
    const TrimRange r = m_timeline->range();
    if (m_preview->isPlaying() && pos >= r.end - kLoopSlack) {
        m_preview->seek(r.start);
    }
    m_timeline->setPosition(pos);
    m_play->setText(m_preview->isPlaying() ? tr("Pause") : tr("Play"));
}

void RecordingEditor::onRangeChanged(bool start)
{
    // Show the frame under the handle being dragged
    m_preview->pause();
    const TrimRange r = m_timeline->range();
    const double t = start ? r.start : r.end;
    m_preview->seek(t);
    m_timeline->setPosition(t);
    updateControls();
}

void RecordingEditor::updateControls()
{
    const TrimRange r = m_timeline->range();
    m_times->setText(tr("Keep %1 s to %2 s (%3 s of %4 s)")
                       .arg(seconds(r.start),
                            seconds(r.end),
                            seconds(r.end - r.start),
                            seconds(m_duration)));
    const bool live = m_machine.isInteractive();
    m_timeline->setEnabled(live);
    m_play->setEnabled(live);
    m_beautify->setEnabled(live);
    m_cancel->setEnabled(live);
    m_exportMp4->setEnabled(live);
    if (m_exportGif) {
        m_exportGif->setEnabled(live);
    }
}

void RecordingEditor::exportAs(const QString& kind)
{
    if (!m_machine.transition(EditState::Exporting)) {
        return;
    }
    m_preview->pause();
    m_status->setText(tr("Exporting..."));
    updateControls();

    VideoExportParams params;
    params.input = m_source;
    // A GIF is made from an MP4 written to the temp folder first
    params.output =
      kind == kMp4
        ? RecordingController::nextOutputPath(kMp4)
        : QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
            .filePath(QStringLiteral("flameshot-edit-%1.mp4")
                        .arg(QDateTime::currentMSecsSinceEpoch()));
    params.range = clampTrim(m_timeline->range(), m_duration);
    params.beautify = m_beautify->isChecked();
    params.effects = ConfigHandler().effectSettings();
    // cfn: the editor's screen scale stands in for the recording's, pass
    // the recorded display's scale along if multi-DPI setups look off
    params.dpr = devicePixelRatioF();

    QPointer<RecordingEditor> self(this);
    const QString mp4 = params.output;
    exportVideoAsync(params, [self, kind, mp4](bool ok, QString error) {
        if (!self) {
            if (kind == kGif) {
                QFile::remove(mp4);
            }
            return;
        }
        self->exported(kind, mp4, ok, error);
    });
}

void RecordingEditor::exported(const QString& kind,
                               const QString& mp4,
                               bool ok,
                               const QString& error)
{
    if (!ok) {
        if (kind == kGif) {
            QFile::remove(mp4);
        }
        failed(tr("Export failed: %1").arg(error));
        return;
    }
    if (kind == kMp4) {
        saved(mp4, kMp4, mp4);
        return;
    }

    m_machine.transition(EditState::Converting);
    m_status->setText(tr("Making GIF..."));
    ConfigHandler config;
    GifEncodeParams params;
    params.inputMp4 = mp4;
    params.outputGif = RecordingController::nextOutputPath(kGif);
    params.fps = config.gifFps();
    params.maxWidth = config.gifMaxWidth();
    QPointer<RecordingEditor> self(this);
    encodeGifAsync(params, [self, params](bool ok, QString error) {
        // On failure, or with nobody left to offer it to, the edited MP4
        // is not needed
        if (!ok || !self) {
            QFile::remove(params.inputMp4);
        }
        if (!self) {
            return;
        }
        if (!ok) {
            self->failed(tr("GIF conversion failed: %1").arg(error));
            return;
        }
        // The edited MP4 becomes the new toast's edit source
        self->saved(params.outputGif, kGif, params.inputMp4);
    });
}

void RecordingEditor::saved(const QString& path,
                            const QString& kind,
                            const QString& source)
{
    m_machine.transition(EditState::Saved);
    const QSize size =
      kind == kGif ? QImageReader(path).size() : probeVideo(path).pixels;
    emit Flameshot::instance()->recordingSaved(path, kind, size, source);
    close();
}

void RecordingEditor::failed(const QString& message)
{
    m_machine.transition(EditState::Failed, message);
    AbstractLogger::error() << message;
    m_status->setText(message);
    // Back to editing so the user can change something and try again
    m_machine.transition(EditState::Editing);
    updateControls();
}
