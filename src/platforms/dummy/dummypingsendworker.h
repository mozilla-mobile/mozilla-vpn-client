/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DUMMYPINGSENDWORKER_H
#define DUMMYPINGSENDWORKER_H

#include "pingsendworker.h"

class QTimer;

class DummyPingSendWorker final : public PingSendWorker
{
    Q_OBJECT

public:
    enum State {
        Stable,
        Unstable,
        NoSignal,
    };

    explicit DummyPingSendWorker(State state);

public Q_SLOTS:
    void sendPing(const QString &destination) override;
    void stopPing() override;

private:
    QTimer *m_timer = nullptr;

    State m_state;
};

#endif // DUMMYPINGSENDWORKER_H
