// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "utils/capturehistory.h"

#include <QList>
#include <QPointer>
#include <QTimer>
#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QPushButton;

// Grid of thumbnails of saved captures, newest first. Single instance.
class CaptureHistoryWindow : public QWidget
{
    Q_OBJECT
public:
    // Opens the window, or reloads and raises it when already open.
    static void showWindow();

private:
    explicit CaptureHistoryWindow(QWidget* parent = nullptr);

    void reload();
    void updateButtons();
    QListWidgetItem* currentItemOrNull() const;
    void loadNextChunk();
    void scheduleLoad();
    QPixmap placeholder(const QString& label) const;

    void openCurrent();
    void editCurrent();
    void copyCurrent();
    void showCurrentInFinder();
    void removeCurrent();

    static QPointer<CaptureHistoryWindow> s_instance;

    CaptureHistory m_history;
    QListWidget* m_list;
    QPushButton* m_openButton;
    QPushButton* m_editButton;
    QPushButton* m_copyButton;
    QPushButton* m_finderButton;
    QPushButton* m_removeButton;
    QTimer m_loadTimer;
};
