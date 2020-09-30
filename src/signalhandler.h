/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <QObject>

class SignalHandler : public QObject
{
    Q_OBJECT

public:
    SignalHandler();

private:
    static void saHandler(int signal);

signals:
    void quitRequested();
};

#endif // SIGNALHANDLER_H
