/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XPC_CLIENT_H
#define XPC_CLIENT_H

#include <QObject>

class XPCClient : QObject final {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(XPCClient)

 public:
  explicit XPCClient(QObject* parent) : QObject(parent) {}
  ~XPCClient() = default;

 private:
};

#endif XPC_CLIENT_H
