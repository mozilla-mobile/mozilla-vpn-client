/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cryptosettings.h"

uint8_t *CryptoSettings::getKey()
{
    Q_ASSERT(false);
    return nullptr;
}

// static
CryptoSettings::Version CryptoSettings::getSupportedVersion()
{
    return CryptoSettings::NoEncryption;
}
