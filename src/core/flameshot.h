// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "core/capturerequest.h"
#include "widgets/capture/capturewidget.h"

#include <QMap>
#include <QObject>
#include <QPointer>
#include <QVersionNumber>
#include <QWindow>

class ConfigWindow;
class InfoWindow;
class CaptureLauncher;
class QTimer;
class QWidget;
class RecordingController;
#ifdef ENABLE_IMGUR
class UploadHistory;
#endif
#if (defined(Q_OS_MACOS) || defined(Q_OS_WIN))
class QHotkey;
#endif

enum ErrCode : uint8_t
{
    E_OK = 0,
    E_GENERAL,
    E_ABORTED,
    E_DBUSCONN,
    E_SIG_BASE = 128,
    E_SIGINT = E_SIG_BASE + 2,
    E_SIGTERM = E_SIG_BASE + 15,
};

class Flameshot : public QObject
{
    Q_OBJECT

public:
    enum Origin
    {
        CLI,
        DAEMON
    };

    static Flameshot* instance();

public slots:
    CaptureWidget* gui(
      const CaptureRequest& req = CaptureRequest::GRAPHICAL_MODE);
    void screen(CaptureRequest req, int const screenNumber = -1);
    void full(const CaptureRequest& req);
    void launcher();
    void config();

    void info();

#ifdef ENABLE_IMGUR
    void history();
#endif

    void openSavePath();

    QVersionNumber getVersion();

public:
    static void setOrigin(Origin origin);
    static Origin origin();
    void setExternalWidget(bool b);
    bool haveExternalWidget();

signals:
    void captureTaken(QPixmap p);
    void captureFailed();
    // selection uses exportCapture units (for pins); globalRect is the
    // captured area in global logical points.
    void captureSaved(const QString& path,
                      const QPixmap& capture,
                      const QRect& selection,
                      const QRect& globalRect);
    // A screen recording was saved. kind is "mp4" or "gif", size in pixels.
    void recordingSaved(const QString& path,
                        const QString& kind,
                        const QSize& size);

public slots:
    // Record hotkey or menu item: picks an area, or stops a running recording
    void toggleRecording(CaptureRequest::RecordMode mode);
    void requestCapture(const CaptureRequest& request);
    // Reopen a saved capture in the editor. Saving overwrites the same file
    // when overwrite is true, otherwise it saves a numbered copy beside it.
    void editSavedCapture(const QString& path,
                          const QPixmap& capture,
                          const QRect& globalRect,
                          bool overwrite = true);
    void exportCapture(const QPixmap& p,
                       QRect& selection,
                       const CaptureRequest& req);

private:
    Flameshot();
    bool resolveAnyConfigErrors();
    void runWorkflow(const QPixmap& capture,
                     const QRect& selection,
                     const CaptureRequest& req);

    // class members
    static Origin m_origin;
    bool m_haveExternalWidget;

    QPointer<CaptureWidget> m_captureWindow;
    QPointer<InfoWindow> m_infoWindow;
    QPointer<CaptureLauncher> m_launcherWindow;
    QPointer<ConfigWindow> m_configWindow;
    RecordingController* m_recording = nullptr;

#if defined(Q_OS_MACOS)
public:
    void showDockIcon(QWidget* window);

private:
    void onWindowVisibilityChanged(QWindow::Visibility newVisibility);
    int m_dockIconVisibleCount = 0;
#endif

#if (defined(Q_OS_MACOS) || defined(Q_OS_WIN))
    // Global hotkeys by shortcut name. Entries are created once and only
    // re-registered when their key sequence changes in the config.
    void syncHotkeys();
    void onHotkeyActivated(const QString& name);
    QMap<QString, QHotkey*> m_hotkeys;
    QMap<QString, QString> m_hotkeySequences;
    QTimer* m_hotkeyDebounce = nullptr;
#endif
};
