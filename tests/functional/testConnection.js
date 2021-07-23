/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const util = require('util');
const vpn = require('./helper.js');

const exec = util.promisify(require('child_process').exec);

describe('Connectivity', function() {
  this.timeout(300000);

  before(async () => {
    await vpn.connect();
  });

  beforeEach(() => {});

  afterEach(vpn.dumpFailure);

  after(async () => {
    vpn.disconnect();
  });


  it('authenticate', async () => await vpn.authenticate());

  it('Post authentication view', async () => {
    await vpn.waitForElement('postAuthenticationButton');
    await vpn.clickOnElement('postAuthenticationButton');
    await vpn.wait();
  });

  it('check the ui', async () => {
    await vpn.waitForElement('controllerTitle');
    await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
    assert(
        await vpn.getElementProperty('controllerTitle', 'text') ===
        'VPN is off');

    await vpn.waitForElementProperty('controllerSubTitle', 'visible', 'true');
    assert(
        await vpn.getElementProperty('controllerSubTitle', 'text') ===
        'Turn on to protect your privacy');

    await vpn.waitForElementProperty('controllerToggle', 'visible', 'true');
  });

  it('connecting', async () => {
    await vpn.setSetting('connection-change-notification', 'true');
    // TODO: investigate why the click doesn't work on github.
    // await vpn.clickOnElement('controllerToggle');
    await vpn.activate();

    await vpn.waitForCondition(async () => {
      let connectingMsg =
          await vpn.getElementProperty('controllerTitle', 'text');
      return connectingMsg === 'Connecting…';
    });

    assert(
        await vpn.getElementProperty('controllerSubTitle', 'text') ===
        'Masking connection and location');
  });

  it('connected', async () => {
    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('controllerTitle', 'text') ==
          'VPN is on';
    });

    assert((await vpn.getElementProperty('controllerSubTitle', 'text'))
               .startsWith('Secure and private'));

    await vpn.waitForCondition(() => {
      return vpn.lastNotification().title === 'VPN Connected';
    });

    assert(vpn.lastNotification().title === 'VPN Connected');
    assert(vpn.lastNotification().message.startsWith('Connected to '));

    vpn.wait();
  });

  it('disconnecting', async () => {
    // TODO: investigate why the click doesn't work on github.
    // await vpn.clickOnElement('controllerToggle');
    await vpn.setSetting('connection-change-notification', 'true');
    await vpn.deactivate();

    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('controllerTitle', 'text') ===
          'Disconnecting…';
    });

    assert(
        await vpn.getElementProperty('controllerSubTitle', 'text') ===
        'Unmasking connection and location');

    vpn.wait();
  });

  it('disconnected', async () => {
    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('controllerTitle', 'text') ===
          'VPN is off';
    });

    assert(
        await vpn.getElementProperty('controllerSubTitle', 'text') ===
        'Turn on to protect your privacy');

    await vpn.waitForCondition(() => {
      return vpn.lastNotification().title === 'VPN Disconnected';
    });

    assert(vpn.lastNotification().title === 'VPN Disconnected');
    assert(vpn.lastNotification().message.startsWith('Disconnected from '));

    vpn.wait();
  });

  it('Logout', async () => {
    await vpn.logout();
    await vpn.wait();
  });

  it('quit the app', async () => await vpn.quit());
});
