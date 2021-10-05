/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "adjustproxy.h"
#include "adjustproxyconnection.h"
#include "leakdetector.h"
#include "logger.h"

#include <QHostAddress>
#include <QTcpSocket>

namespace {
Logger logger(LOG_ADJUST, "AdjustProxy");
AdjustProxy* s_instance = nullptr;
}  // namespace

AdjustProxy::AdjustProxy(QObject* parent) : QTcpServer(parent) {
  MVPN_COUNT_CTOR(AdjustProxy);

  Q_ASSERT(!s_instance);
  s_instance = this;

  logger.debug() << "Creating the AdjustProxy server";
}

// static
AdjustProxy* AdjustProxy::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

AdjustProxy::~AdjustProxy() { MVPN_COUNT_DTOR(AdjustProxy); }

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
