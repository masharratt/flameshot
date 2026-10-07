// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/actionlist.h"

#include <QMap>
#include <QWidget>

class QCheckBox;
class QLabel;

// Settings tab: which after-capture actions each workflow hotkey runs.
class WorkflowsConf : public QWidget
{
    Q_OBJECT
public:
    explicit WorkflowsConf(QWidget* parent = nullptr);

public slots:
    void updateComponents();

private:
    void addWorkflow(const QString& hotkey, const QString& title);
    void onToggled(const QString& hotkey);

    struct Row
    {
        QLabel* shortcut = nullptr;
        QMap<CaptureAction, QCheckBox*> boxes;
    };
    QMap<QString, Row> m_rows;
    bool m_updating = false;
};
