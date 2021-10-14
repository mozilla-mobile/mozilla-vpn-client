/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALRESULT_H
#define CAPTIVEPORTALRESULT_H

enum CaptivePortalResult {
  Invalid = -1,
  NoPortal = 0,
  PortalDetected = 1,
  Failure = 2
};

#endif  // CAPTIVEPORTALRESULT_H
