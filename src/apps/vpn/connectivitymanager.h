/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTIVITYMANAGER_H
#define CONNECTIVITYMANAGER_H

#include <QObject>

class MozillaVPN;

class ConnectivityManager final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ConnectivityManager)

 public:
  enum ConnectionState {
    // StateInitializing,
    // ...
  };
  Q_ENUM(ConnectionState)

 private:
    //  Q_PROPERTY(State state READ state NOTIFY stateChanged)

 public:
  ConnectivityManager();
  ~ConnectivityManager();

  void initialize();

 public slots:

 private slots:


 signals:

 private:
  // methods

 private:
  // members
  //  State m_state = StateInitializing;
};

#endif  // CONNECTIVITYMANAGER_H
