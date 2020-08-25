#include "keys.h"

#include <QSettings>

constexpr const char* PRIVATE_KEY = "privateKey";

bool Keys::fromSettings(QSettings &settings) {
    if (!settings.contains(PRIVATE_KEY)) {
        return false;
    }
    m_privateKey = settings.value(PRIVATE_KEY).toString();

    return true;
}

void Keys::storeKeys(QSettings &settings, const QString &privateKey) {
    settings.setValue(PRIVATE_KEY, privateKey);
    m_privateKey = privateKey;
}
