// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "capturetoolobjects.h"
#include <QUndoCommand>

class CaptureWidget;

class ModificationCommand : public QUndoCommand
{
public:
    ModificationCommand(CaptureWidget* captureWidget,
                        const CaptureToolObjects& captureToolObjects,
                        const CaptureToolObjects& captureToolObjectsBackup);

    virtual void undo() override;
    virtual void redo() override;

private:
    CaptureToolObjects m_captureToolObjects;
    CaptureToolObjects m_captureToolObjectsBackup;
    CaptureWidget* m_captureWidget;
};

// Windowed editor: one click of Expand canvas. Redo adds the step on every
// edge, undo removes it again. Annotations are shifted by the same step, so
// the stack of earlier commands stays valid.
class ExpandCanvasCommand : public QUndoCommand
{
public:
    ExpandCanvasCommand(CaptureWidget* captureWidget, int stepLogical);

    void undo() override;
    void redo() override;

private:
    CaptureWidget* m_captureWidget;
    int m_stepLogical;
};
