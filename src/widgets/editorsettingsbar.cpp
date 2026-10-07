// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorsettingsbar.h"
#include "editorsettings.h"
#include "utils/confighandler.h"
#include "widgets/panel/sidepanelwidget.h"

#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QToolButton>

namespace {
constexpr int kBarHeight = 40;
constexpr int kSwatchSize = 22;
} // namespace

EditorSettingsBar::EditorSettingsBar(QWidget* parent)
  : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedHeight(kBarHeight);

    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(6, 0, 6, 0);
    row->setSpacing(6);

    auto* colorLabel = new QLabel(tr("Color"), this);
    colorLabel->setStyleSheet(QStringLiteral("color: #dddddd;"));
    row->addWidget(colorLabel);

    m_swatchLayout = new QHBoxLayout();
    m_swatchLayout->setContentsMargins(0, 0, 0, 0);
    m_swatchLayout->setSpacing(4);
    row->addLayout(m_swatchLayout);

    m_plus = new QToolButton(this);
    m_plus->setText(QStringLiteral("+"));
    m_plus->setToolTip(tr("Other color"));
    m_plus->setFixedSize(kSwatchSize, kSwatchSize);
    m_plus->setStyleSheet(QStringLiteral(
      "QToolButton { color: #dddddd; border: 1px solid #888888; "
      "border-radius: %1px; background: transparent; }")
                            .arg(kSwatchSize / 2));
    connect(m_plus, &QToolButton::clicked, this, [this]() {
        pickCustomColor();
    });
    row->addWidget(m_plus);

    row->addSpacing(12);
    auto* sizeLabel = new QLabel(tr("Size"), this);
    sizeLabel->setStyleSheet(QStringLiteral("color: #dddddd;"));
    row->addWidget(sizeLabel);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(1, maxToolSize);
    m_slider->setMinimumWidth(minSliderWidth);
    m_slider->setMaximumWidth(220);
    row->addWidget(m_slider);

    m_spin = new QSpinBox(this);
    m_spin->setRange(1, maxToolSize);
    m_spin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    row->addWidget(m_spin);
    row->addStretch(1);

    // Keep slider and box in sync, and report only user edits
    connect(m_slider, &QSlider::valueChanged, this, [this](int v) {
        const int size = clampThickness(v, 1, maxToolSize);
        {
            QSignalBlocker block(m_spin);
            m_spin->setValue(size);
        }
        emit sizePicked(size);
    });
    connect(m_spin,
            qOverload<int>(&QSpinBox::valueChanged),
            this,
            [this](int v) {
                const int size = clampThickness(v, 1, maxToolSize);
                {
                    QSignalBlocker block(m_slider);
                    m_slider->setValue(size);
                }
                emit sizePicked(size);
            });

    rebuildSwatches();
}

int EditorSettingsBar::barHeight()
{
    return kBarHeight;
}

void EditorSettingsBar::rebuildSwatches()
{
    qDeleteAll(m_swatches);
    m_swatches.clear();
    m_colors = swatchColors(ConfigHandler().userColors().toList(), kMaxSwatches);
    for (const QColor& c : m_colors) {
        auto* b = new QToolButton(this);
        b->setFixedSize(kSwatchSize, kSwatchSize);
        b->setToolTip(c.name(c.alpha() < 255 ? QColor::HexArgb
                                              : QColor::HexRgb));
        b->setProperty("swatchColor", c);
        connect(b, &QToolButton::clicked, this, [this, c]() {
            emit colorPicked(c);
        });
        m_swatchLayout->addWidget(b);
        m_swatches.append(b);
    }
    refreshSelection();
}

// Draw each swatch with a ring on the current colour
void EditorSettingsBar::refreshSelection()
{
    for (QToolButton* b : m_swatches) {
        const QColor c = b->property("swatchColor").value<QColor>();
        const bool selected = m_current.isValid() && c.rgba() == m_current.rgba();
        b->setStyleSheet(
          QStringLiteral("QToolButton { background: rgba(%1,%2,%3,%4); "
                         "border: %5px solid %6; border-radius: %7px; }")
            .arg(c.red())
            .arg(c.green())
            .arg(c.blue())
            .arg(c.alpha())
            .arg(selected ? 3 : 1)
            .arg(selected ? QStringLiteral("#ffffff") : QStringLiteral("#666666"))
            .arg(kSwatchSize / 2));
    }
}

void EditorSettingsBar::pickCustomColor()
{
    const QColor chosen = QColorDialog::getColor(
      m_current.isValid() ? m_current : QColor(Qt::red),
      this,
      tr("Choose color"),
      QColorDialog::ShowAlphaChannel);
    if (chosen.isValid()) {
        emit colorPicked(chosen);
    }
}

void EditorSettingsBar::setColor(const QColor& color)
{
    m_current = color;
    refreshSelection();
}

void EditorSettingsBar::setSize(int size)
{
    const int clamped = clampThickness(size, 1, maxToolSize);
    QSignalBlocker blockSlider(m_slider);
    QSignalBlocker blockSpin(m_spin);
    m_slider->setValue(clamped);
    m_spin->setValue(clamped);
}
