// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/recordingedit.h"

#include <QString>
#include <QTimer>
#include <QWidget>

#include <memory>

class QCheckBox;
class QLabel;
class QPushButton;
class TrimTimeline;
class VideoPreview;

// "Edit recording" window, opened from a recording's toast: preview with
// play/pause, trim handles, a Beautify checkbox (saved effect settings) and
// Export. Exports are new files next to the original, which is never
// changed; each one gets its own history entry and toast. The lifecycle is
// EditStateMachine.
class RecordingEditor : public QWidget
{
    Q_OBJECT

public:
    // path is the saved recording, kind "mp4" or "gif", editSource the MP4
    // to work from. A temp editSource (one that differs from path) belongs
    // to the editor from here and is removed when the window closes, also
    // when the window cannot open.
    static void open(const QString& path,
                     const QString& kind,
                     const QString& editSource);

    ~RecordingEditor() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    RecordingEditor(const QString& path,
                    const QString& kind,
                    const QString& editSource,
                    double duration);

    void togglePlay();
    void tick();
    void onRangeChanged(bool start);
    void exportAs(const QString& kind);
    void exported(const QString& kind,
                  const QString& mp4,
                  bool ok,
                  const QString& error);
    void saved(const QString& path, const QString& kind, const QString& source);
    void failed(const QString& message);
    void updateControls();

    EditStateMachine m_machine;
    QString m_path;
    QString m_kind;
    QString m_source;
    double m_duration = 0.0;
    std::unique_ptr<VideoPreview> m_preview;
    TrimTimeline* m_timeline = nullptr;
    QPushButton* m_play = nullptr;
    QPushButton* m_exportMp4 = nullptr;
    QPushButton* m_exportGif = nullptr;
    QPushButton* m_cancel = nullptr;
    QCheckBox* m_beautify = nullptr;
    QLabel* m_times = nullptr;
    QLabel* m_status = nullptr;
    QTimer m_tick;
};
