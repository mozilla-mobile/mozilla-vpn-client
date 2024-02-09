/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');
const queries = require('./queries.js');

describe('Unsecured network alert', function() {
  describe('Basic tests through the intro phases of app', function() {
    it('Enable unsecured-network-alert feature', async () => {
      vpn.resetLastNotification();

      await vpn.setSetting('unsecuredNetworkAlert', 'false');
      assert.equal(await vpn.getSetting('unsecuredNetworkAlert'), false);

      await vpn.setSetting('unsecuredNetworkAlert', 'true');
      assert.equal(await vpn.getSetting('unsecuredNetworkAlert'), true);
    });

    it('Unsecured network alert during the main view', async () => {
      assert.equal(await vpn.getLastUrl(), '');

      await vpn.waitForQuery(queries.screenInitialize.GET_HELP_LINK.visible());

      await vpn.forceUnsecuredNetworkAlert();
      await vpn.wait();

      // No notifications during the main view.
      assert.equal(vpn.lastNotification().title, null);
    });

    it('Unsecured network alert during the browser authentication', async () => {
      if (this.ctx.wasm) {
        // In wasm, the auth is fake and we cannot cancel the auth flow
        return;
      }

      await vpn.flipFeatureOff('inAppAuthentication');
      await vpn.waitForQuery(queries.screenInitialize.GET_HELP_LINK.visible());

      await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());

      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });

      await vpn.wait();

      await vpn.waitForQuery(
          queries.screenInitialize.AUTHENTICATE_VIEW.visible());

      await vpn.forceUnsecuredNetworkAlert();
      await vpn.wait();

      // No notifications during the main view.
      assert.equal(vpn.lastNotification().title, null);

      await vpn.waitForQueryAndClick(
          queries.screenAuthenticating.CANCEL_FOOTER_LINK.visible());

      await vpn.waitForQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
    });

    it('Unsecured network alert in the Post authentication view', async function () {
      //Post auth view does not exist in new onboarding
      if (await vpn.isFeatureEnabled('newOnboarding')) {
        this.skip();
      }

      await vpn.authenticateInApp(false);
      await vpn.waitForQuery(queries.screenPostAuthentication.BUTTON.visible());

      await vpn.forceUnsecuredNetworkAlert();
      await vpn.wait();

      // Notifications are not OK yet.
      assert.equal(vpn.lastNotification().title, null);

      await vpn.waitForQueryAndClick(
          queries.screenPostAuthentication.BUTTON.visible());
      await vpn.wait();
    });

    it('Unsecured network alert in the Telemetry policy view', async function () {
      //Telemetry view does not exist in new onboarding
      if (await vpn.isFeatureEnabled('newOnboarding')) {
        this.skip();
      }

      await vpn.authenticateInApp(false);
      await vpn.waitForQuery(queries.screenTelemetry.BUTTON.visible());

      await vpn.forceUnsecuredNetworkAlert();
      await vpn.wait();

      // Notifications are not OK yet.
      assert.equal(vpn.lastNotification().title, null);

      await vpn.waitForQueryAndClick(queries.screenTelemetry.BUTTON.visible());
      await vpn.wait();
    });

    it('Unsecured network alert in the Controller view', async () => {
      await vpn.authenticateInApp();
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenHome.CONTROLLER_TITLE.visible(), 'text'),
          'VPN is off');

      await vpn.forceUnsecuredNetworkAlert();
      await vpn.wait();

      // Notifications are OK now.
      assert.equal(
          vpn.lastNotification().title, 'Unsecured Wi-Fi network detected');
      vpn.resetLastNotification();
      assert.equal(vpn.lastNotification().title, null);

      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenHome.CONTROLLER_TITLE.visible(), 'text'),
          'VPN is off');
    });
  });

  describe('Tests during activation / deactivation phase', function() {
    this.ctx.authenticationNeeded = true;

    it('Clicking the notification activates the VPN', async () => {
      await vpn.forceUnsecuredNetworkAlert();
      await vpn.wait();
      assert.equal(
          vpn.lastNotification().title, 'Unsecured Wi-Fi network detected');
      vpn.resetLastNotification();
      assert.equal(vpn.lastNotification().title, null);

      await vpn.clickOnNotification();

      await vpn.waitForCondition(async () => {
        return await vpn.getQueryProperty(
                   queries.screenHome.CONTROLLER_TITLE.visible(), 'text') ===
            'Connecting…';
      });

      assert.equal(
          await vpn.getQueryProperty(
              queries.screenHome.CONTROLLER_SUBTITLE.visible(), 'text'),
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
        return await vpn.getQueryProperty(
                   queries.screenHome.CONTROLLER_TITLE.visible(), 'text') ===
            'VPN is on';
      });

      await vpn.waitForCondition(() => {
        return vpn.lastNotification().title === 'VPN Connected';
      });

      vpn.resetLastNotification();

      await vpn.forceUnsecuredNetworkAlert();
      await vpn.wait();

      // Notifications are not OK when connected.
      assert.equal(vpn.lastNotification().title, null);
    });

    it('After disconnecting unsecured network alert should show', async () => {
      await vpn.activate();
      await vpn.waitForCondition(async () => {
        return await vpn.getQueryProperty(
                   queries.screenHome.CONTROLLER_TITLE.visible(), 'text') ===
            'VPN is on';
      });
      await vpn.deactivate();

      await vpn.waitForCondition(async () => {
        const msg = await vpn.getQueryProperty(
            queries.screenHome.CONTROLLER_TITLE.visible(), 'text');
        return msg === 'Disconnecting…' || msg === 'VPN is off';
      });

      await vpn.waitForCondition(() => {
        return vpn.lastNotification().title === 'VPN Disconnected';
      });

      vpn.resetLastNotification();
      assert.equal(vpn.lastNotification().title, null);

      await vpn.forceUnsecuredNetworkAlert();
      await vpn.wait();

      // Notifications are OK when disconnected.
      assert.equal(
          vpn.lastNotification().title, 'Unsecured Wi-Fi network detected');
    });
  });
});
