/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if defined(_MSVC_TRADITIONAL) && _MSVC_TRADITIONAL
#  error Not supported
#endif

// This list is expected to be in sync with the list on
// http://github.com/mozilla-services/guardian-website/blob/main/nimbus.yaml

#ifdef UNIT_TEST
EXPERIMENTAL_FEATURE(myExperimentalFeature,                // Feature ID
                     "myExperimentalFeature",              // Feature name
                     QStringList({"one", "two", "three"})  // Feature settings
)
#endif
