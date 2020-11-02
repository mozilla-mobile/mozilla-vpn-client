/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TIMERSINGLESHOT_H
#define TIMERSINGLESHOT_H

#include <functional>
#include <QObject>

// Like QTimer::singleShot but with a parent object.

class TimerSingleShot
{
public:
    static void create(QObject *parent, uint32_t timer, std::function<void()> &&callback);
};

#endif // TIMERSINGLESHOT_H
