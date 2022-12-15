/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// This file is used to include the setting list for the VPN module
// NOTE! do not include this file directly. Use settingslist.h instead.

SETTING_BOOL(captivePortalAlert,        // getter
             setCaptivePortalAlert,     // setter
             removeCaptivePortalAlert,  // remover
             hasCaptivePortalAlert,     // has
             "captivePortalAlert",      // key
             Feature::get(Feature::Feature_captivePortal)
                 ->isSupported(),  // default value
             true,                 // user setting
             false                 // remove when reset
)

SETTING_STRINGLIST(captivePortalIpv4Addresses,        // getter
                   setCaptivePortalIpv4Addresses,     // setter
                   removeCaptivePortalIpv4Addresses,  // remover
                   hasCaptivePortalIpv4Addresses,     // has
                   "captivePortal/ipv4Addresses",     // key
                   QStringList(),                     // default value
                   false,                             // user setting
                   false                              // remove when reset
)

SETTING_STRINGLIST(captivePortalIpv6Addresses,         // getter
                   setCaptivePortalIpv6Addresses,      // setter
                   removerCaptivePortalIpv6Addresses,  // remover
                   hasCaptivePortalIpv6Addresses,      // has
                   "captivePortal/ipv6Addresses",      // key
                   QStringList(),                      // default value
                   false,                              // user setting
                   false                               // remove when reset
)

SETTING_BOOL(connectionChangeNotification,        // getter
             setConnectionChangeNotification,     // setter
             removeConnectionChangeNotification,  // remover
             hasConnectionChangeNotification,     // has
             "connectionChangeNotification",      // key
             true,                                // default value
             true,                                // user setting
             false                                // remove when reset
)

SETTING_INT(dnsProvider,                           // getter
            setDNSProvider,                        // setter
            removeDNSProvider,                     // remover
            hasDNSProvider,                        // has
            "dnsProvider",                         // key
            SettingsHolder::DnsProvider::Gateway,  // default value
            true,                                  // user setting
            false                                  // remove when reset
)

SETTING_INT64(keyRegenerationTimeSec,        // getter
              setKeyRegenerationTimeSec,     // setter
              removeKeyRegenerationTimeSec,  // remover
              hasKeyRegenerationTimeSec,     // has
              "keyRegenerationTimeSec",      // key
              0,                             // default value
              false,                         // user setting
              true                           // remove when reset
)
