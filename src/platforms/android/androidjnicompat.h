/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if QT_VERSION >= 0x060000
#  include <QJniObject>
#  include <QJniEnvironment>

#else
#  include <QAndroidJniObject>
#  include <QAndroidJniEnvironment>
#endif

#if QT_VERSION < 0x060000
typedef QAndroidJniObject QJniObject;
typedef QAndroidJniEnvironment QJniEnvironment;
#endif