#ifndef KEYS_H
#define KEYS_H

#include <QString>

class SettingsHolder;

class Keys
{
public:
    bool fromSettings(SettingsHolder &settings);

    void storeKey(const QString &privateKey);

    void forgetKey();

    const QString &privateKey() const { return m_privateKey; }

private:
    QString m_privateKey;
};

#endif // KEYS_H
