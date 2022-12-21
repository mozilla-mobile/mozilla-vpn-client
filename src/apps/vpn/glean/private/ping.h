/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PING_H
#define PING_H

#include <QObject>

class Ping final {
  Q_GADGET

 public:
  // QML custom types require these three declarations.
  // See: https://doc.qt.io/qt-6/custom-types.html#creating-a-custom-type
  Ping() = default;
  Ping(const Ping&) = default;
  Ping& operator=(const Ping&) = default;

  explicit Ping(int aId);
  ~Ping() = default;

  Q_INVOKABLE void submit() const;

 private:
  int m_id;
};

#endif  // PING_H
