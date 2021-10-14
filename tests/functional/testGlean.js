/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');

describe('Glean event logging', function() {
  this.timeout(5000);

  before(async () => {
    await vpn.connect();
  });

  beforeEach(async () => {
    await vpn.reset();
    await vpn.setGleanAutomationHeader();
  });

  afterEach(async () => {
    await vpn.dumpFailure;
  });

  after(async () => {
    await vpn.quit();
    vpn.disconnect();
  })

  /*
  (UPDATE THIS BASED ON NEW TELEMETRY PAGE POSITION)

  I would like to do the following suite of tests:
  * set telemetryPolicyShown to false, call initializeGlean, ensure glean is not
  initialized
  * set telemetryPolicyShown to true, call initializeGlean, ensure glean is
  initialized
  * set debugMode to true/false, ensure glean is initialized appropriately
  * set stagingMode to true/false, ensure glean is initialized appropriately
  * change VPNSettings.glean-enabled and ensure glean upload enabled has been
  changed
  * make an event and ensure the right data is in the ping
  * ensure that a call to sendPings, sends pings
  * ensure that startup in a logged in state causes a glean initialization
  * ensure Glean.shutdown is called onAboutToQuit

  Do we want to test for each glean event that should generate a ping?

  Update the testTelemetryView tests to
  * ensure that "Accept telemetry" and "Reject telemetry" result in the correct
  glean initalization

  Possible c++ unit tests:
  * ensure that timer to sendPings is setup

  We do not need to test that glean does the right thing e.g. testing for a
  deletion request. As long as we check that we've told glean to
  setUploadEnabled to false, then its handling of that is a glean implementation
  detail and glean tests their own code.
  */
});
