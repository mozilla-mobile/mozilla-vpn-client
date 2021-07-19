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

    await vpn.clickOnElement('getStarted');

    await vpn.waitForElement('telemetryPolicyButton');
    await vpn.waitForElementProperty(
        'telemetryPolicyButton', 'visible', 'true');
    await vpn.clickOnElement('telemetryPolicyButton');

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });

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

    await vpn.waitForElement('authenticatingView');
    await vpn.waitForElementProperty('authenticatingView', 'visible', 'true');

    await vpn.waitForElement('cancelFooterLink');
    await vpn.waitForElementProperty('cancelFooterLink', 'visible', 'true');

    await vpn.clickOnElement('cancelFooterLink');

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

    await vpn.waitForElement('getStarted');
    await vpn.waitForElementProperty('getStarted', 'visible', 'true');
  });

  it('reset the app', async () => await vpn.reset());

  it('Start and abort the authentication (no logging)', async () => {
    let glean = await vpn.getLastGleanRequest();
    if (glean.url !== '') {
      assert(glean.url.includes('deletion-request'));
    }

    await vpn.clickOnElement('getStarted');

    await vpn.waitForElement('telemetryPolicyButton');
    await vpn.waitForElementProperty(
        'telemetryPolicyButton', 'visible', 'true');
    await vpn.clickOnElement('declineTelemetryLink');

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });

    await vpn.wait();

    glean = await vpn.getLastGleanRequest();
    if (glean.url !== '') {
      assert(glean.url.includes('deletion-request'));
    }

    await vpn.waitForElement('authenticatingView');
    await vpn.waitForElementProperty('authenticatingView', 'visible', 'true');

    await vpn.waitForElement('cancelFooterLink');
    await vpn.waitForElementProperty('cancelFooterLink', 'visible', 'true');

    await vpn.clickOnElement('cancelFooterLink');

    await vpn.wait();

    glean = await vpn.getLastGleanRequest();
    assert(glean.url === '');
    assert(glean.data === '');

    await vpn.waitForElement('getStarted');
    await vpn.waitForElementProperty('getStarted', 'visible', 'true');
  });

  it('quit the app', async () => await vpn.quit());
});
