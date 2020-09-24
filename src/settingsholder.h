#ifndef SETTINGSHOLDER_H
#define SETTINGSHOLDER_H

#include <QObject>

class MozillaVPN;
class QSettings;

class SettingsHolder : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool ipv6 READ ipv6 WRITE setIpv6 NOTIFY ipv6Changed)

public:
    void setVPN(MozillaVPN *vpn, QSettings *settings)
    {
        m_vpn = vpn;
        m_settings = settings;
    }

    bool ipv6() const;
    void setIpv6(bool ipv6);

signals:
    void ipv6Changed();

private:
    MozillaVPN *m_vpn = nullptr;
    QSettings *m_settings = nullptr;
};

#endif // SETTINGSHOLDER_H
