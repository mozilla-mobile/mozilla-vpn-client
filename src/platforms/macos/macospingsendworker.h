/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSPINGSENDWORKER_H
#define MACOSPINGSENDWORKER_H

#include "pingsendworker.h"

class QSocketNotifier;

class MacOSPingSendWorker final : public PingSendWorker
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MacOSPingSendWorker)

public:
    MacOSPingSendWorker();
    ~MacOSPingSendWorker();

public slots:
    void sendPing(const QString &destination) override;

private:
    void releaseObjects();

private:
    QSocketNotifier *m_socketNotifier = nullptr;
    int m_socket = 0;
};

#endif // MACOSPINGSENDWORKER_H
