/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SOCKS5PROXYHANDLER_H
#define SOCKS5PROXYHANDLER_H

#include <QObject>

class Socks5;

class Socks5ProxyHandler final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Socks5ProxyHandler)
  Q_PROPERTY(uint16_t connections READ connections NOTIFY connectionsChanged);

 public:
  static Socks5ProxyHandler* instance();

  ~Socks5ProxyHandler();

  uint16_t connections() const;

 signals:
  void connectionsChanged();

 private:
  explicit Socks5ProxyHandler(QObject* parent);

  void stateChanged();
  void maybeTurnOff();
  void maybeTurnOn();

 private:
  Socks5* m_socks5Proxy = nullptr;
};

#endif  // SOCKS5PROXYHANDLER_H
