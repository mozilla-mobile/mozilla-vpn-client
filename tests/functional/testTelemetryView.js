/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
const assert = require('assert');
const { telemetryView } = require('./elements.js');
const vpn = require('./helper.js');

describe('Telemetry view', function() {
  beforeEach(async () => {
    await vpn.waitForMainView();
  });

  async function _getToTelemetryPage() {
    await vpn.authenticateInApp();
    await vpn.clickOnElement(telemetryView.POST_AUTHENTICATION_BUTTON);
    await vpn.wait();
    await vpn.waitForElement(telemetryView.TELEMETRY_POLICY_BUTTON);
    await vpn.waitForElementProperty(telemetryView.TELEMETRY_POLICY_BUTTON, 'visible', 'true');
    await vpn.waitForElement(telemetryView.DECLINE_TELEMETRY);
    await vpn.waitForElementProperty(telemetryView.DECLINE_TELEMETRY, 'visible', 'true');
  }

  it('Accept telemetry', async () => {
    assert(await vpn.getSetting('telemetry-policy-shown') === 'false');
    assert(await vpn.getSetting('glean-enabled') === 'true');
    await _getToTelemetryPage();
    await vpn.clickOnElement(telemetryView.TELEMETRY_POLICY_BUTTON);
    await vpn.wait();
    assert(await vpn.getSetting('telemetry-policy-shown') === 'true');
    assert(await vpn.getSetting('glean-enabled') === 'true');
  });

  it('Deny telemetry', async () => {
    assert(await vpn.getSetting('telemetry-policy-shown') === 'false');
    assert(await vpn.getSetting('glean-enabled') === 'true');
    await _getToTelemetryPage();
    await vpn.clickOnElement(telemetryView.DECLINE_TELEMETRY);
    await vpn.wait();
    assert(await vpn.getSetting('telemetry-policy-shown') === 'true');
    assert(await vpn.getSetting('glean-enabled') === 'false');
  });

});
