/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RELEASEMONITOR_H
#define RELEASEMONITOR_H

#include <QObject>

class ReleaseMonitor final : public QObject
{
    Q_OBJECT

public:
    void runSoon();

private:
    void runInternal();

    void schedule();

    [[nodiscard]] bool processData(const QByteArray &data);
};

#endif // RELEASEMONITOR_H
