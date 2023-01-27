/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// 1. Define the SETTING macro
// 2. include this file
// 3. undefine the SETTING macro

#if defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL
#  error No supported
#endif

#define SETTING_BOOL(getter, ...) SETTING(bool, toBool, getter, __VA_ARGS__)

#define SETTING_BYTEARRAY(getter, ...) \
  SETTING(QByteArray, toByteArray, getter, __VA_ARGS__)

#define SETTING_DATETIME(getter, ...) \
  SETTING(QDateTime, toDateTime, getter, __VA_ARGS__)

#define SETTING_INT(getter, ...) SETTING(int, toInt, getter, __VA_ARGS__)

#define SETTING_INT64(getter, ...) \
  SETTING(qint64, toLongLong, getter, __VA_ARGS__)

#define SETTING_STRING(getter, ...) \
  SETTING(QString, toString, getter, __VA_ARGS__)

#define SETTING_STRINGLIST(getter, ...) \
  SETTING(QStringList, toStringList, getter, __VA_ARGS__)

SETTING_STRING(userEmail,        // getter
               setUserEmail,     // setter
               removeUserEmail,  // remover
               hasUserEmail,     // has
               "user/email",     // key
               "",               // default value
               false,            // user setting
               true,             // remove when reset
               true              // sensitive (do not log)
)

// This setting is only intended for running the functional tests.
SETTING_BOOL(localhostRequestsOnly,        // Feature ID
             setLocalhostRequestsOnly,     // setter
             removeLocalhostRequestsOnly,  // remover
             hasLocalhostRequestsOnly,     // has
             "localhostRequestOnly",       // key
             false,                        // default value
             false,                        // user setting
             true                          // remove when reset
)

// The app must implement its settings list file.
#include "appsettingslist.h"
