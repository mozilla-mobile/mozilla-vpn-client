/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTORCONNECTION_H
#define INSPECTORCONNECTION_H

#include <QByteArray>
#include <QObject>

class QTcpSocket;
class QQuickItem;

class InspectorConnection : public QObject
{
    Q_OBJECT

public:
    InspectorConnection(QObject *parent, QTcpSocket *connection);
    ~InspectorConnection();

private slots:
    void readData();
    void parseCommand(const QString &command);
    void tooManyArguments(int arguments);
    QObject *findObject(const QString &name);

private:
    QTcpSocket *m_connection;

    QByteArray m_buffer;
};

#endif // INSPECTORCONNECTION_H
