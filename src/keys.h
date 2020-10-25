/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef KEYS_H
#define KEYS_H

#include <QString>

class SettingsHolder;

class Keys final
{
public:
    [[nodiscard]] bool fromSettings(SettingsHolder &settings);

    void storeKey(const QString &privateKey);

    void forgetKey();

    const QString &privateKey() const { return m_privateKey; }

private:
    QString m_privateKey;
};

#endif // KEYS_H
