/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorconnection.h"
#include "logger.h"

#include <QTcpSocket>

namespace {
Logger logger(LOG_INSPECTOR, "InspectorConnection");
}

InspectorConnection::InspectorConnection(QObject *parent, QTcpSocket *connection)
    : QObject(parent), m_connection(connection)
{
    logger.log() << "New connection received";

    Q_ASSERT(m_connection);
    connect(m_connection, &QTcpSocket::readyRead, this, &InspectorConnection::readData);
}

InspectorConnection::~InspectorConnection()
{
    logger.log() << "Connection released";
}

void InspectorConnection::readData()
{
    Q_ASSERT(m_connection);
    QByteArray input = m_connection->readAll();
    m_buffer.append(input);

    while (true) {
        int pos = m_buffer.indexOf("\n");
        if (pos == -1) {
            break;
        }

        QByteArray line = m_buffer.left(pos);
        m_buffer.remove(0, pos + 1);

        logger.log() << "LINE:" << line;
    }
}
