/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOGOUTOBSERVER_H
#define LOGOUTOBSERVER_H

#include <QObject>

class LogoutObserver : public QObject
{
    Q_OBJECT
public:
    explicit LogoutObserver(QObject *parent);

signals:
    void ready();
};

#endif // LOGOUTOBSERVER_H
