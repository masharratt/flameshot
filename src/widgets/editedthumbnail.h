// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPixmap>
#include <QPoint>
#include <QPointer>
#include <QString>
#include <QWidget>

// Small always-on-top picture of an image just saved from the editor, in the
// bottom-right corner of the screen under the cursor. Left click opens it in
// the editor again, right click closes it, and dragging drops the file into
// another app. Only one is shown at a time; a newer edit replaces it.
class EditedThumbnail : public QWidget
{
    Q_OBJECT

public:
    static void showFor(const QString& path, const QPixmap& image);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    EditedThumbnail(const QString& path, const QPixmap& image);
    void startDrag();

    QString m_path;
    QPixmap m_image;
    QPixmap m_scaled;
    QPoint m_pressPos;
    bool m_pressed = false;

    static QPointer<EditedThumbnail> s_current;
};
