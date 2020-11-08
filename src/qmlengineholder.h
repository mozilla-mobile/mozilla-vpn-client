/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef QMLENGINEHOLDER_H
#define QMLENGINEHOLDER_H

#include <QObject>
#include <QQmlApplicationEngine>

class QNetworkAccessManager;

class QmlEngineHolder final : public QObject
{
public:
    QmlEngineHolder();
    ~QmlEngineHolder();

    static QmlEngineHolder *instance();

    QQmlApplicationEngine *engine() { return &m_engine; }

    QNetworkAccessManager *networkAccessManager();

    void showWindow();

private:
    QQmlApplicationEngine m_engine;
};

#endif // QMLENGINEHOLDER_H
