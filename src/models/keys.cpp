/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "keys.h"
#include "leakdetector.h"
#include "settingsholder.h"

Keys::Keys()
{
    MVPN_COUNT_CTOR(Keys);
}

Keys::~Keys()
{
    MVPN_COUNT_DTOR(Keys);
}

bool Keys::fromSettings()
{
    SettingsHolder *settingsHolder = SettingsHolder::instance();
    Q_ASSERT(settingsHolder);

    if (!settingsHolder->hasPrivateKey()) {
        return false;
    }

    m_privateKey = settingsHolder->privateKey();
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
