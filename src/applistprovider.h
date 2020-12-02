/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef APPLISTPROVIDER_H
#define APPLISTPROVIDER_H

#include "controller.h"
#include <QMap>
#include <QObject>

class AppListProvider : public QObject
{
    Q_OBJECT

public:
    AppListProvider() = default;

    virtual ~AppListProvider() = default;


    // Requests a fresh Application List
    // Impl should emit newAppList signal when done.
    virtual void getApplicationList() =0;

signals:
    // Should be emitted when the new Application List is Ready
    // QMap key should be the identifier that the controller can
    // use on activation.
    // QMap Value should be a User readable Name of the app
    void newAppList(QMap<QString,QString> applist);

};


#endif // APPLISTPROVIDER_H
