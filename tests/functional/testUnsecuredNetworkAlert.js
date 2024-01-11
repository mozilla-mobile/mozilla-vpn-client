/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import assert, { equal } from 'assert';
import { resetLastNotification, setSetting, getSetting, getLastUrl, waitForQuery, forceUnsecuredNetworkAlert, wait, lastNotification, flipFeatureOff, clickOnQuery, waitForCondition, waitForQueryAndClick, authenticateInApp, getQueryProperty, clickOnNotification, activate, deactivate } from './helper.js';
import { screenInitialize, screenAuthenticating, screenPostAuthentication, screenTelemetry, screenHome } from './queries.js';

describe('Unsecured network alert', function() {
  describe('Basic tests through the intro phases of app', function() {
    it('Enable unsecured-network-alert feature', async () => {
      resetLastNotification();

      await setSetting('unsecuredNetworkAlert', 'false');
      equal(await getSetting('unsecuredNetworkAlert'), false);

      await setSetting('unsecuredNetworkAlert', 'true');
      equal(await getSetting('unsecuredNetworkAlert'), true);
    });

    it('Unsecured network alert during the main view', async () => {
      equal(await getLastUrl(), '');

      await waitForQuery(screenInitialize.GET_HELP_LINK.visible());

      await forceUnsecuredNetworkAlert();
      await wait();

      // No notifications during the main view.
      equal(lastNotification().title, null);
    });

    it('Unsecured network alert during the browser authentication', async () => {
      if (this.ctx.wasm) {
        // In wasm, the auth is fake and we cannot cancel the auth flow
        return;
      }

      await flipFeatureOff('inAppAuthentication');
      await waitForQuery(screenInitialize.GET_HELP_LINK.visible());

      await clickOnQuery(screenInitialize.SIGN_UP_BUTTON.visible());

      await waitForCondition(async () => {
        const url = await getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });

      await wait();

      await waitForQuery(
          screenInitialize.AUTHENTICATE_VIEW.visible());

      await forceUnsecuredNetworkAlert();
      await wait();

      // No notifications during the main view.
      equal(lastNotification().title, null);

      await waitForQueryAndClick(
          screenAuthenticating.CANCEL_FOOTER_LINK.visible());

      await waitForQuery(screenInitialize.SIGN_UP_BUTTON.visible());
    });

    it('Unsecured network alert in the Post authentication view', async () => {
      await authenticateInApp(false, false);
      await waitForQuery(screenPostAuthentication.BUTTON.visible());

      await forceUnsecuredNetworkAlert();
      await wait();

      // Notifications are not OK yet.
      equal(lastNotification().title, null);

      await waitForQueryAndClick(
          screenPostAuthentication.BUTTON.visible());
      await wait();
    });

    it('Unsecured network alert in the Telemetry policy view', async () => {
      await authenticateInApp(true, false);
      await waitForQuery(screenTelemetry.BUTTON.visible());

      await forceUnsecuredNetworkAlert();
      await wait();

      // Notifications are not OK yet.
      equal(lastNotification().title, null);

      await waitForQueryAndClick(screenTelemetry.BUTTON.visible());
      await wait();
    });

    it('Unsecured network alert in the Controller view', async () => {
      await authenticateInApp(true, true);
      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
      equal(
          await getQueryProperty(
              screenHome.CONTROLLER_TITLE.visible(), 'text'),
          'VPN is off');

      await forceUnsecuredNetworkAlert();
      await wait();

      // Notifications are OK now.
      equal(
          lastNotification().title, 'Unsecured Wi-Fi network detected');
      resetLastNotification();
      equal(lastNotification().title, null);

      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
      equal(
          await getQueryProperty(
              screenHome.CONTROLLER_TITLE.visible(), 'text'),
          'VPN is off');
    });
  });

  describe('Tests during activation / deactivation phase', function() {
    this.ctx.authenticationNeeded = true;

    it('Clicking the notification activates the VPN', async () => {
      await forceUnsecuredNetworkAlert();
      await wait();
      equal(
          lastNotification().title, 'Unsecured Wi-Fi network detected');
      resetLastNotification();
      equal(lastNotification().title, null);

      await clickOnNotification();

      await waitForCondition(async () => {
        return await getQueryProperty(
                   screenHome.CONTROLLER_TITLE.visible(), 'text') ===
            'Connecting…';
      });

      equal(
          await getQueryProperty(
              screenHome.CONTROLLER_SUBTITLE.visible(), 'text'),
          'Masking connection and location');

      await forceUnsecuredNetworkAlert();
      await wait();

      // Notifications are not OK when connecting, but because the connection
      // is so fast recently, often at this point we are already in the connect
      // state.
      assert(
          lastNotification().title === null ||
          lastNotification().title === 'VPN Connected');
    });

    it('Unsecured network alert should not show when connected', async () => {
      await activate();
      await waitForCondition(async () => {
        return await getQueryProperty(
                   screenHome.CONTROLLER_TITLE.visible(), 'text') ===
            'VPN is on';
      });

      await waitForCondition(() => {
        return lastNotification().title === 'VPN Connected';
      });

      resetLastNotification();

      await forceUnsecuredNetworkAlert();
      await wait();

      // Notifications are not OK when connected.
      equal(lastNotification().title, null);
    });

    it('After disconnecting unsecured network alert should show', async () => {
      await activate();
      await waitForCondition(async () => {
        return await getQueryProperty(
                   screenHome.CONTROLLER_TITLE.visible(), 'text') ===
            'VPN is on';
      });
      await deactivate();

      await waitForCondition(async () => {
        const msg = await getQueryProperty(
            screenHome.CONTROLLER_TITLE.visible(), 'text');
        return msg === 'Disconnecting…' || msg === 'VPN is off';
      });

      await waitForCondition(() => {
        return lastNotification().title === 'VPN Disconnected';
      });

      resetLastNotification();
      equal(lastNotification().title, null);

      await forceUnsecuredNetworkAlert();
      await wait();

      // Notifications are OK when disconnected.
      equal(
          lastNotification().title, 'Unsecured Wi-Fi network detected');
    });
  });
});
