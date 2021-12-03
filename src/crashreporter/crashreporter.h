/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CRASHREPORTER_H
#define CRASHREPORTER_H

#include <QObject>

#include "crashdata.h"

class CrashData;

class CrashReporter : public QObject
{
    Q_OBJECT
public:
    explicit CrashReporter(QObject *parent = nullptr);
    virtual bool start() = 0;
    virtual void stop() {};
    virtual bool shouldPromptUser();
public slots:
    void crashReported(const CrashData &data);


};

#endif // CRASHREPORTER_H
