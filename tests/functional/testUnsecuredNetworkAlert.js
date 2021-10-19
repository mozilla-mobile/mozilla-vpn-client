/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const util = require('util');
const vpn = require('./helper.js');

describe('Unsecured network alert', function() {
  this.timeout(300000);

  before(async () => {
    await vpn.connect();
  });

  beforeEach(() => {});

  afterEach(vpn.dumpFailure);

  after(async () => {
    vpn.disconnect();
  });

  it('Enable unsecured-network-alert feature', async () => {
    await vpn.setSetting('unsecured-network-alert', 'false');
    assert(await vpn.getSetting('unsecured-network-alert') === 'false');

    await vpn.setSetting('unsecured-network-alert', 'true');
    assert(await vpn.getSetting('unsecured-network-alert') === 'true');
  });

  it('Unsecured network alert during the main view', async () => {
    assert(await vpn.getLastUrl() === '');

    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');

    await vpn.forceUnsecuredNetworkAlert();
    await vpn.wait();

    // No notifications during the main view.
    assert(vpn.lastNotification().title === null);
  });

  it('Unsecured network alert during the authentication', async () => {
    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');

    await vpn.clickOnElement('getStarted');

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });

    await vpn.wait();

    await vpn.waitForElement('authenticatingView');
    await vpn.waitForElementProperty('authenticatingView', 'visible', 'true');

    await vpn.forceUnsecuredNetworkAlert();
    await vpn.wait();

    // No notifications during the main view.
    assert(vpn.lastNotification().title === null);

    await vpn.waitForElement('cancelFooterLink');
    await vpn.waitForElementProperty('cancelFooterLink', 'visible', 'true');

    await vpn.clickOnElement('cancelFooterLink');
    await vpn.wait();

    await vpn.waitForElement('getStarted');
    await vpn.waitForElementProperty('getStarted', 'visible', 'true');
  });

  it('authenticate', async () => await vpn.authenticate());

  it('Unsecured network alert in the Post authentication view', async () => {
    await vpn.waitForElement('postAuthenticationButton');

    await vpn.forceUnsecuredNetworkAlert();
    await vpn.wait();

    // Notifications are not OK yet.
    assert(vpn.lastNotification().title === null);

    await vpn.clickOnElement('postAuthenticationButton');
    await vpn.wait();
  });

  it('Unsecured network alert in the Telemetry policy view', async () => {
    await vpn.waitForElement('telemetryPolicyButton');

    await vpn.forceUnsecuredNetworkAlert();
    await vpn.wait();

    // Notifications are not OK yet.
    assert(vpn.lastNotification().title === null);

    await vpn.clickOnElement('telemetryPolicyButton');
    await vpn.wait();
  });

  it('Unsecured network alert in the Controller view', async () => {
    await vpn.waitForElement('controllerTitle');
    await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
    assert(
        await vpn.getElementProperty('controllerTitle', 'text') ===
        'VPN is off');

    await vpn.forceUnsecuredNetworkAlert();
    await vpn.wait();

    // Notifications are OK now.
    assert(vpn.lastNotification().title === 'Unsecured Wi-Fi network detected');
    vpn.resetLastNotification();
    assert(vpn.lastNotification().title === null);

    await vpn.waitForElement('controllerTitle');
    await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
    assert(
        await vpn.getElementProperty('controllerTitle', 'text') ===
        'VPN is off');
  });

  it('Clicking the notification', async () => {
    await vpn.clickOnNotification();

    await vpn.waitForCondition(async () => {
      let connectingMsg =
          await vpn.getElementProperty('controllerTitle', 'text');
      return connectingMsg === 'Connecting…';
    });

    assert(
        await vpn.getElementProperty('controllerSubTitle', 'text') ===
        'Masking connection and location');

    await vpn.forceUnsecuredNetworkAlert();
    await vpn.wait();

    // Notifications are not OK when connecting.
    assert(vpn.lastNotification().title === null);
  });

  it('Unsecured network alert when connected', async () => {
    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('controllerTitle', 'text') ===
          'VPN is on';
    });

    await vpn.waitForCondition(() => {
      return vpn.lastNotification().title === 'VPN Connected';
    });

    vpn.resetLastNotification();

    await vpn.forceUnsecuredNetworkAlert();
    await vpn.wait();

    // Notifications are not OK when connected.
    assert(vpn.lastNotification().title === null);
  });

  it('disconnecting', async () => {
    await vpn.deactivate();

    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('controllerTitle', 'text') ===
          'Disconnecting…';
    });

    await vpn.waitForCondition(() => {
      return vpn.lastNotification().title === 'VPN Disconnected';
    });

    vpn.resetLastNotification();

    await vpn.forceUnsecuredNetworkAlert();
    await vpn.wait();

    // Notifications are not OK when disconnected.
    assert(vpn.lastNotification().title === null);
  });

});
