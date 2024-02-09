/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');
const queries = require('./queries.js');

describe('Captive portal', function() {
  this.timeout(300000);

  beforeEach(async function() {
    vpn.resetLastNotification();

    //Enable captive-portal-alert feature
    await vpn.setSetting('captivePortalAlert', 'false');
    assert.equal(await vpn.getSetting('captivePortalAlert'), false);

    await vpn.setSetting('captivePortalAlert', 'true');
    assert.equal(await vpn.getSetting('captivePortalAlert'), true);
  });

  it('Captive portal during the main view', async () => {
    await vpn.waitForInitialView();
    await vpn.forceCaptivePortalDetection();
    await vpn.wait();

    // No notifications during the main view.
    assert.equal(vpn.lastNotification().title, null);
  });

  it('Captive portal during the authentication', async () => {
    if (this.ctx.wasm) {
      // This test cannot run in wasm
      return;
    }

    await vpn.waitForInitialView();

    await vpn.flipFeatureOff('inAppAuthentication');
    await vpn.waitForQueryAndClick(
        queries.screenInitialize.SIGN_UP_BUTTON.visible());

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });

    await vpn.wait();

    await vpn.waitForQuery(
        queries.screenInitialize.AUTHENTICATE_VIEW.visible());

    await vpn.forceCaptivePortalDetection();
    await vpn.wait();

    // No notifications during the main view.
    assert.equal(vpn.lastNotification().title, null);
  });

  it('Captive portal in the Post authentication view', async () => {
    await vpn.authenticateInApp();
    // Setup - end

    await vpn.forceCaptivePortalDetection();
    await vpn.wait();

    // Notifications are not OK yet.
    assert.equal(vpn.lastNotification().title, null);
  });

  it('Captive portal in the Telemetry policy view', async function () {
    //Telemetry policy view does not exist in new onboarding
    if (await vpn.isFeatureEnabled('newOnboarding')) {
      this.skip();
    }

    await vpn.authenticateInApp(false);
    // Setup - end

    await vpn.waitForQuery(queries.screenTelemetry.BUTTON.visible());
    await vpn.forceCaptivePortalDetection();
    await vpn.wait();

    // Notifications are not OK yet.
    assert.equal(vpn.lastNotification().title, null);
  });

  describe('Captive portal post auth', function() {
    this.ctx.authenticationNeeded = true;

    it('Captive portal in the Controller view', async () => {
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenHome.CONTROLLER_TITLE, 'text'), 'VPN is off');

      vpn.resetLastNotification();

      await vpn.forceCaptivePortalDetection();
      await vpn.wait();

      // Notifications are not OK yet.
      assert.equal(vpn.lastNotification().title, null);
    });

    it('Captive portal when connected', async () => {
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenHome.CONTROLLER_TITLE, 'text'), 'VPN is off');

      await vpn.activate();
      await vpn.waitForCondition(async () => {
        return await vpn.getQueryProperty(
                   queries.screenHome.CONTROLLER_TITLE, 'text') === 'VPN is on';
      });
      await vpn.waitForCondition(() => {
        return vpn.lastNotification().title === 'VPN Connected';
      });
      vpn.resetLastNotification();
      // Setup - end

      await vpn.forceCaptivePortalDetection();
      await vpn.wait();

      // Notifications are OK now.
      await vpn.waitForCondition(() => {
        return vpn.lastNotification().title === 'Guest Wi-Fi portal blocked';
      });
      assert.equal(vpn.lastNotification().title, 'Guest Wi-Fi portal blocked');
    });
    it('Shows the prompt Before activation when a portal is detected before the activation',
       async () => {
         await vpn.setSetting('captivePortalAlert', 'true');
         await vpn.forceCaptivePortalDetection();

         await vpn.activate();
         await vpn.waitForQuery(queries.screenHome.CAP_PORTAL_BUTTON);
       });

    it('Shows the Captive Portal Info prompt when a portal is detected and the client is connected',
       async () => {
         await vpn.setSetting('captivePortalAlert', 'true');

         await vpn.activate();
         await vpn.waitForCondition(() => {
           return vpn.lastNotification().title === 'VPN Connected';
         });
         await vpn.wait();
         await vpn.forceCaptivePortalDetection();
         await vpn.wait();
         await vpn.waitForQuery(queries.screenHome.CAP_PORTAL_BUTTON);
       });
  });
});
