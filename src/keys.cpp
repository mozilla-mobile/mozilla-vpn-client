#include "keys.h"
#include "settingsholder.h"

bool Keys::fromSettings(SettingsHolder &settingsHolder)
{
    if (!settingsHolder.hasPrivateKey()) {
        return false;
    }

    m_privateKey = settingsHolder.privateKey();
    return true;
}

void Keys::storeKey(const QString &privateKey)
{
    m_privateKey = privateKey;
}

void Keys::forgetKey()
{
    m_privateKey.clear();
}
