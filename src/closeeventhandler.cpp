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
    for (int i = m_layers.length() - 1; i >= 0; --i) {
        const Layer &layer = m_layers.at(i);

        if (layer.m_type == Layer::eStackView) {
            QVariant property = layer.m_layer->property("depth");
            if (!property.isValid()) {
                logger.log() << "Invalid depth property!!";
                continue;
            }

            int depth = property.toInt();
            if (depth > 1) {
                emit goBack(layer.m_layer);
                return true;
            }

            continue;
        }

        Q_ASSERT(layer.m_type == Layer::eView);
        QVariant property = layer.m_layer->property("visible");
        if (!property.isValid()) {
            logger.log() << "Invalid visible property!!";
            continue;
        }

        bool visible = property.toBool();
        if (visible) {
            emit goBack(layer.m_layer);
            return true;
        }

        continue;
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
    m_layers.append(Layer(item, Layer::eStackView));
}

void CloseEventHandler::addView(const QVariant &view)
{
    logger.log() << "Add view";

    QQuickItem *item = qobject_cast<QQuickItem *>(view.value<QObject *>());
    Q_ASSERT(item);

    connect(item, &QObject::destroyed, this, &CloseEventHandler::removeItem);
    m_layers.append(Layer(item, Layer::eView));
}

void CloseEventHandler::removeItem(QObject *item)
{
    logger.log() << "Remove item";
    Q_ASSERT(item);

#ifdef QT_DEBUG
    bool found = false;
#endif

    for (int i = 0; i < m_layers.length(); ++i) {
        if (m_layers.at(i).m_layer == item) {
            m_layers.removeAt(i);
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
