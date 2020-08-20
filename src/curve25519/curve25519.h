#ifndef CURVE25519_H
#define CURVE25519_H

#include <QByteArray>

class Curve25519 final
{
public:
    static QByteArray generatePrivateKey();

    static QByteArray generatePublicKey(const QByteArray &privateKey);
};

#endif // CURVE25519_H
