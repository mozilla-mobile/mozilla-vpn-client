/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINUTILS_H
#define WINUTILS_H

#include <QString>

namespace WinUtils {
QString win32strerror(unsigned long code);
quint64 getVpnLuid() ;
}

#endif  // WINUTILS_H
