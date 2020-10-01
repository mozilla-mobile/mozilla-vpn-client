#ifndef CAPTIVEPORTALDETECTION_H
#define CAPTIVEPORTALDETECTION_H

#include <QObject>
#include <QTimer>

class CaptivePortalDetection final : public QObject
{
    Q_OBJECT

public:
    CaptivePortalDetection();

    void start();
    void stop();

private:
    void handleSuccess();
    void handleFailure();

signals:
    void captivePortalDetected();

private:
    QTimer m_timer;
    bool m_active = false;
};

#endif // CAPTIVEPORTALDETECTION_H
