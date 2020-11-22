/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALDETECTION_H
#define CAPTIVEPORTALDETECTION_H

#include <QObject>
#include <QTimer>

class CaptivePortalDetection final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(CaptivePortalDetection)

public:
    CaptivePortalDetection();
    ~CaptivePortalDetection();

    void initialize();

private:
    void handleSuccess();
    void handleFailure();

public slots:
    void controllerStateChanged();

    void settingsChanged();

    void detectCaptivePortal();

signals:
    void captivePortalDetected();

private:
    QTimer m_timer;
    bool m_active = false;
};

#endif // CAPTIVEPORTALDETECTION_H

