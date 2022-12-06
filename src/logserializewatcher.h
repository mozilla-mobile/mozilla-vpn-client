/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOGSERIALIZEWATCHER_H
#define LOGSERIALIZEWATCHER_H

#include <QObject>

class QTextStream;

class LogSerializeWatcher final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(LogSerializeWatcher)

 public:
  LogSerializeWatcher(QObject* parent, QTextStream* output);
  ~LogSerializeWatcher();

 private:
  void maybeLogsReady();

 signals:
  void logsReady();

 private:
  int m_count = 0;
};

#endif  // LOGSERIALIZEWATCHER_H
