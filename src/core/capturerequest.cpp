// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "capturerequest.h"

#include <stdexcept>
#include <utility>

CaptureRequest::CaptureRequest(CaptureRequest::CaptureMode mode,
                               const uint delay,
                               QVariant data,
                               CaptureRequest::ExportTask tasks)
  : m_mode(mode)
  , m_delay(delay)
  , m_tasks(tasks)
  , m_data(std::move(data))
  , m_selectedMonitor(-1)
  , m_hasSelectedMonitor(false)
{}

CaptureRequest::CaptureMode CaptureRequest::captureMode() const
{
    return m_mode;
}

uint CaptureRequest::delay() const
{
    return m_delay;
}

QString CaptureRequest::path() const
{
    return m_path;
}

QVariant CaptureRequest::data() const
{
    return m_data;
}

CaptureRequest::ExportTask CaptureRequest::tasks() const
{
    return m_tasks;
}

QRect CaptureRequest::initialSelection() const
{
    return m_initialSelection;
}

QPixmap CaptureRequest::presetScreenshot() const
{
    return m_presetScreenshot;
}

bool CaptureRequest::overwriteExisting() const
{
    return m_overwriteExisting;
}

bool CaptureRequest::captureFirst() const
{
    return m_captureFirst;
}

QString CaptureRequest::workflow() const
{
    return m_workflow;
}

void CaptureRequest::setWorkflow(const QString& workflow)
{
    m_workflow = workflow;
}

CaptureRequest::RecordMode CaptureRequest::recordMode() const
{
    return m_recordMode;
}

void CaptureRequest::setRecordMode(RecordMode mode)
{
    m_recordMode = mode;
}

void CaptureRequest::setCaptureFirst(bool captureFirst)
{
    m_captureFirst = captureFirst;
}

void CaptureRequest::setPresetScreenshot(const QPixmap& screenshot)
{
    m_presetScreenshot = screenshot;
}

void CaptureRequest::setOverwriteExisting(bool overwrite)
{
    m_overwriteExisting = overwrite;
}

void CaptureRequest::addTask(CaptureRequest::ExportTask task)
{
    if (task == SAVE) {
        throw std::logic_error("SAVE task must be added using addSaveTask");
    }
    m_tasks |= task;
}

void CaptureRequest::removeTask(ExportTask task)
{
    ((int&)m_tasks) &= ~task;
}

void CaptureRequest::addSaveTask(const QString& path)
{
    m_tasks |= SAVE;
    m_path = path;
}

void CaptureRequest::addPinTask(const QRect& pinWindowGeometry)
{
    m_tasks |= PIN;
    m_pinWindowGeometry = pinWindowGeometry;
}

void CaptureRequest::setInitialSelection(const QRect& selection)
{
    m_initialSelection = selection;
}

void CaptureRequest::setSelectedMonitor(int monitorIndex)
{
    m_selectedMonitor = monitorIndex;
    m_hasSelectedMonitor = true;
}

int CaptureRequest::selectedMonitor() const
{
    return m_selectedMonitor;
}

bool CaptureRequest::hasSelectedMonitor() const
{
    return m_hasSelectedMonitor;
}

QRect CaptureRequest::capturedGlobalRect() const
{
    return m_capturedGlobalRect;
}

void CaptureRequest::setCapturedGlobalRect(const QRect& rect)
{
    m_capturedGlobalRect = rect;
}
