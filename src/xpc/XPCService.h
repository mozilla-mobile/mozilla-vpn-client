/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XPCSERVICE_H
#define XPCSERVICE_H

#include <QObject>

class XPCService : QObject final {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(XPCService)

 public:
  explicit XPCService(QObject* parent) : QObject(parent) {}
  ~XPCService() = default;

 private:
};

#endif XPCSERVICE_H
