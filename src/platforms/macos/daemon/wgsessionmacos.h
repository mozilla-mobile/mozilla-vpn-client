/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WGSESSIONMACOS_H
#define WGSESSIONMACOS_H

#include <QObject>
#include <QTimer>

#include "daemon/interfaceconfig.h"

struct wireguard_tunnel;

class WgSessionMacos final : public QObject {
  Q_OBJECT

 public:
 WgSessionMacos(const InterfaceConfig& config, QObject* parent = nullptr);
  ~WgSessionMacos();

  const QString& pubkey() const { return m_config.m_serverPublicKey; }
  qint64 lastHandshake() const;
  qint64 rxData() const;
  qint64 txData() const;

  void encrypt(const QByteArray& data);
  void netInput(const QByteArray& data);

 signals:
  void netOutput(const QByteArray& data);
  void decrypted(const QByteArray& data);

 public slots:
  void readyRead();

 private:
  void processResult(int op, const QByteArray& buf);
  void timeout();

  const InterfaceConfig m_config;
  QTimer m_timer;

  struct wireguard_tunnel* m_tunnel = nullptr;
};

#endif  // WGSESSIONMACOS_H