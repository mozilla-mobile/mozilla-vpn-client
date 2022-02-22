/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionbenchmarkitem.h"
#include "l18nstrings.h"
#include "logger.h"

#include <QList>

namespace {
Logger logger(LOG_MODEL, "ConnectionBenchmarkItem");
}  // namespace

ConnectionBenchmarkItem::ConnectionBenchmarkItem(const QString& id,
                                                 const QString& displayName,
                                                 const QString& icon,
                                                 quint64 result)
    : m_id(id), m_displayName(displayName), m_icon(icon), m_result(result) {}

ConnectionBenchmarkItem::~ConnectionBenchmarkItem() {}

QString ConnectionBenchmarkItem::displayName() const {
  return m_displayName;
  // return L18nStrings::instance()->t(m_displayName_id);
}