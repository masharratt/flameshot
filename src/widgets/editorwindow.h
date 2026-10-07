// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPixmap>
#include <QPointer>
#include <QString>
#include <QVector>
#include <QWidget>

class CaptureToolButton;
class CaptureWidget;
class QScrollArea;

// A normal top-level window that edits a saved capture. It hosts a
// CaptureWidget in windowed edit mode inside a scroll area and overwrites the
// file when the edit is saved. Closing the window discards unsaved edits.
// The tool buttons live in a fixed strip above the scroll area and the side
// panel is docked to the left edge of the viewport, so neither scrolls away
// with a large canvas.
class EditorWindow : public QWidget
{
    Q_OBJECT
public:
    // image is in device pixels; its device pixel ratio is the canvas ratio
    EditorWindow(const QString& path, const QPixmap& image);
    ~EditorWindow() override;

    // Size and center the window on the screen under the cursor, then show it
    void present();

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    // Strip height for the current buttons laid out in `width` points
    int toolbarHeightFor(int width) const;
    void layoutToolbar();
    void dockSidePanel();

    QPointer<CaptureWidget> m_capture;
    QScrollArea* m_scroll{ nullptr };
    QWidget* m_toolbar{ nullptr };
    QVector<CaptureToolButton*> m_buttons;
    QPixmap m_image;
};
