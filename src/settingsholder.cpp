/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settingsholder.h"

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>

#include "env.h"
#include "leakdetector.h"
#include "logger.h"
#ifdef UNIT_TEST
#  include "settings/settingsmanager.h"
#endif

namespace {

Logger logger("SettingsHolder");
SettingsHolder* s_instance = nullptr;
}  // namespace

// static
SettingsHolder* SettingsHolder::instance() {
  if (!s_instance) {
    s_instance = new SettingsHolder();
    qAddPostRoutine([]() { delete s_instance; });
  }
  return s_instance;
}

#ifdef UNIT_TEST
// static
void SettingsHolder::testCleanup() {
  if (s_instance) {
    delete s_instance;
  }
}
#endif

SettingsHolder::SettingsHolder() {
  MZ_COUNT_CTOR(SettingsHolder);
  logger.debug() << "Initializing SettingsHolder";
#define SETTING(type, toType, getter, ...)     \
  connect(m_##getter, &Setting::changed, this, \
          [&]() { emit getter##Changed(); });

#include "settingslist.h"
#undef SETTING

  if (!hasInstallationTime()) {
    m_firstExecution = true;
    setInstallationTime(QDateTime::currentDateTime());
  }

  if (!hasUpdateTime() || !hasInstalledVersion() ||
      installedVersion() != Env::versionString()) {
    setUpdateTime(QDateTime::currentDateTime());
    setInstalledVersion(Env::versionString());
  }
}

SettingsHolder::~SettingsHolder() {
  MZ_COUNT_DTOR(SettingsHolder);

  logger.debug() << "Destroying SettingsHolder";

#ifdef UNIT_TEST
  // Tie the lifetime of the SettingsManager singleton to the SettingsHolder
  // singleton.
  SettingsManager::testCleanup();
#endif
  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}