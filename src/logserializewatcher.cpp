/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "logserializewatcher.h"
#include "leakdetector.h"
#include "logger.h"
#include "moduleholder.h"
#include "mozillavpn.h"

namespace {
Logger logger(LOG_MAIN, "LogSerializeWatcher");
}

LogSerializeWatcher::LogSerializeWatcher(QObject* parent, QTextStream* out)
    : QObject(parent) {
  MVPN_COUNT_CTOR(LogSerializeWatcher);

  if (!ModuleHolder::instance()->hasModules()) {
    logger.debug() << "We don't have modules. We can send logs.";
    QTimer::singleShot(0, this, &LogSerializeWatcher::logsReady);
    return;
  }

  ModuleHolder::instance()->forEach(
      [out, this](const QString&, Module* module) {
        m_count++;
        module->serializeLogs(out, [this]() { maybeLogsReady(); });
      });

  logger.debug() << "created with modules:" << m_count;
}

LogSerializeWatcher::~LogSerializeWatcher() {
  MVPN_COUNT_DTOR(LogSerializeWatcher);
}

void LogSerializeWatcher::maybeLogsReady() {
  Q_ASSERT(m_count > 0);
  logger.debug() << "Pending modules:" << --m_count;

  if (m_count == 0) {
    emit logsReady();
    deleteLater();
  }
}
