/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');

describe('Telemetry view', function() {
  beforeEach(async () => {
    await vpn.waitForInitialView();
  });

  async function _getToTelemetryPage() {
    await vpn.authenticateInApp();
    await vpn.clickOnQuery(queries.screenPostAuthentication.BUTTON.visible());
    await vpn.waitForQuery(queries.screenTelemetry.BUTTON.visible());
    await vpn.waitForQuery(queries.screenTelemetry.DECLINE_LINK.visible());
  }

  it('Accept telemetry', async () => {
    assert(await vpn.getSetting('telemetryPolicyShown') === false);
    assert(await vpn.getSetting('gleanEnabled') === true);
    await _getToTelemetryPage();
    await vpn.clickOnQuery(queries.screenTelemetry.BUTTON.visible());

    await vpn.waitForCondition(
        async () => await vpn.getSetting('telemetryPolicyShown') === true);
    assert(await vpn.getSetting('telemetryPolicyShown') === true);
    assert(await vpn.getSetting('gleanEnabled') === true);
  });

  it('Deny telemetry', async () => {
    assert(await vpn.getSetting('telemetryPolicyShown') === false);
    assert(await vpn.getSetting('gleanEnabled') === true);
    await _getToTelemetryPage();
    await vpn.clickOnQuery(queries.screenTelemetry.DECLINE_LINK.visible());

    await vpn.waitForCondition(
        async () => await vpn.getSetting('telemetryPolicyShown') === true);
    assert(await vpn.getSetting('telemetryPolicyShown') === true);
    assert(await vpn.getSetting('gleanEnabled') === false);
  });
});
