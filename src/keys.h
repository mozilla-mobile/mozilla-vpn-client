#ifndef KEYS_H
#define KEYS_H

#include <QString>

class QSettings;

class Keys
{
public:
    bool fromSettings(QSettings &settings);

    void storeKey(QSettings &settings, const QString &privateKey);

    void forgetKey();

    const QString &privateKey() const { return m_privateKey; }

private:
    QString m_privateKey;
};

#endif // KEYS_H
