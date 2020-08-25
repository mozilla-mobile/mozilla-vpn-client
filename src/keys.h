#ifndef KEYS_H
#define KEYS_H

#include <QString>

class QSettings;

class Keys
{
public:
    bool fromSettings(QSettings &settings);

    void storeKeys(QSettings &settings, const QString &privateKey);

    const QString &privateKey() const { return m_privateKey; }

private:
    QString m_privateKey;
};

#endif // KEYS_H
