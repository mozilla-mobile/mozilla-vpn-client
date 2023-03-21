/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// NOTE! Do not include this file directly. Include settingslist.h instead.

#ifdef MZ_WASM
#  define STAGING_SERVER_DEFAULT_VALUE true
#else
#  define STAGING_SERVER_DEFAULT_VALUE false
#endif
SETTING_BOOL(stagingServer,                 // getter
             setStagingServer,              // setter
             removeStagingServer,           // remover
             hasStagingServer,              // has
             "stagingServer",               // key
             STAGING_SERVER_DEFAULT_VALUE,  // default value
             false,                         // user setting
             false,                         // remove when reset
             false                          // sensitive (do not log)
)

SETTING_STRING(
    stagingServerAddress,        // getter
    setStagingServerAddress,     // setter
    removeStagingServerAddress,  // remover
    hasStagingServerAddress,     // has
    "stagingServerAddress",      // key
    Constants::envOrDefault("RELAY_API_BASE_URL",
                            AppConstants::API_STAGING_URL),  // default value
    false,                                                   // user setting
    false,  // remove when reset
    false   // sensitive (do not log)
)
