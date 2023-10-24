/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ENV_H
#define ENV_H

#include "constants.h"

#ifdef MZ_WINDOWS
#  include "platforms/windows/windowsutils.h"
#endif

#include <QObject>

class Env final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Env)

  Q_PROPERTY(bool inProduction READ inProduction CONSTANT)
  Q_PROPERTY(QString devVersion READ devVersion CONSTANT)
  Q_PROPERTY(QString versionString READ versionString CONSTANT)
  Q_PROPERTY(QString buildNumber READ buildNumber CONSTANT)
  Q_PROPERTY(QString osVersion READ osVersion CONSTANT)
  Q_PROPERTY(QString architecture READ architecture CONSTANT)
  Q_PROPERTY(QString platform READ platform CONSTANT)

#ifdef UNIT_TEST
  Q_PROPERTY(
      bool debugMode READ debugMode WRITE setDebugMode NOTIFY debugModeChanged)
  Q_PROPERTY(bool stagingMode READ stagingMode WRITE setStagingMode NOTIFY
                 stagingModeChanged)
#else
  Q_PROPERTY(bool debugMode READ debugMode CONSTANT)
  Q_PROPERTY(bool stagingMode READ stagingMode CONSTANT)
#endif

 public:
  static Env* instance();

  ~Env() = default;

  static bool inProduction() { return Constants::inProduction(); }
  static QString versionString() { return Constants::versionString(); }
  static QString buildNumber() { return Constants::buildNumber(); }
  static QString osVersion() {
#ifdef MZ_WINDOWS
    return WindowsUtils::windowsVersion();
#else
    return QSysInfo::productVersion();
#endif
  }
  static QString architecture() { return QSysInfo::currentCpuArchitecture(); }
  static QString platform() { return Constants::PLATFORM_NAME; }
  static QString devVersion();

  static bool debugMode();
  static bool stagingMode();

#ifdef UNIT_TEST
  void setDebugMode(bool debugMode);
  void setStagingMode(bool stagingMode);

 signals:
  void debugModeChanged();
  void stagingModeChanged();
#endif

 private:
  Env() = default;
};

#endif  // ENV_H
