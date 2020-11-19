/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NOTIFICATIONHANDLER_H
#define NOTIFICATIONHANDLER_H

#include <QObject>

class NotificationHandler : public QObject
{
    Q_OBJECT
public:
    static NotificationHandler *create(QObject *parent);

    virtual ~NotificationHandler() = default;

public slots:
    void showNotification();

protected:
    explicit NotificationHandler(QObject *parent);

    virtual void notify(const QString &title, const QString &message) = 0;
};

#endif // NOTIFICATIONHANDLER_H
