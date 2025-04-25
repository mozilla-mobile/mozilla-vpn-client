/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDPEERMACOS_H
#define WIREGUARDPEERMACOS_H

#include <QObject>
#include <QTimer>

struct wireguard_tunnel;
class InterfaceConfig;

class WireguardPeerMacos final : public QObject {
  Q_OBJECT

 public:
  WireguardPeerMacos(const InterfaceConfig& config, QObject* parent = nullptr);
  ~WireguardPeerMacos();

  const QString& pubkey() const { return m_pubkey; }
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

  QString m_pubkey;
  QTimer m_timer;

  struct wireguard_tunnel* m_tunnel = nullptr;
};

#endif  // WIREGUARDPEERMACOS_H