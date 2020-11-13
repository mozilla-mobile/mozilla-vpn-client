/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CLOSEEVENTHANDLER_H
#define CLOSEEVENTHANDLER_H

#include <QList>
#include <QObject>

class QQuickItem;

class CloseEventHandler : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE bool eventHandled();
    Q_INVOKABLE void addStackView(const QVariant &stackView);

signals:
    void goBack(QQuickItem *item);

private slots:
    void removeItem(QObject *item);

private:
    QList<QQuickItem *> m_stackViews;
};

#endif // CLOSEEVENTHANDLER_H
