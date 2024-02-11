/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIONCHECKER_H
#define CONNECTIONCHECKER_H

#include <QObject>

// Singleton that checks network connection
//
class ConnectionChecker : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ConnectionChecker)

 public:
  ConnectionChecker() = default;
  virtual ~ConnectionChecker() = default;

  static ConnectionChecker* instance();

  bool checkConnectionAndReport();

 protected:
  virtual bool checkInternetConnectionAndReport();
  virtual bool checkFirewallAndReport();
  virtual bool checkRoutingTableAndReport();
};

#endif  // CONNECTIONCHECKER_H