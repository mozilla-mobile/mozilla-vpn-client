/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { equal } from 'assert';
import { resetLastNotification, setSetting, getSetting, waitForInitialView, forceCaptivePortalDetection, wait, lastNotification, flipFeatureOff, waitForQueryAndClick, waitForCondition, getLastUrl, waitForQuery, authenticateInApp, getQueryProperty, activate } from './helper.js';
import { screenInitialize, screenTelemetry, screenHome } from './queries.js';

describe('Captive portal', function() {
  this.timeout(300000);

  beforeEach(async function() {
    resetLastNotification();

    //Enable captive-portal-alert feature
    await setSetting('captivePortalAlert', 'false');
    equal(await getSetting('captivePortalAlert'), false);

    await setSetting('captivePortalAlert', 'true');
    equal(await getSetting('captivePortalAlert'), true);
  });

  it('Captive portal during the main view', async () => {
    await waitForInitialView();
    await forceCaptivePortalDetection();
    await wait();

    // No notifications during the main view.
    equal(lastNotification().title, null);
  });

  it('Captive portal during the authentication', async () => {
    if (this.ctx.wasm) {
      // This test cannot run in wasm
      return;
    }

    await waitForInitialView();

    await flipFeatureOff('inAppAuthentication');
    await waitForQueryAndClick(
        screenInitialize.SIGN_UP_BUTTON.visible());

    await waitForCondition(async () => {
      const url = await getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });

    await wait();

    await waitForQuery(
        screenInitialize.AUTHENTICATE_VIEW.visible());

    await forceCaptivePortalDetection();
    await wait();

    // No notifications during the main view.
    equal(lastNotification().title, null);
  });

  it('Captive portal in the Post authentication view', async () => {
    await authenticateInApp();
    // Setup - end

    await forceCaptivePortalDetection();
    await wait();

    // Notifications are not OK yet.
    equal(lastNotification().title, null);
  });

  it('Captive portal in the Telemetry policy view', async () => {
    await authenticateInApp(true, false);
    // Setup - end

    await waitForQuery(screenTelemetry.BUTTON.visible());
    await forceCaptivePortalDetection();
    await wait();

    // Notifications are not OK yet.
    equal(lastNotification().title, null);
  });

  describe('Captive portal post auth', function() {
    this.ctx.authenticationNeeded = true;

    it('Captive portal in the Controller view', async () => {
      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
      equal(
          await getQueryProperty(
              screenHome.CONTROLLER_TITLE, 'text'), 'VPN is off');

      resetLastNotification();

      await forceCaptivePortalDetection();
      await wait();

      // Notifications are not OK yet.
      equal(lastNotification().title, null);
    });

    it('Captive portal when connected', async () => {
      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
      equal(
          await getQueryProperty(
              screenHome.CONTROLLER_TITLE, 'text'), 'VPN is off');

      await activate();
      await waitForCondition(async () => {
        return await getQueryProperty(
                   screenHome.CONTROLLER_TITLE, 'text') === 'VPN is on';
      });
      await waitForCondition(() => {
        return lastNotification().title === 'VPN Connected';
      });
      resetLastNotification();
      // Setup - end

      await forceCaptivePortalDetection();
      await wait();

      // Notifications are OK now.
      await waitForCondition(() => {
        return lastNotification().title === 'Guest Wi-Fi portal blocked';
      });
      equal(lastNotification().title, 'Guest Wi-Fi portal blocked');
    });
    it('Shows the prompt Before activation when a portal is detected before the activation',
       async () => {
         await setSetting('captivePortalAlert', 'true');
         await forceCaptivePortalDetection();

         await activate();
         await waitForQuery(screenHome.CAP_PORTAL_BUTTON);
       });

    it('Shows the Captive Portal Info prompt when a portal is detected and the client is connected',
       async () => {
         await setSetting('captivePortalAlert', 'true');

         await activate();
         await waitForCondition(() => {
           return lastNotification().title === 'VPN Connected';
         });
         await wait();
         await forceCaptivePortalDetection();
         await wait();
         await waitForQuery(screenHome.CAP_PORTAL_BUTTON);
       });
  });
});
