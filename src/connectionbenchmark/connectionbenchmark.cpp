/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionbenchmark.h"
#include "l18nstrings.h"
#include "logger.h"

#include <QScopeGuard>
#include <QList>

namespace {
Logger logger(LOG_MODEL, "ConnectionBenchmark");
}  // namespace

ConnectionBenchmark::ConnectionBenchmark(const QString& id, const QString& displayName, bool isAvailable) : m_id(id), m_displayName(displayName), m_isAvailable(isAvailable) {
  logger.debug() << "Initializing connection benchmark" << id;
}

ConnectionBenchmark::~ConnectionBenchmark() { }

bool ConnectionBenchmark::isAvailable() const {
  logger.debug() << "Is available" << m_id;

  return m_isAvailable;
}

QString ConnectionBenchmark::displayName() const {
  return m_displayName;
  // return L18nStrings::instance()->t(m_displayName_id);
}