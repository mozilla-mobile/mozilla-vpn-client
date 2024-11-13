/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINFWPOLICY_H
#define WINFWPOLICY_H

#include <QObject>
#include <QUuid>

class Socks5;

class WinFwPolicy final : public QObject {
  Q_OBJECT

 public:
  static WinFwPolicy* create(Socks5* proxy);
  ~WinFwPolicy();

  bool isValid() const { return m_fwEngineHandle != nullptr; }

 private:
  WinFwPolicy(QObject* parent = nullptr);

  void restrictProxyPort(quint16 port);
  void fwpmSublayerChanged(uint changeType, const QUuid& subLayerKey);

 private:
  void* m_fwEngineHandle = nullptr;
  void* m_fwChangeHandle = nullptr;
};

#endif  // WINFWPOLICY_H
