// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "tools/abstracttwopointtool.h"

class RulerTool : public AbstractTwoPointTool
{
    Q_OBJECT
public:
    explicit RulerTool(QObject* parent = nullptr);

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;
    QRect boundingRect() const override;

    CaptureTool* copy(QObject* parent = nullptr) override;
    void process(QPainter& painter, const QPixmap& pixmap) override;

protected:
    CaptureTool::Type type() const override;

public slots:
    void pressed(CaptureContext& context) override;

private:
    int tickHalfLength() const;
    QRectF labelRect(const QString& text) const;
};
