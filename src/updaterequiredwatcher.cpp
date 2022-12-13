/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "updaterequiredwatcher.h"

#include "leakdetector.h"
#include "logger.h"
#include "moduleholder.h"
#include "mozillavpn.h"

namespace {
Logger logger("UpdateRequiredWatcher");
}

UpdateRequiredWatcher::UpdateRequiredWatcher(QObject* parent)
    : QObject(parent) {
  MVPN_COUNT_CTOR(UpdateRequiredWatcher);
  connect(this, &UpdateRequiredWatcher::readyToUpdate, this,
          &QObject::deleteLater);
}

UpdateRequiredWatcher::~UpdateRequiredWatcher() {
  MVPN_COUNT_DTOR(UpdateRequiredWatcher);
}

void UpdateRequiredWatcher::maybeReadyToUpdate() {
  Q_ASSERT(m_count > 0);
  --m_count;

  logger.debug() << "Pending modules:" << m_count;

  if (m_count == 0) {
    emit readyToUpdate();
  }
}

void UpdateRequiredWatcher::run() {
  if (!ModuleHolder::instance()->hasModules()) {
    logger.debug() << "We don't have modules. We can update.";
    emit readyToUpdate();
    return;
  }

  ModuleHolder::instance()->forEach([this](const QString&, Module* module) {
    m_count++;
    connect(module, &Module::readyToUpdate, this,
            &UpdateRequiredWatcher::maybeReadyToUpdate);
    module->updateRequired();
  });

  logger.debug() << "created with modules:" << m_count;
}
