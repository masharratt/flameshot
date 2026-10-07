// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/actionlist.h"

#include <QMap>
#include <QWidget>

#include <QColor>

class QCheckBox;
class QLabel;
class QPushButton;
class QSpinBox;

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
    void addEffectsGroup();
    void addRecordingGroup();
    void onRecordingChanged();
    void onEffectChanged();
    void updatePreview();

    struct Row
    {
        QLabel* shortcut = nullptr;
        QMap<CaptureAction, QCheckBox*> boxes;
    };
    QMap<QString, Row> m_rows;
    bool m_updating = false;

    QSpinBox* m_borderPx = nullptr;
    QPushButton* m_borderColor = nullptr;
    QSpinBox* m_cornerRadius = nullptr;
    QCheckBox* m_shadow = nullptr;
    QLabel* m_preview = nullptr;

    // Only created when screen recording is supported
    QMap<QString, QLabel*> m_recordShortcuts;
    QSpinBox* m_gifFps = nullptr;
    QSpinBox* m_gifMaxWidth = nullptr;
    QColor m_borderColorValue;
};
