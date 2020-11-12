/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorconnection.h"
#include "logger.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"

#include <QQuickItem>
#include <QQuickWindow>
#include <QTcpSocket>
#include <QTest>

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

        QString command(line);
        command = command.trimmed();

        if (command.isEmpty()) {
            continue;
        }

        parseCommand(command);
    }
}

void InspectorConnection::parseCommand(const QString &command)
{
    Q_ASSERT(!command.isEmpty());

    logger.log() << "command received: " << command;

    QStringList parts = command.split(" ");
    Q_ASSERT(!parts.isEmpty());

    if (parts[0].trimmed() == "reset") {
        if (parts.length() != 1) {
            tooManyArguments(0);
            return;
        }

        m_connection->write("ok\n");
        MozillaVPN::instance()->reset();
        return;
    }

    if (parts[0].trimmed() == "quit") {
        if (parts.length() != 1) {
            tooManyArguments(0);
            return;
        }

        m_connection->write("ok\n");
        MozillaVPN::instance()->controller()->quit();
        return;
    }

    if (parts[0].trimmed() == "has") {
        if (parts.length() != 2) {
            tooManyArguments(1);
            return;
        }

        QQuickItem *obj = findObject(parts[1]);
        if (!obj) {
            m_connection->write("ko\n");
            return;
        }

        m_connection->write("ok\n");
        return;
    }

    if (parts[0].trimmed() == "property") {
        if (parts.length() != 3) {
            tooManyArguments(2);
            return;
        }

        QQuickItem *obj = findObject(parts[1]);
        if (!obj) {
            m_connection->write("ko\n");
            return;
        }

        QVariant property = obj->property(parts[2].toLocal8Bit());
        if (!property.isValid()) {
            m_connection->write("ko\n");
            return;
        }

        m_connection->write(
            QString("-%1-\n").arg(property.toString().toHtmlEscaped()).toLocal8Bit());
        return;
    }

    if (parts[0].trimmed() == "click") {
        if (parts.length() != 2) {
            tooManyArguments(1);
            return;
        }

        QQuickItem *obj = findObject(parts[1]);
        if (!obj) {
            m_connection->write("ko\n");
            return;
        }

        QPointF pointF = obj->mapToScene(QPoint(0, 0));
        QPoint point = pointF.toPoint();
        point.rx() += obj->width() / 2;
        point.ry() += obj->height() / 2;
        QTest::mouseClick(obj->window(), Qt::LeftButton, Qt::NoModifier, point);

        m_connection->write("ok\n");
        return;
    }

    m_connection->write("invalid command\n");
}

void InspectorConnection::tooManyArguments(int arguments)
{
    m_connection->write(QString("too many arguments (%1 expected)\n").arg(arguments).toLocal8Bit());
}

QQuickItem *InspectorConnection::findObject(const QString &name)
{
    QQmlApplicationEngine *engine = QmlEngineHolder::instance()->engine();
    for (QObject *rootObject : engine->rootObjects()) {
        if (!rootObject) {
            continue;
        }

        QQuickItem *obj = rootObject->findChild<QQuickItem *>(name);
        if (obj) {
            return obj;
        }
    }

    return nullptr;
}
