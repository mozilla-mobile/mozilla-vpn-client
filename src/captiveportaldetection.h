#ifndef CAPTIVEPORTALDETECTION_H
#define CAPTIVEPORTALDETECTION_H

#include <QObject>
#include <QTimer>

class CaptivePortalDetection final : public QObject
{
    Q_OBJECT

public:
    CaptivePortalDetection();

private:
    void handleSuccess();
    void handleFailure();

public Q_SLOTS:
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
