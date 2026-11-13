/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSHELPERPROCESS_H
#define MACOSHELPERPROCESS_H

#include <QProcess>
#include <QTemporaryDir>

#include "logger.h"

class MacOSHelperProcess final : public QProcess {
  Q_OBJECT

 public:
  MacOSHelperProcess(const QString& name, QObject* parent = nullptr);
  ~MacOSHelperProcess();

  void start();
  QProcess::ProcessError error() const;

 private:
  static void dropPermissions();
  static QStringList codesignRequirements();
  OSStatus enforceCodesign();
  void stdoutReady();
  void stderrReady();

  QStringList m_codesignRequires;
  QTemporaryDir m_tempdir;
  OSStatus m_codesignResult;
  Logger m_logger;
};

#endif // MACOSHELPERPROCESS_H
