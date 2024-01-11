/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PING_H
#define PING_H

#include <QObject>

class Ping final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Ping)

 public:
  explicit Ping(int aId);

  Q_INVOKABLE void submit(const QString& reason = QString()) const;

 private:
  const int m_id;
};

#endif  // PING_H
