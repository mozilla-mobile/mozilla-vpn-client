/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const fs = require('fs');
const util = require('util');
const vpn = require('./helper.js');

describe('Glean event logging', function() {
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

  it('initial view', async () => {
    const glean = await vpn.getLastGleanRequest();
    assert('url' in glean);
    assert(glean.url === '');
    assert('data' in glean);
    assert(glean.data === '');

    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
    await vpn.wait();
  });

  it('Start and abort the authentication (logging)', async () => {
    await vpn.setSetting('glean-enabled', true);
    await vpn.wait();

    await vpn.authenticate();

    await vpn.waitForElement('postAuthenticationButton');
    await vpn.clickOnElement('postAuthenticationButton');
    await vpn.wait();

    await vpn.waitForElement('telemetryPolicyButton');
    await vpn.waitForElementProperty(
        'telemetryPolicyButton', 'visible', 'true');
    await vpn.clickOnElement('telemetryPolicyButton');

    await vpn.wait();

    await vpn.waitForCondition(async () => {
      let glean = await vpn.getLastGleanRequest();
      if (glean.url === '') return false;

      assert(glean.url !== '');
      assert(glean.data !== '');
      return true;
    });

    glean = await vpn.getLastGleanRequest();
    assert(glean.url === '');
    assert(glean.data === '');
  });

  it('reset the app', async () => await vpn.reset());

  it('Start and abort the authentication (no logging)', async () => {
    let glean = await vpn.getLastGleanRequest();
    if (glean.url !== '') {
      assert(glean.url.includes('deletion-request'));
    }

    await vpn.authenticate();

    await vpn.waitForElement('postAuthenticationButton');
    await vpn.clickOnElement('postAuthenticationButton');
    await vpn.wait();

    await vpn.waitForElement('telemetryPolicyButton');
    await vpn.waitForElementProperty(
        'telemetryPolicyButton', 'visible', 'true');
    await vpn.clickOnElement('declineTelemetryLink');

    await vpn.wait();

    glean = await vpn.getLastGleanRequest();
    assert(glean.url.includes('deletion-request'));
  });

  it('quit the app', async () => await vpn.quit());
});
