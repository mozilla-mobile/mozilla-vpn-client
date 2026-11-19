/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tcppingsender.h"

#include <QTcpSocket>

#include "leakdetector.h"

TcpPingSender::TcpPingSender(quint16 port, QObject* parent)
    : PingSender(QHostAddress::AnyIPProtocol, parent), m_port(port) {
  MZ_COUNT_CTOR(TcpPingSender);
}

TcpPingSender::~TcpPingSender() { MZ_COUNT_DTOR(TcpPingSender); }

void TcpPingSender::sendPing(const QHostAddress& dest, quint16 sequence) {
  QTcpSocket* socket = new QTcpSocket(this);

  // Consider the ping to be recieved once the TCP handshake is complete.
  connect(socket, &QAbstractSocket::connected, this,
          [this, sequence] { emit recvPing(sequence); });

  // Cleanup the socket upon completion.
  connect(socket, &QAbstractSocket::connected, socket, &QObject::deleteLater);
  connect(socket, &QAbstractSocket::disconnected, socket,
          &QObject::deleteLater);
  connect(socket, &QAbstractSocket::errorOccurred, socket,
          &QObject::deleteLater);

  // Try to connect
  socket->connectToHost(dest, m_port);
}
