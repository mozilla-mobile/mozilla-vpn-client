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

  beforeEach(() => {});

  afterEach(vpn.dumpFailure);

  after(async () => {
    vpn.disconnect();
  });

  it('reset the app', async () => await vpn.reset());

  it('wait for the main view', async () => {
    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
    assert(await vpn.getElementProperty('getStarted', 'visible') === 'true');
    assert(await vpn.getElementProperty('learnMoreLink', 'visible') === 'true');
  });

  it('Accept telemetry', async () => {
    await vpn.clickOnElement('getStarted');

    await vpn.waitForElement('telemetryPolicyButton');
    await vpn.waitForElementProperty(
        'telemetryPolicyButton', 'visible', 'true');
    await vpn.clickOnElement('telemetryPolicyButton');

    await vpn.wait();

    assert(await vpn.getSetting('glean-enabled') === 'true');
  });

  it('reset the app', async () => await vpn.reset());

  it('wait for the main view', async () => {
    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
    assert(await vpn.getElementProperty('getStarted', 'visible') === 'true');
    assert(await vpn.getElementProperty('learnMoreLink', 'visible') === 'true');
  });

  it('Deny telemetry', async () => {
    await vpn.clickOnElement('getStarted');

    await vpn.waitForElement('declineTelemetryLink');
    await vpn.waitForElementProperty('declineTelemetryLink', 'visible', 'true');
    await vpn.clickOnElement('declineTelemetryLink');

    await vpn.wait();

    assert((await vpn.getSetting('glean-enabled')) === 'false');
  });

  it('quit the app', async () => await vpn.quit());
});
