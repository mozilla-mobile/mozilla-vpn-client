#ifndef RELEASEMONITOR_H
#define RELEASEMONITOR_H

#include <QObject>

class MozillaVPN;
class QSettings;

class ReleaseMonitor final : public QObject
{
    Q_OBJECT
public:
    void init(MozillaVPN *vpn, QSettings *settings);

    void run();

private:
    void schedule();

    void processData(const QByteArray &data);

    void maybeForceUpdate();

private:
    MozillaVPN *m_vpn = nullptr;
    QSettings *m_settings = nullptr;

    double m_latestVersion = 0;
    double m_minimumVersion = 0;
};

#endif // RELEASEMONITOR_H
