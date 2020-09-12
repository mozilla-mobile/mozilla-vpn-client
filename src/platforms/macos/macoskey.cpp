#include <QByteArray>

#define WG_KEY_LEN (32)
#define WG_KEY_LEN_BASE64 (45)
#define WG_KEY_LEN_HEX (65)

#define EXPORT __attribute__((visibility("default")))

extern "C" {
EXPORT void key_to_base64(char base64[WG_KEY_LEN_BASE64], const uint8_t key[WG_KEY_LEN]);
EXPORT bool key_from_base64(uint8_t key[WG_KEY_LEN], const char *base64);

EXPORT void key_to_hex(char hex[WG_KEY_LEN_HEX], const uint8_t key[WG_KEY_LEN]);
EXPORT bool key_from_hex(uint8_t key[WG_KEY_LEN], const char *hex);
}

EXPORT void key_to_base64(char base64[WG_KEY_LEN_BASE64], const uint8_t key[WG_KEY_LEN])
{
    QByteArray k = QByteArray::fromRawData((const char *) key, WG_KEY_LEN);
    QByteArray k64 = k.toBase64();
    Q_ASSERT(k64.length() == WG_KEY_LEN_BASE64);

    for (int i = 0; i < WG_KEY_LEN_BASE64; ++i) {
        base64[i] = k64.at(i);
    }
}

EXPORT bool key_from_base64(uint8_t key[WG_KEY_LEN], const char *base64)
{
    Q_UNUSED(key);
    Q_UNUSED(base64)

    if (strlen(base64) != WG_KEY_LEN_BASE64 - 1 || base64[WG_KEY_LEN_BASE64 - 2] != '=') {
        return false;
    }

    QByteArray k64 = QByteArray::fromRawData(base64, WG_KEY_LEN_BASE64);
    QByteArray k = QByteArray::fromBase64(k64);

    for (int i = 0; i < WG_KEY_LEN; ++i) {
        key[i] = k.at(i);
    }

    return true;
}

EXPORT void key_to_hex(char hex[WG_KEY_LEN_HEX], const uint8_t key[WG_KEY_LEN])
{
    QByteArray k = QByteArray::fromRawData((const char *) key, WG_KEY_LEN);
    QByteArray h = k.toHex();
    Q_ASSERT(h.length() == WG_KEY_LEN_HEX);

    for (int i = 0; i < WG_KEY_LEN_HEX; ++i) {
        hex[i] = h.at(i);
    }
}

EXPORT bool key_from_hex(uint8_t key[WG_KEY_LEN], const char *hex)
{
    if (strlen(hex) != WG_KEY_LEN_HEX - 1) {
        return false;
    }

    QByteArray h = QByteArray::fromRawData(hex, WG_KEY_LEN_HEX);
    QByteArray k = QByteArray::fromHex(h);

    for (int i = 0; i < WG_KEY_LEN; ++i) {
        key[i] = k.at(i);
    }

    return true;
}
