/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WGSESSIONWORKER_H
#define WGSESSIONWORKER_H

#include <QByteArray>
#include <QRunnable>
#include <QThread>

class WgSessionMacos;

class WgEncryptWorker final : public QThread {
 public:
  WgEncryptWorker(WgSessionMacos* session, qintptr socket);
  ~WgEncryptWorker();

  void setMtu(int mtu) { m_mtu = mtu; }
  void shutdown();

 protected:
  void run() override;

  WgSessionMacos* m_session;
  QAtomicInt m_mtu;
  int m_socket;
};

class WgDecryptWorker final : public QRunnable {
 public:
  WgDecryptWorker(WgSessionMacos* session, const QByteArray& packet)
    : QRunnable(), m_session(session), m_packet(packet) {}

 protected:
  void run() override;

  WgSessionMacos* m_session;
  QByteArray m_packet;
};

#endif  // WGSESSIONWORKER_H
