/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "quitwatcher.h"

#include "leakdetector.h"
#include "logger.h"
#include "moduleholder.h"
#include "mozillavpn.h"

namespace {
Logger logger("QuitWatcher");
}

QuitWatcher::QuitWatcher(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(QuitWatcher);
  connect(this, &QuitWatcher::readyToQuit, this, &QObject::deleteLater);
}

QuitWatcher::~QuitWatcher() { MVPN_COUNT_DTOR(QuitWatcher); }

void QuitWatcher::maybeReadyToQuit() {
  Q_ASSERT(m_count > 0);
  --m_count;

  logger.debug() << "Pending modules:" << m_count;

  if (m_count == 0) {
    emit readyToQuit();
  }
}

void QuitWatcher::run() {
  if (!ModuleHolder::instance()->hasModules()) {
    logger.debug() << "We don't have modules. We can quit.";
    emit readyToQuit();
    return;
  }

  ModuleHolder::instance()->forEach([this](const QString&, Module* module) {
    m_count++;
    connect(module, &Module::readyToQuit, this, &QuitWatcher::maybeReadyToQuit);
    module->quit();
  });

  logger.debug() << "created with modules:" << m_count;
}
