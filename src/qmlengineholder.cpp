/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "qmlengineholder.h"
#include "logger.h"

#include <QWindow>

namespace {
Logger logger(LOG_MAIN, "QmlEngineHolder");
QmlEngineHolder *s_instance = nullptr;
} // namespace

// static
void QmlEngineHolder::createInstance(QObject *parent)
{
    logger.log() << "Creating QmlEngineHolder";

    Q_ASSERT(!s_instance);
    s_instance = new QmlEngineHolder(parent);
}

// static
QmlEngineHolder *QmlEngineHolder::instance()
{
    Q_ASSERT(s_instance);
    return s_instance;
}

QmlEngineHolder::QmlEngineHolder(QObject *parent) : QObject(parent) {}

QNetworkAccessManager *QmlEngineHolder::networkAccessManager()
{
    return m_engine.networkAccessManager();
}

void QmlEngineHolder::showWindow()
{
    QObject *rootObject = m_engine.rootObjects().first();
    QWindow *window = qobject_cast<QWindow *>(rootObject);
    Q_ASSERT(window);

    window->show();
    window->raise();
    window->requestActivate();
}
