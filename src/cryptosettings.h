#ifndef CRYPTOSETTINGS_H
#define CRYPTOSETTINGS_H

#include <QSettings>

constexpr int CRYPTO_SETTINGS_KEY_SIZE = 32;

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
    static bool getKey(uint8_t[CRYPTO_SETTINGS_KEY_SIZE]);

    static Version getSupportedVersion();
    static bool writeVersion(QIODevice &device, Version version);

    static bool readJsonFile(QIODevice &device, QSettings::SettingsMap &map);
    static bool readEncryptedFile(QIODevice &device, QSettings::SettingsMap &map);

    static bool writeJsonFile(QIODevice &device, const QSettings::SettingsMap &map);
    static bool writeEncryptedFile(QIODevice &device, const QSettings::SettingsMap &map);
};

#endif // CRYPTOSETTINGS_H
