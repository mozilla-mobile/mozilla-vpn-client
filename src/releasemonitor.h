#ifndef RELEASEMONITOR_H
#define RELEASEMONITOR_H

#include <QObject>

class MozillaVPN;
class QSettings;

class ReleaseMonitor final : public QObject
{
    Q_OBJECT
public:
    void setVPN(MozillaVPN *vpn);

    void runSoon();

private:
    void runInternal();

    void schedule();

    void processData(const QByteArray &data);

private:
    MozillaVPN *m_vpn = nullptr;
};

#endif // RELEASEMONITOR_H
