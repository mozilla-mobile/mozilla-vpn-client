/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// 1. Define the ERROR macro
// 2. include this file
// 3. undefine the ERROR macro

#if defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL
#  error No supported
#endif

// Restore button
ERROR(subscriptionGeneric)
ERROR(noSubscriptionFound)
ERROR(subscriptionExpired)
ERROR(subscriptionInUse)
