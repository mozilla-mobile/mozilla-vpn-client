/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const vpn = require('./helper.js');

describe('Glean event logging', function() {
  this.timeout(5000);

  beforeEach(async () => {
    await vpn.setGleanAutomationHeader();
  });

  /*
  (UPDATE THIS BASED ON NEW TELEMETRY PAGE POSITION)

  I would like to do the following suite of tests:
  * set debugMode to true/false, ensure glean is initialized appropriately
  * set stagingMode to true/false, ensure glean is initialized appropriately
  * change VPNSettings.glean-enabled and ensure glean upload enabled has been
  changed
  * make an event and ensure the right data is in the ping
  * ensure that a call to sendPings, sends pings
  * ensure Glean.shutdown is called onAboutToQuit

  Update the testTelemetryView tests to
  * ensure that "Accept telemetry" and "Reject telemetry" result in the correct
  glean state change

  Possible c++ unit tests:
  * ensure that timer to sendPings is setup

  We do not need to test that glean does the right thing e.g. testing for a
  deletion request. As long as we check that we've told glean to
  setUploadEnabled to false, then its handling of that is a glean implementation
  detail and glean tests their own code.
  */
});
