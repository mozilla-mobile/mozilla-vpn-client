/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOSLOGGER_H
#define IOSLOGGER_H

#include <QObject>

class IOSLogger {
  Q_DISABLE_COPY_MOVE(IOSLogger)

 public:
  IOSLogger(const QString& tag);
  ~IOSLogger();

  static void debug(const QString& message);
  static void info(const QString& message);
  static void error(const QString& message);
};

#endif  // IOSLOGGER_H
