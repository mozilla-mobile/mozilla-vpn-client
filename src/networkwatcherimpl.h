/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETWORKWATCHERIMPL_H
#define NETWORKWATCHERIMPL_H

#include <QObject>

class NetworkWatcherImpl : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(NetworkWatcherImpl)

 public:
  NetworkWatcherImpl(QObject* parent) : QObject(parent) {}

  virtual ~NetworkWatcherImpl() = default;

  virtual void initialize() = 0;

  virtual void start() = 0;
  virtual void stop() = 0;

 signals:
  void unsecuredNetwork(const QString& networkName, const QString& networkId);
};

#endif  // NETWORKWATCHERIMPL_H
