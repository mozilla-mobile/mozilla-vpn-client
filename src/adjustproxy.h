/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADJUSTPROXY_H
#define ADJUSTPROXY_H

#include <QTcpServer>

class AdjustProxy final : public QTcpServer {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AdjustProxy)

 public:
  explicit AdjustProxy(QObject* parent);
  ~AdjustProxy();

  quint16 proxyPort();

 private:
  void newConnectionReceived();

  quint16 m_port = 0;
};

#endif  // ADJUSTPROXY_H
