/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorserver.h"
#include "inspectorconnection.h"
#include "leakdetector.h"
#include "logger.h"

#include <QTcpSocket>

namespace {
Logger logger(LOG_INSPECTOR, "InspectorServer");
}

constexpr int INSPECT_PORT = 8765;

InspectorServer::InspectorServer()
{
    MVPN_COUNT_CTOR(InspectorServer);

    logger.log() << "Creating the inspector server";

    if (!listen(QHostAddress::Any, INSPECT_PORT)) {
        logger.log() << "Failed to listen on port" << INSPECT_PORT;
        return;
    }

    connect(this, &InspectorServer::newConnection, this, &InspectorServer::newConnectionReceived);
}

InspectorServer::~InspectorServer()
{
    MVPN_COUNT_DTOR(InspectorServer);
}

void InspectorServer::newConnectionReceived()
{
    QTcpSocket *child = nextPendingConnection();
    Q_ASSERT(child);

    InspectorConnection *connection = new InspectorConnection(this, child);
    connect(child, &QTcpSocket::disconnected, connection, &QObject::deleteLater);
}
