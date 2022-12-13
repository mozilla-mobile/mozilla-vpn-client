/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "backendfailurewatcher.h"
#include "leakdetector.h"
#include "logger.h"
#include "moduleholder.h"
#include "mozillavpn.h"

namespace {
Logger logger("BackendFailureWatcher");
}

BackendFailureWatcher::BackendFailureWatcher(QObject* parent)
    : QObject(parent) {
  MVPN_COUNT_CTOR(BackendFailureWatcher);
  connect(this, &BackendFailureWatcher::readyToBackendFailure, this,
          &QObject::deleteLater);
}

BackendFailureWatcher::~BackendFailureWatcher() {
  MVPN_COUNT_DTOR(BackendFailureWatcher);
}

void BackendFailureWatcher::maybeReadyToBackendFailure() {
  Q_ASSERT(m_count > 0);
  --m_count;

  logger.debug() << "Pending modules:" << m_count;

  if (m_count == 0) {
    emit readyToBackendFailure();
  }
}

void BackendFailureWatcher::run() {
  if (!ModuleHolder::instance()->hasModules()) {
    logger.debug()
        << "We don't have modules. We can propagate the backend failure.";
    emit readyToBackendFailure();
    return;
  }

  ModuleHolder::instance()->forEach([this](const QString&, Module* module) {
    m_count++;
    connect(module, &Module::readyToBackendFailure, this,
            &BackendFailureWatcher::maybeReadyToBackendFailure);
    module->backendFailure();
  });

  logger.debug() << "created with modules:" << m_count;
}
