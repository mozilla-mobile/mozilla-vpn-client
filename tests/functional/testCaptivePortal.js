/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
const assert = require('assert');
const vpn = require('./helper.js');

describe('Captive portal', function() {
  this.timeout(300000);

  beforeEach(async function() {
    vpn.resetLastNotification();

    //Enable captive-portal-alert feature
    await vpn.setSetting('captive-portal-alert', 'false');
    assert(await vpn.getSetting('captive-portal-alert') === 'false');

    await vpn.setSetting('captive-portal-alert', 'true');
    assert(await vpn.getSetting('captive-portal-alert') === 'true');
  });

  it('Captive portal during the main view', async () => {
    await vpn.waitForMainView();
    await vpn.forceCaptivePortalDetection();
    await vpn.wait();

    // No notifications during the main view.
    assert(vpn.lastNotification().title === null);
  });

  it('Captive portal during the authentication', async () => {
    if (this.ctx.wasm) {
      // This test cannot run in wasm
      return;
    }

    await vpn.waitForMainView();

    await vpn.flipFeatureOff('inAppAuthentication');
    await vpn.clickOnElement('getStarted');

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });

    await vpn.wait();

    await vpn.waitForElement('authenticatingView');
    await vpn.waitForElementProperty('authenticatingView', 'visible', 'true');

    await vpn.forceCaptivePortalDetection();
    await vpn.wait();

    // No notifications during the main view.
    assert(vpn.lastNotification().title === null);
  });

  it('Captive portal in the Post authentication view', async () => {
    await vpn.authenticateInApp();
    // Setup - end

    await vpn.forceCaptivePortalDetection();
    await vpn.wait();

    // Notifications are not OK yet.
    assert(vpn.lastNotification().title === null);
  });

  it('Captive portal in the Telemetry policy view', async () => {
    await vpn.authenticateInApp(true, false);
    // Setup - end

    await vpn.waitForElement('telemetryPolicyButton');
    await vpn.forceCaptivePortalDetection();
    await vpn.wait();

    // Notifications are not OK yet.
    assert(vpn.lastNotification().title === null);
  });

  describe('Captive portal post auth', function() {
    this.ctx.authenticationNeeded = true;

    it('Captive portal in the Controller view', async () => {
      await vpn.waitForElement('controllerTitle');
      await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
      assert(
          await vpn.getElementProperty('controllerTitle', 'text') ===
          'VPN is off');

      vpn.resetLastNotification();

      await vpn.forceCaptivePortalDetection();
      await vpn.wait();

      // Notifications are not OK yet.
      assert(vpn.lastNotification().title === null);
    });

    it('Captive portal when connected', async () => {
      await vpn.waitForElement('controllerTitle');
      await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
      assert(
          await vpn.getElementProperty('controllerTitle', 'text') ===
          'VPN is off');
      await vpn.activate();
      await vpn.waitForCondition(async () => {
        return await vpn.getElementProperty('controllerTitle', 'text') ===
            'VPN is on';
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
      assert(vpn.lastNotification().title === 'Guest Wi-Fi portal blocked');
    });

    it('Clicking the alert and wait for recovering', async () => {
      await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
      assert(
          await vpn.getElementProperty('controllerTitle', 'text') ===
          'VPN is off');
      await vpn.activate();
      await vpn.waitForCondition(() => {
        return vpn.lastNotification().title === 'VPN Connected';
      });
      vpn.resetLastNotification();
      // Setup - end

      await vpn.forceCaptivePortalDetection();
      await vpn.wait();

      await vpn.waitForCondition(() => {
        return vpn.hasElement('captivePortalAlertActionButton');
      });
      // Clicking on the alert should disable the vpn
      await vpn.clickOnElement('captivePortalAlertActionButton');

      await vpn.waitForCondition(async () => {
        let connectingMsg =
            await vpn.getElementProperty('controllerTitle', 'text');
        return connectingMsg === 'Disconnecting…' ||
            connectingMsg === 'VPN is off';
      });
      await vpn.waitForCondition(() => {
        return vpn.lastNotification().title === 'VPN Disconnected';
      });

      // 'Wait for recovering'

      await vpn.waitForCondition(() => {
        return vpn.lastNotification().title === 'Guest Wi-Fi portal detected';
      });

      await vpn.clickOnNotification();

      await vpn.waitForCondition(async () => {
        let connectingMsg =
            await vpn.getElementProperty('controllerTitle', 'text');
        return connectingMsg === 'Connecting…';
      });

      await vpn.waitForCondition(() => {
        return vpn.lastNotification().title === 'VPN Connected';
      });
    });

    it('Shows the prompt Before activation when a portal is detected before the activation',
       async () => {
         await vpn.setSetting('captive-portal-alert', 'true');
         await vpn.forceCaptivePortalDetection();

         await vpn.activate();
         await vpn.waitForCondition(() => {
           return vpn.hasElement('captivePortalAlertActionButton');
         });
         assert(true);
       });

    it('Shows the Captive Portal Info prompt when a portal is detected and the client is connected',
       async () => {
         await vpn.setSetting('captive-portal-alert', 'true');

         await vpn.activate();
         await vpn.waitForCondition(() => {
           return vpn.lastNotification().title === 'VPN Connected';
         });
         await vpn.wait();
         await vpn.forceCaptivePortalDetection();
         await vpn.wait();
         await vpn.waitForCondition(() => {
           return vpn.hasElement('captivePortalAlertActionButton');
         });
         assert(true);
       });
  });
});
