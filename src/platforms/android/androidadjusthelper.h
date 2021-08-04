/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDADJUSTHELPER_H
#define ANDROIDADJUSTHELPER_H

#include <QString>

class AndroidAdjustHelper {

  public:
    AndroidAdjustHelper() = default;

    static void trackEvent(const QString& event);

};

#endif // ANDROIDADJUSTHELPER_H