// SPDX-License-Identifier: GPL-3.0-or-later

#include "workflowsconf.h"
#include "utils/confighandler.h"
#include "utils/imageeffects.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLinearGradient>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {

struct ActionLabel
{
    CaptureAction action;
    const char* text;
};

// Display order of the checkboxes; the run order is fixed by normalisedOrder
const ActionLabel kActionLabels[] = {
    { CaptureAction::ApplyEffects, QT_TR_NOOP("Apply effects") },
    { CaptureAction::Save, QT_TR_NOOP("Save") },
    { CaptureAction::CopyImage, QT_TR_NOOP("Copy image") },
    { CaptureAction::CopyPath, QT_TR_NOOP("Copy path") },
    { CaptureAction::Pin, QT_TR_NOOP("Pin") },
    { CaptureAction::OpenEditor, QT_TR_NOOP("Open editor") },
    { CaptureAction::ShowToast, QT_TR_NOOP("Show toast") },
};

} // namespace

WorkflowsConf::WorkflowsConf(QWidget* parent)
  : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);

    addWorkflow(QStringLiteral("TAKE_SCREENSHOT"), tr("Capture screen"));
    addWorkflow(QStringLiteral("CAPTURE_AND_EDIT"), tr("Capture and edit"));
    addEffectsGroup();
    layout->addStretch();

    updateComponents();
}

void WorkflowsConf::addWorkflow(const QString& hotkey, const QString& title)
{
    auto* box = new QGroupBox(title, this);
    auto* boxLayout = new QVBoxLayout(box);

    Row row;
    row.shortcut = new QLabel(box);
    boxLayout->addWidget(row.shortcut);

    auto* hint = new QLabel(tr("Change the shortcut in the Shortcuts tab."), box);
    hint->setEnabled(false);
    boxLayout->addWidget(hint);

    auto* checks = new QHBoxLayout();
    for (const ActionLabel& entry : kActionLabels) {
        auto* check = new QCheckBox(tr(entry.text), box);
        row.boxes.insert(entry.action, check);
        checks->addWidget(check);
        connect(check, &QCheckBox::toggled, this, [this, hotkey]() {
            onToggled(hotkey);
        });
    }
    boxLayout->addLayout(checks);

    static_cast<QVBoxLayout*>(layout())->addWidget(box);
    m_rows.insert(hotkey, row);
}

void WorkflowsConf::addEffectsGroup()
{
    auto* box = new QGroupBox(tr("Effects"), this);
    auto* boxLayout = new QHBoxLayout(box);
    auto* form = new QFormLayout();

    m_borderPx = new QSpinBox(box);
    m_borderPx->setRange(0, 100);
    m_borderPx->setSuffix(tr(" px"));
    form->addRow(tr("Border width"), m_borderPx);

    m_borderColor = new QPushButton(box);
    form->addRow(tr("Border color"), m_borderColor);

    m_cornerRadius = new QSpinBox(box);
    m_cornerRadius->setRange(0, 200);
    m_cornerRadius->setSuffix(tr(" px"));
    form->addRow(tr("Corner radius"), m_cornerRadius);

    m_shadow = new QCheckBox(tr("Drop shadow"), box);
    form->addRow(QString(), m_shadow);
    boxLayout->addLayout(form);

    m_preview = new QLabel(box);
    m_preview->setAlignment(Qt::AlignCenter);
    m_preview->setMinimumSize(180, 140);
    boxLayout->addWidget(m_preview, 1);

    connect(m_borderPx,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &WorkflowsConf::onEffectChanged);
    connect(m_cornerRadius,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &WorkflowsConf::onEffectChanged);
    connect(m_shadow, &QCheckBox::toggled, this, &WorkflowsConf::onEffectChanged);
    connect(m_borderColor, &QPushButton::clicked, this, [this]() {
        const QColor picked =
          QColorDialog::getColor(m_borderColorValue, this, tr("Border color"));
        if (picked.isValid()) {
            m_borderColorValue = picked;
            onEffectChanged();
        }
    });

    static_cast<QVBoxLayout*>(layout())->addWidget(box);
}

void WorkflowsConf::onEffectChanged()
{
    if (m_updating) {
        return;
    }
    ConfigHandler config;
    config.setEffectBorderPx(m_borderPx->value());
    config.setEffectBorderColor(m_borderColorValue);
    config.setEffectCornerRadius(m_cornerRadius->value());
    config.setEffectShadow(m_shadow->isChecked());
    updatePreview();
}

void WorkflowsConf::updatePreview()
{
    m_borderColor->setText(m_borderColorValue.name());
    m_borderColor->setStyleSheet(
      QStringLiteral("background-color: %1;").arg(m_borderColorValue.name()));

    // Small gradient stand-in for a capture
    QImage sample(96, 64, QImage::Format_ARGB32_Premultiplied);
    QLinearGradient gradient(0, 0, 96, 64);
    gradient.setColorAt(0, QColor(70, 130, 220));
    gradient.setColorAt(1, QColor(240, 170, 60));
    {
        QPainter p(&sample);
        p.fillRect(sample.rect(), gradient);
    }
    EffectSettings settings;
    settings.borderPx = m_borderPx->value();
    settings.borderColor = m_borderColorValue;
    settings.cornerRadius = m_cornerRadius->value();
    settings.shadow = m_shadow->isChecked();
    m_preview->setPixmap(
      QPixmap::fromImage(imageeffects::applyEffects(sample, settings)));
}

void WorkflowsConf::onToggled(const QString& hotkey)
{
    if (m_updating) {
        return;
    }
    QList<CaptureAction> actions;
    const Row row = m_rows.value(hotkey);
    for (const ActionLabel& entry : kActionLabels) {
        if (row.boxes.value(entry.action)->isChecked()) {
            actions << entry.action;
        }
    }
    ConfigHandler().setWorkflowActions(hotkey, actions);
}

void WorkflowsConf::updateComponents()
{
    ConfigHandler config;
    m_updating = true;
    for (auto it = m_rows.begin(); it != m_rows.end(); ++it) {
        const QList<CaptureAction> actions = config.workflowActions(it.key());
        for (auto b = it->boxes.begin(); b != it->boxes.end(); ++b) {
            b.value()->setChecked(actions.contains(b.key()));
        }
        const QString sequence = config.shortcut(it.key());
        it->shortcut->setText(
          tr("Shortcut: %1")
            .arg(sequence.isEmpty() ? tr("not set") : sequence));
    }
    m_borderPx->setValue(config.effectBorderPx());
    m_borderColorValue = config.effectBorderColor();
    m_cornerRadius->setValue(config.effectCornerRadius());
    m_shadow->setChecked(config.effectShadow());
    m_updating = false;
    updatePreview();
}
