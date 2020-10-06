/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSUTILS_H
#define MACOSUTILS_H

#include <QString>

class MacOSUtils
{
public:
    static QString computerName();

    static void enableLoginItem(bool startAtBoot);
};

#endif // MACOSUTILS_H
