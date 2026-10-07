// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QPixmap>
#include <QString>
#include <QVariant>

class CaptureRequest
{
public:
    enum CaptureMode
    {
        FULLSCREEN_MODE,
        GRAPHICAL_MODE,
        SCREEN_MODE,
    };

    enum ExportTask
    {
        NO_TASK = 0,
        COPY = 1,
        SAVE = 2,
        PRINT_RAW = 4,
        PRINT_GEOMETRY = 8,
        PIN = 16,
        UPLOAD = 32,
        ACCEPT_ON_SELECT = 64,
    };

    CaptureRequest(CaptureMode mode,
                   const uint delay = 0,
                   QVariant data = QVariant(),
                   ExportTask tasks = NO_TASK);

    void setStaticID(uint id);

    uint id() const;
    uint delay() const;
    QString path() const;
    QVariant data() const;
    CaptureMode captureMode() const;
    ExportTask tasks() const;
    QRect initialSelection() const;
    QPixmap presetScreenshot() const;
    bool overwriteExisting() const;
    bool captureFirst() const;
    QRect capturedGlobalRect() const;

    void addTask(ExportTask task);
    void removeTask(ExportTask task);
    void addSaveTask(const QString& path = QString());
    void addPinTask(const QRect& pinWindowGeometry);
    void setInitialSelection(const QRect& selection);
    // Use this pixmap instead of grabbing the screen (used by the toast Edit).
    void setPresetScreenshot(const QPixmap& screenshot);
    // Save to the exact path given, replacing the file instead of numbering.
    void setOverwriteExisting(bool overwrite);
    // Marks a request made by the capture-first hotkey or tray path.
    void setCaptureFirst(bool captureFirst);
    void setCapturedGlobalRect(const QRect& rect);
    void setSelectedMonitor(int monitorIndex);
    int selectedMonitor() const;
    bool hasSelectedMonitor() const;

private:
    CaptureMode m_mode;
    uint m_delay;
    QString m_path;
    ExportTask m_tasks;
    QVariant m_data;
    QRect m_pinWindowGeometry, m_initialSelection;
    QPixmap m_presetScreenshot;
    bool m_overwriteExisting = false;
    bool m_captureFirst = false;
    QRect m_capturedGlobalRect;
    int m_selectedMonitor;
    bool m_hasSelectedMonitor;

    CaptureRequest() {}
};

using eTask = CaptureRequest::ExportTask;

inline eTask operator|(const eTask& a, const eTask& b)
{
    return static_cast<eTask>(static_cast<int>(a) | static_cast<int>(b));
}

inline eTask operator&(const eTask& a, const eTask& b)
{
    return static_cast<eTask>(static_cast<int>(a) & static_cast<int>(b));
}

inline eTask& operator|=(eTask& a, const eTask& b)
{
    a = static_cast<eTask>(static_cast<int>(a) | static_cast<int>(b));
    return a;
}
