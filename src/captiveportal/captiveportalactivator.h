/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALACTIVATOR_H
#define CAPTIVEPORTALACTIVATOR_H

#include <QObject>

class CaptivePortalActivator : public QObject
{
    Q_OBJECT

public:
    explicit CaptivePortalActivator(QObject *parent);

    void run();

private Q_SLOTS:
    void checkStatus();
};

#endif // CAPTIVEPORTALACTIVATOR_H
