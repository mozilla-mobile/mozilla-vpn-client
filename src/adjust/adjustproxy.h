/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADJUSTPROXY_H
#define ADJUSTPROXY_H

#include <QSslServer>

class AdjustProxy final : public QSslServer {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AdjustProxy)

 public:
  explicit AdjustProxy(QObject* parent);
  ~AdjustProxy();

  bool initialize(quint16 port);

 private:
  void newConnectionReceived();
};

#endif  // ADJUSTPROXY_H
