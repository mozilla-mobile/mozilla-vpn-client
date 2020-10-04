#include "cryptosettings.h"
#include "hacl-star/Hacl_Chacha20Poly1305_32.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QRandomGenerator>

constexpr const char *NONCE_KEY = "nonce_encrypt_key";

constexpr int NONCE_SIZE = 12;
constexpr int MAC_SIZE = 16;

// static
bool CryptoSettings::readFile(QIODevice &device, QSettings::SettingsMap &map)
{
    qDebug() << "Read the settings file";

    QByteArray version = device.read(1);
    if (version.length() != 1) {
        qDebug() << "Failed to read the version";
        return false;
    }

    switch ((CryptoSettings::Version) version.at(0)) {
    case NoEncryption:
        return readJsonFile(device, map);
    case Encryption:
        return readEncryptedFile(device, map);
    default:
        qDebug() << "Unsupported version";
        return false;
    }
}

// static
bool CryptoSettings::readJsonFile(QIODevice &device, QSettings::SettingsMap &map)
{
    QByteArray content = device.readAll();

    QJsonDocument json = QJsonDocument::fromJson(content);
    if (!json.isObject()) {
        qDebug() << "Invalid content read from the JSON file";
        return false;
    }

    QJsonObject obj = json.object();
    for (QJsonObject::const_iterator i = obj.constBegin(); i != obj.constEnd(); ++i) {
        map.insert(i.key(), i.value().toVariant());
    }

    return true;
}

// static
bool CryptoSettings::readEncryptedFile(QIODevice &device, QSettings::SettingsMap &map)
{
    QByteArray nonce = device.read(NONCE_SIZE);
    if (nonce.length() != NONCE_SIZE) {
        qDebug() << "Failed to read the nonce";
        return false;
    }

    QByteArray mac = device.read(MAC_SIZE);
    if (mac.length() != MAC_SIZE) {
        qDebug() << "Failed to read the MAC";
        return false;
    }

    QByteArray cipher = device.readAll();
    if (cipher.length() == 0) {
        qDebug() << "Failed to read the cipher";
        return false;
    }

    uint8_t key[CRYPTO_SETTINGS_KEY_SIZE];
    if (!getKey(key)) {
        qDebug() << "Something went wrong reading the key";
        return false;
    }

    QByteArray version(1, Encryption);
    QByteArray content(cipher.length(), 0x00);
    uint32_t result = Hacl_Chacha20Poly1305_32_aead_decrypt(key,
                                                            (uint8_t *) nonce.data(),
                                                            version.length(),
                                                            (uint8_t *) version.data(),
                                                            cipher.length(),
                                                            (uint8_t *) content.data(),
                                                            (uint8_t *) cipher.data(),
                                                            (uint8_t *) mac.data());
    qDebug() << "Result:" << result;
    if (result != 0) {
        return false;
    }

    QJsonDocument json = QJsonDocument::fromJson(content);
    if (!json.isObject()) {
        qDebug() << "Invalid content read from the JSON file";
        return false;
    }

    QJsonObject obj = json.object();
    for (QJsonObject::const_iterator i = obj.constBegin(); i != obj.constEnd(); ++i) {
        if (i.key() != NONCE_KEY) {
            map.insert(i.key(), i.value().toVariant());
        }
    }

    // Let's store the nonce into the map.
    map.insert(NONCE_KEY, QVariant(nonce));
    return true;
}

// static
bool CryptoSettings::writeFile(QIODevice &device, const QSettings::SettingsMap &map)
{
    qDebug() << "Writing the settings file";

    Version version = getSupportedVersion();
    if (!writeVersion(device, version)) {
        qDebug() << "Failed to write the version";
        return false;
    }

    switch (version) {
    case NoEncryption:
        return writeJsonFile(device, map);
    case Encryption:
        return writeEncryptedFile(device, map);
    default:
        qDebug() << "Unsupported version.";
        return false;
    }
}

// static
bool CryptoSettings::writeVersion(QIODevice &device, CryptoSettings::Version version)
{
    QByteArray v(1, version);
    return device.write(v) == v.length();
}

// static
bool CryptoSettings::writeJsonFile(QIODevice &device, const QSettings::SettingsMap &map)
{
    qDebug() << "Write plaintext JSON file";

    QJsonObject obj;
    for (QSettings::SettingsMap::ConstIterator i = map.begin(); i != map.end(); ++i) {
        obj.insert(i.key(), QJsonValue::fromVariant(i.value()));
    }

    QJsonDocument json;
    json.setObject(obj);
    QByteArray content = json.toJson(QJsonDocument::Compact);

    if (device.write(content) != content.length()) {
        qDebug() << "Failed to write the content";
        return false;
    }

    return true;
}

// static
bool CryptoSettings::writeEncryptedFile(QIODevice &device, const QSettings::SettingsMap &map)
{
    qDebug() << "Write encrypted file";

    QByteArray nonce;
    QJsonObject obj;
    for (QSettings::SettingsMap::ConstIterator i = map.begin(); i != map.end(); ++i) {
        if (i.key() == NONCE_KEY) {
            nonce = i.value().toByteArray();
            continue;
        }

        obj.insert(i.key(), QJsonValue::fromVariant(i.value()));
    }

    QJsonDocument json;
    json.setObject(obj);
    QByteArray content = json.toJson(QJsonDocument::Compact);

    uint8_t key[CRYPTO_SETTINGS_KEY_SIZE];
    if (!getKey(key)) {
        qDebug() << "Invalid key";
        return false;
    }

    if (nonce.length() != NONCE_SIZE) {
        qDebug() << "Nonce not found. Let's generate it";

        QRandomGenerator* rg = QRandomGenerator::global();
        nonce = QByteArray(NONCE_SIZE, 0x00);
        for (QByteArray::Iterator i = nonce.begin(); i != nonce.end(); ++i) {
            *i = rg->generate() % 0xFF;
        }
    }

    QByteArray version(1, Encryption);
    QByteArray cipher(content.length(), 0x00);
    QByteArray mac(MAC_SIZE, 0x00);

    Hacl_Chacha20Poly1305_32_aead_encrypt(key,
                                          (uint8_t *) nonce.data(),
                                          version.length(),
                                          (uint8_t *) version.data(),
                                          content.length(),
                                          (uint8_t *) content.data(),
                                          (uint8_t *) cipher.data(),
                                          (uint8_t *) mac.data());

    if (device.write(nonce) != nonce.length()) {
        qDebug() << "Failed to write the nonce";
        return false;
    }

    if (device.write(mac) != mac.length()) {
        qDebug() << "Failed to write the MAC";
        return false;
    }

    if (device.write(cipher) != cipher.length()) {
        qDebug() << "Failed to write the cipher";
        return false;
    }

    return true;
}
