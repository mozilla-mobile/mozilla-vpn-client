/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "adjustproxy.h"

#include <QHostAddress>
#include <QTcpSocket>

#include "adjustproxyconnection.h"
#include "leakdetector.h"
#include "logging/logger.h"

namespace {
Logger logger("AdjustProxy");
}  // namespace

AdjustProxy::AdjustProxy(QObject* parent) : QTcpServer(parent) {
  MZ_COUNT_CTOR(AdjustProxy);
  logger.debug() << "Creating the AdjustProxy server";
}

AdjustProxy::~AdjustProxy() { MZ_COUNT_DTOR(AdjustProxy); }

bool AdjustProxy::initialize(quint16 port) {
  if (!listen(QHostAddress::LocalHost, port)) {
    logger.error() << "Failed to listen on port: " << port;
    return false;
  }

  logger.debug() << "AdjustProxy listening on port " << serverPort();

  connect(this, &AdjustProxy::newConnection, this,
          &AdjustProxy::newConnectionReceived);

  return true;
}

void AdjustProxy::newConnectionReceived() {
  logger.debug() << "New Adjust Proxy connection received";

  QTcpSocket* child = nextPendingConnection();
  Q_ASSERT(child);

  AdjustProxyConnection* connection = new AdjustProxyConnection(this, child);
  connect(child, &QTcpSocket::disconnected, connection, &QObject::deleteLater);
}
