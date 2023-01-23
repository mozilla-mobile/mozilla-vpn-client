/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');
const { SubscriptionDetails } = require('./servers/guardian_endpoints.js')
const { startAndConnect } = require('./setupVpn.js')

describe('Addons', function() {
  this.ctx.authenticationNeeded = true;

  it('Empty addon index', async () => {
    await vpn.resetAddons('01_empty_manifest');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getVPNProperty('VPNAddonManager', 'count'), 10) ===
          0;
    });
  });

  it('Broken addon index', async () => {
    await vpn.resetAddons('03_single_addon');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getVPNProperty('VPNAddonManager', 'count'), 10) ===
          1;
    });

    await vpn.fetchAddons('02_broken_manifest');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getVPNProperty('VPNAddonManager', 'count'), 10) ===
          1;
    });
  });

  it('Addons are loaded', async () => {
    await vpn.resetAddons('03_single_addon');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getVPNProperty('VPNAddonManager', 'count'), 10) ===
          1;
    });

    await vpn.fetchAddons('01_empty_manifest');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getVPNProperty('VPNAddonManager', 'count'), 10) ===
          0;
    });

    await vpn.fetchAddons('03_single_addon');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getVPNProperty('VPNAddonManager', 'count'), 10) ===
          1;
    });
  });

  it('Settings rollback - location', async () => {
    // Loading the custom tutorial
    await vpn.resetAddons('05_settings_rollback');

    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getVPNProperty('VPNAddonManager', 'count'), 10) ===
          1;
    });

    const exitCityName =
        await vpn.getVPNProperty('VPNCurrentServer', 'exitCityName');
    const exitCountryCode =
        await vpn.getVPNProperty('VPNCurrentServer', 'exitCountryCode');

    // Let's start the tutorial
    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
    await vpn.waitForQueryAndClick(
        queries.screenSettings.TIPS_AND_TRICKS.visible());
    await vpn.waitForQueryAndClick(
        queries.screenSettings.TUTORIAL_LIST_HIGHLIGHT.visible());

    // Confirmation dialog for settings-rollback
    await vpn.waitForQuery(
        queries.screenHome.TUTORIAL_POPUP_PRIMARY_BUTTON.visible());
    assert(
        (await vpn.getQueryProperty(
            queries.screenHome.TUTORIAL_POPUP_PRIMARY_BUTTON.visible(),
            'text')) === 'Continue');
    await vpn.clickOnQuery(
        queries.screenHome.TUTORIAL_POPUP_PRIMARY_BUTTON.visible());

    await vpn.waitForCondition(async () => {
      return await vpn.getVPNProperty('VPNCurrentServer', 'exitCityName') ===
          'Vienna';
    });

    assert(
        await vpn.getVPNProperty('VPNCurrentServer', 'exitCityName') ===
        'Vienna');
    assert(
        await vpn.getVPNProperty('VPNCurrentServer', 'exitCountryCode') ===
        'at');

    await vpn.waitForQuery(queries.screenHome.TUTORIAL_LEAVE.visible());
    await vpn.waitForQueryAndClick(
        queries.screenHome.SERVER_LIST_BUTTON.visible());

    // Final dialog
    await vpn.waitForQuery(
        queries.screenHome.TUTORIAL_POPUP_PRIMARY_BUTTON.visible());
    assert(
        (await vpn.getQueryProperty(
            queries.screenHome.TUTORIAL_POPUP_PRIMARY_BUTTON.visible(),
            'text')) === 'Let’s go!');
    await vpn.clickOnQuery(
        queries.screenHome.TUTORIAL_POPUP_PRIMARY_BUTTON.visible());

    await vpn.waitForCondition(async () => {
      return await vpn.getVPNProperty('VPNCurrentServer', 'exitCityName') ===
          exitCityName;
    });

    assert(
        await vpn.getVPNProperty('VPNCurrentServer', 'exitCityName') ===
        exitCityName);
    assert(
        await vpn.getVPNProperty('VPNCurrentServer', 'exitCountryCode') ===
        exitCountryCode);
  });

  describe('test message_subscription_expiring addon condition', async () => {
    async function checkForSubscriptionExpiringMessage(ctx, subscriptionExpirationCases, shouldBeAvailable) {
      for (const expiresOn of subscriptionExpirationCases) {
        const mockDetails = { ...SubscriptionDetails };
        // We are faking a Stripe subscription, so this value is expected to be in seconds.
        mockDetails.subscription.current_period_end = expiresOn / 1000;
        ctx.guardianSubscriptionDetailsCallback = () => {
          ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails'].status = 200;
          ctx.guardianOverrideEndpoints
            .GETs['/api/v1/vpn/subscriptionDetails']
            .body = mockDetails;
        };

        // Restart the VPN to load the new sub details.
        await vpn.quit();
        await startAndConnect();

        // Load all production addons.
        // These are loaded all together, so we don't know the exact number of addons.
        await vpn.resetAddons('prod');
        await vpn.waitForCondition(async () => (
          parseInt(await vpn.getVPNProperty('VPNAddonManager', 'count'), 10) > 0
        ));

        await vpn.waitForCondition(async () => {
          const loadedMessages = await vpn.messages();
          console.log(loadedMessages)
          const isSubscriptionExpiringMessageAvailable = loadedMessages.includes("message_subscription_expiring");

          return shouldBeAvailable ? isSubscriptionExpiringMessageAvailable : !isSubscriptionExpiringMessageAvailable;
        });
      }
    }

    it('message is enabled when subscription is about to expire', async () => {
      // 1 to 7 days out from expiring.
      const subscriptionExpirationCases = Array.from(
        { length: 7 },
        (_, i) => Date.now() + 1000 * 60 * 60 * 24 * (i + 1)
      );

      await checkForSubscriptionExpiringMessage(this.ctx, subscriptionExpirationCases, true);
    });

    it('message is not enabled when subscription is not about to expire', async () => {
      const subscriptionExpirationCases = [
        // Seven days out + a minute  from expiring.
        Date.now() + 1000 * 60 * 60 * 24 * 7 + 1000 * 60,
        // Eight days from expiring.
        Date.now() + 1000 * 60 * 60 * 24 * 8,
        // One month from expiring.
        Date.now() + 1000 * 60 * 60 * 24 * 30,
      ]

      await checkForSubscriptionExpiringMessage(this.ctx, subscriptionExpirationCases, false);
    });

    it('message is not enabled when subscription is already expired', async () => {
      const subscriptionExpirationCases = [
        // Literally, has just expired.
        Date.now(),
        // Has been expired for a day.
        Date.now() - 1000 * 60 * 60 * 24,
        // Has been expired for 30 days.
        Date.now() - 1000 * 60 * 60 * 24 * 30,
      ]

      await checkForSubscriptionExpiringMessage(this.ctx, subscriptionExpirationCases, false);
    });
  });
});
