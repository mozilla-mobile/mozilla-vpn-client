#include "polkithelper.h"

#include <QDebug>

// static
PolkitHelper *PolkitHelper::instance()
{
    static PolkitHelper s_instance;
    return &s_instance;
}

bool PolkitHelper::checkAuthorization(const QString &actionId)
{
    qDebug() << "Check Authorization for" << actionId;
    // TODO
    return true;
}
