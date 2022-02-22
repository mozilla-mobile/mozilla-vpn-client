/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionbenchmarkitem.h"
#include "l18nstrings.h"
#include "logger.h"

#include <QScopeGuard>
#include <QList>

namespace {
Logger logger(LOG_MODEL, "ConnectionBenchmarkItem");
}  // namespace

ConnectionBenchmarkItem::ConnectionBenchmarkItem(const QString& id,
                                                 const QString& displayName,
                                                 quint64 result)
    : m_id(id), m_displayName(displayName), m_result(result) {
  logger.debug() << "Initializing connection benchmark" << id;
}

ConnectionBenchmarkItem::~ConnectionBenchmarkItem() {}

QString ConnectionBenchmarkItem::displayName() const {
  return m_displayName;
  // return L18nStrings::instance()->t(m_displayName_id);
}