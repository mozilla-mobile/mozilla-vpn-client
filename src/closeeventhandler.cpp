/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "closeeventhandler.h"
#include "logger.h"

#include <QQuickItem>

namespace {
Logger logger(LOG_MAIN, "CloseEventHandler");
}

bool CloseEventHandler::eventHandled()
{
    logger.log() << "Close event handled";

#ifdef MVPN_ANDROID
    for (int i = m_stackViews.length() - 1; i >= 0; --i) {
        QQuickItem *stackView = m_stackViews.at(i);
        QVariant property = stackView->property("depth");
        if (!property.isValid()) {
            logger.log() << "Invalid depth property!!";
            continue;
        }

        int depth = property.toInt();
        if (depth > 1) {
            emit goBack(stackView);
            return true;
        }
    }
#endif

    return false;
}

void CloseEventHandler::addStackView(const QVariant &stackView)
{
    logger.log() << "Add stack view";

    QQuickItem *item = qobject_cast<QQuickItem *>(stackView.value<QObject *>());
    Q_ASSERT(item);

    connect(item, &QObject::destroyed, this, &CloseEventHandler::removeItem);
    m_stackViews.append(item);
}

void CloseEventHandler::removeItem(QObject *item)
{
    logger.log() << "Remove item";
    Q_ASSERT(item);

#ifdef QT_DEBUG
    bool found = false;
#endif

    for (int i = 0; i < m_stackViews.length(); ++i) {
        if (m_stackViews.at(i) == item) {
            m_stackViews.removeAt(i);
#ifdef QT_DEBUG
            found = true;
#endif
            break;
        }
    }

#ifdef QT_DEBUG
    Q_ASSERT(found);
#endif
}
