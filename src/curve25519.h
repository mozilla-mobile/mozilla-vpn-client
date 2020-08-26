#ifndef CURVE25519_H
#define CURVE25519_H

#include <QByteArray>

#define CURVE25519_KEY_SIZE 32

class Curve25519 final
{
public:
    static QByteArray generatePublicKey(const QByteArray &privateKey);
};

#endif // CURVE25519_H
