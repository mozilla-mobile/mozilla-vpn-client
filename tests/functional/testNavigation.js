/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');

const assert = require('assert');
const vpn = require('./helper.js');

describe('Settings', function() {
  this.timeout(60000);

  /* Nav bar is...
    ... not visible over initial screen
    ... not visible over mobile onboarding
    ... not visible during in-app auth
    ... not visible during browser auth
    ... not visible over telemetry screen
    ... not visible over post-auth screen
    ... not visible over subscription needed
    ... not visible over reauth screen

    - visible over home screens
    - visible over settings screens
    - visible over messaging screens
    - visible over get help screens

    - clicking settings icon opens settings screen
    - clicking home icon opens home screen
    - clicking messages icon opens messages screen

    - double clicking settings icon unwinds settings stack
    - double clicking home icon unwinds home stack
    - double clicking messaging icon unwinds messaging stack


    - selected navbar correctly reflects screen opens from systray

    what else?
  */

  /* Menu bar
    ... has no back button on initial views
    ... is not visible if menu.title === ""
    ... accessibleName aligns with image
  */

  /*

  */
});