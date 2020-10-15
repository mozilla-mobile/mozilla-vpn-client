/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PINGSENDER_H
#define PINGSENDER_H

#include <QObject>
#include <QThread>

class PingSender final : public QObject
{
    Q_OBJECT

public:
    explicit PingSender(QObject *parent);

    ~PingSender();

    void send(const QString &destination);

    void stop();

signals:
    void completed();

    // internal only
    void sendPing(const QString &destination);
    void stopPing();

private slots:
    void pingFailed();
    void pingSucceeded();

private:
    QThread m_workerThread;

    bool m_active = false;
};

#endif // PINGSENDER_H
