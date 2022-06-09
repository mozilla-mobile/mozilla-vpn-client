/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
const assert = require('assert');
const vpn = require('./helper.js');

describe('Telemetry view', function() {
  beforeEach(async () => {
    await vpn.waitForMainView();
  });

  async function _getToTelemetryPage() {
    await vpn.authenticateInApp();
    await vpn.clickOnElement('postAuthenticationButton');
    await vpn.wait();
    await vpn.waitForElement('telemetryPolicyButton');
    await vpn.waitForElementProperty(
        'telemetryPolicyButton', 'visible', 'true');
    await vpn.waitForElement('declineTelemetryLink');
    await vpn.waitForElementProperty('declineTelemetryLink', 'visible', 'true');
  }

  it('Accept telemetry', async () => {
    assert(await vpn.getSetting('telemetry-policy-shown') === 'false');
    assert(await vpn.getSetting('glean-enabled') === 'true');
    await _getToTelemetryPage();
    await vpn.clickOnElement('telemetryPolicyButton');
    await vpn.wait();
    assert(await vpn.getSetting('telemetry-policy-shown') === 'true');
    assert(await vpn.getSetting('glean-enabled') === 'true');
  });

  it('Deny telemetry', async () => {
    assert(await vpn.getSetting('telemetry-policy-shown') === 'false');
    assert(await vpn.getSetting('glean-enabled') === 'true');
    await _getToTelemetryPage();
    await vpn.clickOnElement('declineTelemetryLink');
    await vpn.wait();
    assert(await vpn.getSetting('telemetry-policy-shown') === 'true');
    assert(await vpn.getSetting('glean-enabled') === 'false');
  });

});
