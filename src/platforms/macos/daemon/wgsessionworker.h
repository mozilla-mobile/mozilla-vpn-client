/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WGSESSIONWORKER_H
#define WGSESSIONWORKER_H

#include <QByteArray>
#include <QRunnable>

class WgSessionMacos;

class WgEncryptWorker final : public QRunnable {
 public:
  WgEncryptWorker(WgSessionMacos* session, const QByteArray& packet)
    : QRunnable(), m_session(session), m_packet(packet) {}

 protected:
  void run() override;

  WgSessionMacos* m_session;
  QByteArray m_packet;
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
