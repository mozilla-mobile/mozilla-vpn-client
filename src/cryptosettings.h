#ifndef CRYPTOSETTINGS_H
#define CRYPTOSETTINGS_H

#include <QSettings>

class CryptoSettings
{
public:
    enum Version {
        NoEncryption,
        Encryption,
    };

    static bool readFile(QIODevice &device, QSettings::SettingsMap &map);
    static bool writeFile(QIODevice &device, const QSettings::SettingsMap &map);

private:
    static uint8_t *getKey();

    static Version getSupportedVersion();
    static bool writeVersion(QIODevice &device, Version version);

    static bool readJsonFile(QIODevice &device, QSettings::SettingsMap &map);
    static bool readEncryptedFile(QIODevice &device, QSettings::SettingsMap &map);

    static bool writeJsonFile(QIODevice &device, const QSettings::SettingsMap &map);
    static bool writeEncryptedFile(QIODevice &device, const QSettings::SettingsMap &map);
};

#endif // CRYPTOSETTINGS_H
