/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const { generalElements, telemetryScreen, initialScreen, homeScreen } = require('./elements.js');
const vpn = require('./helper.js');

describe('Unsecured network alert', function() {
  describe('Basic tests through the intro phases of app', function() {
    it('Enable unsecured-network-alert feature', async () => {
      vpn.resetLastNotification();

      await vpn.setSetting('unsecured-network-alert', 'false');
      assert(await vpn.getSetting('unsecured-network-alert') === 'false');

      await vpn.setSetting('unsecured-network-alert', 'true');
      assert(await vpn.getSetting('unsecured-network-alert') === 'true');
    });

    it('Unsecured network alert during the main view', async () => {
      assert(await vpn.getLastUrl() === '');

      await vpn.waitForElement(initialScreen.GET_HELP_LINK);
      await vpn.waitForElementProperty(initialScreen.GET_HELP_LINK, 'visible', 'true');

      await vpn.forceUnsecuredNetworkAlert();
      await vpn.wait();

      // No notifications during the main view.
      assert(vpn.lastNotification().title === null);
    });

    it('Unsecured network alert during the authentication', async () => {
      if (this.ctx.wasm) {
        // In wasm, the auth is fake and we cannot cancel the auth flow
        return;
      }

      await vpn.waitForElement(initialScreen.GET_HELP_LINK);
      await vpn.waitForElementProperty(initialScreen.GET_HELP_LINK, 'visible', 'true');

      await vpn.clickOnElement(initialScreen.GET_STARTED);

      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });

      await vpn.wait();

      await vpn.waitForElement(initialScreen.AUTHENTICATE_VIEW);
      await vpn.waitForElementProperty(initialScreen.AUTHENTICATE_VIEW, 'visible', 'true');

      await vpn.forceUnsecuredNetworkAlert();
      await vpn.wait();

      // No notifications during the main view.
      assert(vpn.lastNotification().title === null);

      await vpn.waitForElement(homeScreen.CANCEL_FOOTER_LINK);
      await vpn.waitForElementProperty(homeScreen.CANCEL_FOOTER_LINK, 'visible', 'true');

      await vpn.clickOnElement(homeScreen.CANCEL_FOOTER_LINK);
      await vpn.wait();

      await vpn.waitForElement(initialScreen.GET_STARTED);
      await vpn.waitForElementProperty(initialScreen.GET_STARTED, 'visible', 'true');
    });

    it('Unsecured network alert in the Post authentication view', async () => {
      await vpn.authenticateInApp(false, false);
      await vpn.waitForElement(telemetryScreen.POST_AUTHENTICATION_BUTTON);

      await vpn.forceUnsecuredNetworkAlert();
      await vpn.wait();

      // Notifications are not OK yet.
      assert(vpn.lastNotification().title === null);

      await vpn.clickOnElement(telemetryScreen.POST_AUTHENTICATION_BUTTON);
      await vpn.wait();
    });

    it('Unsecured network alert in the Telemetry policy view', async () => {
      await vpn.authenticateInApp(true, false);
      await vpn.waitForElement(telemetryScreen.TELEMETRY_POLICY_BUTTON);

      await vpn.forceUnsecuredNetworkAlert();
      await vpn.wait();

      // Notifications are not OK yet.
      assert(vpn.lastNotification().title === null);

      await vpn.clickOnElement(telemetryScreen.TELEMETRY_POLICY_BUTTON);
      await vpn.wait();
    });

    it('Unsecured network alert in the Controller view', async () => {
      await vpn.authenticateInApp(true, true);
      await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
      await vpn.waitForElementProperty(generalElements.CONTROLLER_TITLE, 'visible', 'true');
      assert(
          await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ===
          'VPN is off');

      await vpn.forceUnsecuredNetworkAlert();
      await vpn.wait();

      // Notifications are OK now.
      assert(
          vpn.lastNotification().title === 'Unsecured Wi-Fi network detected');
      vpn.resetLastNotification();
      assert(vpn.lastNotification().title === null);

      await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
      await vpn.waitForElementProperty(generalElements.CONTROLLER_TITLE, 'visible', 'true');
      assert(
          await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ===
          'VPN is off');
    });
  });

  describe('Tests during activation / deactivation phase', function() {
    this.ctx.authenticationNeeded = true;

    it('Clicking the notification activates the VPN', async () => {
      await vpn.forceUnsecuredNetworkAlert();
      await vpn.wait();
      assert(
          vpn.lastNotification().title === 'Unsecured Wi-Fi network detected');
      vpn.resetLastNotification();
      assert(vpn.lastNotification().title === null);

      await vpn.clickOnNotification();

      await vpn.waitForCondition(async () => {
        let connectingMsg =
            await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text');
        return connectingMsg === 'Connecting…';
      });

      assert(
          await vpn.getElementProperty(generalElements.CONTROLLER_SUBTITLE, 'text') ===
          'Masking connection and location');

      await vpn.forceUnsecuredNetworkAlert();
      await vpn.wait();

      // Notifications are not OK when connecting, but because the connection
      // is so fast recently, often at this point we are already in the connect
      // state.
      assert(
          vpn.lastNotification().title === null ||
          vpn.lastNotification().title === 'VPN Connected');
    });

    it('Unsecured network alert should not show when connected', async () => {
      await vpn.activate();
      await vpn.waitForCondition(async () => {
        return await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ===
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

    it('After disconnecting unsecured network alert should show', async () => {
      await vpn.activate();
      await vpn.waitForCondition(async () => {
        return await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ===
            'VPN is on';
      });
      await vpn.deactivate();

      await vpn.waitForCondition(async () => {
        const msg = await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text');
        return msg === 'Disconnecting…' || msg === 'VPN is off';
      });

      await vpn.waitForCondition(() => {
        return vpn.lastNotification().title === 'VPN Disconnected';
      });

      vpn.resetLastNotification();
      assert(vpn.lastNotification().title === null);

      await vpn.forceUnsecuredNetworkAlert();
      await vpn.wait();

      // Notifications are OK when disconnected.
      assert(
          vpn.lastNotification().title === 'Unsecured Wi-Fi network detected');
    });
  });
});
