/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALREQUEST_H
#define CAPTIVEPORTALREQUEST_H

#include <QObject>

class CaptivePortalRequest : public QObject
{
    Q_OBJECT

public:
    explicit CaptivePortalRequest(QObject *parent);

    void run();

signals:
    void completed(bool detected);
};

#endif // CAPTIVEPORTALREQUEST_H
