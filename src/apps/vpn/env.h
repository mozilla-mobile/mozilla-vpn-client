/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ENV_H
#define ENV_H

#include "constants.h"

#ifdef MVPN_WINDOWS
#  include "platforms/windows/windowscommons.h"
#endif

#include <QObject>

class Env final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Env)

  Q_PROPERTY(bool inProduction READ inProduction CONSTANT)
  Q_PROPERTY(QString versionString READ versionString CONSTANT)
  Q_PROPERTY(QString buildNumber READ buildNumber CONSTANT)
  Q_PROPERTY(QString osVersion READ osVersion CONSTANT)
  Q_PROPERTY(QString architecture READ architecture CONSTANT)
  Q_PROPERTY(QString platform READ platform CONSTANT)

 public:
  Env() = default;
  ~Env() = default;

  static bool inProduction() { return Constants::inProduction(); }
  static QString versionString() { return Constants::versionString(); }
  static QString buildNumber() { return Constants::buildNumber(); }
  static QString osVersion() {
#ifdef MVPN_WINDOWS
    return WindowsCommons::WindowsVersion();
#else
    return QSysInfo::productVersion();
#endif
  }
  static QString architecture() { return QSysInfo::currentCpuArchitecture(); }
  static QString platform() { return Constants::PLATFORM_NAME; }
};

#endif  // ENV_H
