/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WGSESSIONWORKER_H
#define WGSESSIONWORKER_H

#include <QByteArray>
#include <QRunnable>
#include <QThread>

class WgSessionMacos;
struct iovec;

class WgSessionWorker : public QThread {
  Q_OBJECT

 public:
  WgSessionWorker(WgSessionMacos* session, qintptr socket);
  ~WgSessionWorker();

  void setMtu(int mtu);
  void stop();

 protected:
  void run() override = 0;
  void wakeup(int sig) const;
  int readSocket(struct iovec* iovec, int iovlen);

  WgSessionMacos* m_session;
  QAtomicInt m_mtu;
  int m_socket;
  int m_txWakeup;
  int m_rxWakeup;
};

class WgEncryptWorker final : public WgSessionWorker {
  Q_OBJECT

 public:
  WgEncryptWorker(WgSessionMacos* s, qintptr sd) : WgSessionWorker(s, sd) {}

 protected:
  void run() override;
};

class WgDecryptWorker final : public WgSessionWorker {
  Q_OBJECT

 public:
  WgDecryptWorker(WgSessionMacos* s, qintptr sd) : WgSessionWorker(s, sd) {}

 protected:
  void run() override;
};

class WgMultihopWorker final : public WgSessionWorker {
  Q_OBJECT

 public:
  WgMultihopWorker(WgSessionMacos* s, qintptr sd) : WgSessionWorker(s, sd) {}

 protected:
  void run() override;
};

#endif  // WGSESSIONWORKER_H
