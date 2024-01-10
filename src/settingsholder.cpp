/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settingsholder.h"

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>

#include "context/env.h"
#include "logging/logger.h"
#include "utilities/leakdetector.h"
#ifdef UNIT_TEST
#  include "settings/settingsmanager.h"
#endif

namespace {

Logger logger("SettingsHolder");

SettingsHolder* s_instance = nullptr;
}  // namespace

// static
SettingsHolder* SettingsHolder::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

SettingsHolder::SettingsHolder() {
  MZ_COUNT_CTOR(SettingsHolder);

  Q_ASSERT(!s_instance);
  s_instance = this;

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
