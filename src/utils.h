/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QStandardPaths>

class Utils final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Utils)

 public:
  static Utils* instance();
  ~Utils() = default;

  Q_INVOKABLE void exitForUnrecoverableError(const QString& reason);

  Q_INVOKABLE static void storeInClipboard(const QString& text);

  Q_INVOKABLE static void crashTest();

  Q_INVOKABLE bool viewLogs();

#ifdef MZ_ANDROID
  Q_INVOKABLE void launchPlayStore();
#endif

 private:
  Utils() = default;

  bool writeAndShowLogs(const QString& location);
  bool writeAndShowLogs(QStandardPaths::StandardLocation location) {
    return writeAndShowLogs(QStandardPaths::writableLocation(location));
  }
};

#endif  // UTILS_H
