#ifndef POLKITHELPER_H
#define POLKITHELPER_H

#include <QString>

class PolkitHelper
{
public:
    static PolkitHelper *instance();

    bool checkAuthorization(const QString &actionId);

private:
    PolkitHelper() = default;
    ~PolkitHelper() = default;

    Q_DISABLE_COPY(PolkitHelper)
};

#endif // POLKITHELPER_H
