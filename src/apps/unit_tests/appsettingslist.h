/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// NOTE! Do not include this file directly. Include settingslist.h instead.

SETTING_STRING(foobar,        // getter
               setFoobar,     // setter
               removeFoobar,  // remover
               hasFoobar,     // has
               "foobar",      // key
               "FOO BAR",     // default value
               true,          // user setting
               true           // remove when reset
)

SETTING_STRING(barfoo,        // getter
               setBarfoo,     // setter
               removeBarfoo,  // remover
               hasBarfoo,     // has
               "barfoo",      // key
               "",            // default value
               false,         // user setting
               true           // remove when reset
)
