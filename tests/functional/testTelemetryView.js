/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const util = require('util');
const vpn = require('./helper.js');

describe('Telemetry view', function() {
  this.timeout(60000);

  before(async () => {
    await vpn.connect();
  });

  beforeEach(async () => {
    await vpn.reset();
    await vpn.waitForMainView();
  });

  afterEach(async () => {
    await vpn.dumpFailure;
  });

  after(async () => {
    await vpn.quit();
    vpn.disconnect();
  })

  it('Accept telemetry', async () => {
    assert(await vpn.getSetting('telemetry-policy-shown') === 'false');

    await vpn.clickOnElement('getStarted');

    assert(await vpn.getSetting('glean-enabled') === 'true');
    await vpn.authenticate();

    await vpn.waitForElement('postAuthenticationButton');
    await vpn.clickOnElement('postAuthenticationButton');
    await vpn.wait();

    // Default value
    await vpn.waitForElement('telemetryPolicyButton');
    await vpn.waitForElementProperty(
        'telemetryPolicyButton', 'visible', 'true');
    await vpn.clickOnElement('telemetryPolicyButton');

    await vpn.wait();

    assert(await vpn.getSetting('telemetry-policy-shown') === 'true');
    assert(await vpn.getSetting('glean-enabled') === 'true');
  });

  it('Deny telemetry', async () => {
    assert(await vpn.getSetting('telemetry-policy-shown') === 'false');
    await vpn.clickOnElement('getStarted');

    await vpn.authenticate();

    await vpn.waitForElement('postAuthenticationButton');
    await vpn.clickOnElement('postAuthenticationButton');
    await vpn.wait();

    await vpn.waitForElement('declineTelemetryLink');
    await vpn.waitForElementProperty('declineTelemetryLink', 'visible', 'true');
    await vpn.clickOnElement('declineTelemetryLink');

    await vpn.wait();

    assert(await vpn.getSetting('telemetry-policy-shown') === 'true');
    assert((await vpn.getSetting('glean-enabled')) === 'false');
  });

});
