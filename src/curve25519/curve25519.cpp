#include "curve25519.h"
#include "curve25519_wg.h"

#include <QRandomGenerator>

#define KEY_LEN 32

extern "C" void curve25519_generate_public(uint8_t pub[CURVE25519_KEY_SIZE],
                                           const uint8_t secret[CURVE25519_KEY_SIZE]);

// static
QByteArray Curve25519::generatePrivateKey()
{
    QByteArray key;

    QRandomGenerator *generator = QRandomGenerator::global();
    Q_ASSERT(generator);

    for (uint8_t i = 0; i < KEY_LEN; ++i) {
        quint32 v = generator->generate();
        key.append(v & 0xFF);
    }

    return key.toBase64();
}

// static
QByteArray Curve25519::generatePublicKey(const QByteArray &privateKey)
{
    QByteArray key = QByteArray::fromBase64(privateKey);

    uint8_t pubKey[CURVE25519_KEY_SIZE];
    uint8_t privKey[CURVE25519_KEY_SIZE];
    curve25519_generate_public(pubKey, privKey);

    QByteArray pk = QByteArray::fromRawData((const char *) pubKey, CURVE25519_KEY_SIZE);
    return pk.toBase64();
}
