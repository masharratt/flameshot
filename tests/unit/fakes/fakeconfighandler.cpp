// SPDX-License-Identifier: GPL-3.0-or-later

// Test-only ConfigHandler: an in-memory value store, so tool code that reads
// config links without the real settings file. It never reads or writes the
// user's configuration. Only the generic value()/setValue() pair is provided;
// the generated getters and setters in the header call through it.

#include "utils/confighandler.h"

#include <QHash>

namespace {
QHash<QString, QVariant>& store()
{
    static QHash<QString, QVariant> s;
    return s;
}
} // namespace

ConfigHandler::ConfigHandler()
  // A read-only resource path: the settings object can never write anywhere
  : m_settings(QStringLiteral(":/fake-config-never-written.ini"),
               QSettings::IniFormat)
{}

QVariant ConfigHandler::value(const QString& key) const
{
    return store().value(key);
}

void ConfigHandler::setValue(const QString& key, const QVariant& value)
{
    store().insert(key, value);
}
