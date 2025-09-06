/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "systraymanager.h"

#include "utils/leakdetector.h"
#include "utils/logger.h"

namespace {
Logger logger("SysTrayManager");
}

SysTrayManager* SysTrayManager::s_instance = nullptr;

SysTrayManager::SysTrayManager(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(SysTrayManager);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

SysTrayManager::~SysTrayManager() {
  MZ_COUNT_DTOR(SysTrayManager);

  if (m_sysTray) {
    m_sysTray->deleteLater();
  }

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

// static
SysTrayManager* SysTrayManager::instance() {
  if (!s_instance) {
    s_instance = new SysTrayManager();
  }
  return s_instance;
}

void SysTrayManager::initialize() {
  logger.debug() << "Initializing SysTrayManager";

  if (!m_sysTray) {
    m_sysTray = new SysTray(this);
    m_sysTray->initialize();
  }
}

void SysTrayManager::showSysTray() {
  logger.debug() << "Showing SysTray";

  if (!m_sysTray) {
    initialize();
  }

  if (m_sysTray) {
    // The systray should show/hide its window when clicked
    // This method can be used to programmatically show it
  }
}

void SysTrayManager::hideSysTray() {
  logger.debug() << "Hiding SysTray";

  if (m_sysTray) {
    // This method can be used to programmatically hide it
  }
}

bool SysTrayManager::isInitialized() const { return m_sysTray != nullptr; }
