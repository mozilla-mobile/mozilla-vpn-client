/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');
const {SubscriptionDetails} = require('./servers/guardian_endpoints.js');
const {env, TestingEnvironments} = require('./helper.js');

describe('Addons', function() {
  this.timeout(3000000);
  this.ctx.authenticationNeeded = true;

  it('Empty addon index', async () => {
    await vpn.resetAddons('01_empty_manifest');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getMozillaProperty(
                     'Mozilla.Shared', 'MZAddonManager', 'count'),
                 10) === 0;
    });
  });

  it('Broken addon index', async () => {
    await vpn.resetAddons('03_single_addon');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getMozillaProperty(
                     'Mozilla.Shared', 'MZAddonManager', 'count'),
                 10) === 1;
    });

    await vpn.fetchAddons('02_broken_manifest');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getMozillaProperty(
                     'Mozilla.Shared', 'MZAddonManager', 'count'),
                 10) === 1;
    });
  });

  it('Addons are loaded', async () => {
    await vpn.resetAddons('03_single_addon');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getMozillaProperty(
                     'Mozilla.Shared', 'MZAddonManager', 'count'),
                 10) === 1;
    });

    await vpn.fetchAddons('01_empty_manifest');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getMozillaProperty(
                     'Mozilla.Shared', 'MZAddonManager', 'count'),
                 10) === 0;
    });

    await vpn.fetchAddons('03_single_addon');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getMozillaProperty(
                     'Mozilla.Shared', 'MZAddonManager', 'count'),
                 10) === 1;
    });
  });

  it('Settings rollback - location', async () => {
    // Loading the custom tutorial
    await vpn.resetAddons('05_settings_rollback');

    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getMozillaProperty(
                     'Mozilla.Shared', 'MZAddonManager', 'count'),
                 10) === 1;
    });

    const exitCityName = await vpn.getMozillaProperty(
        'Mozilla.VPN', 'VPNCurrentServer', 'exitCityName');
    const exitCountryCode = await vpn.getMozillaProperty(
        'Mozilla.VPN', 'VPNCurrentServer', 'exitCountryCode');

    // Let's start the tutorial
    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
    await vpn.waitForQueryAndClick(
        queries.screenSettings.TIPS_AND_TRICKS.visible());
    await vpn.waitForQueryAndClick(
        queries.screenSettings.TUTORIAL_LIST_HIGHLIGHT.visible());

    // Confirmation dialog for settings-rollback
    await vpn.waitForQuery(
        queries.screenHome.TUTORIAL_POPUP_PRIMARY_BUTTON.visible());
    assert.equal(
        (await vpn.getQueryProperty(
            queries.screenHome.TUTORIAL_POPUP_PRIMARY_BUTTON.visible(),
            'text')),
        'Continue');
    await vpn.clickOnQuery(
        queries.screenHome.TUTORIAL_POPUP_PRIMARY_BUTTON.visible());

    await vpn.waitForCondition(async () => {
      return await vpn.getMozillaProperty(
                 'Mozilla.VPN', 'VPNCurrentServer', 'exitCityName') ===
          'Vienna';
    });

    assert.equal(
        await vpn.getMozillaProperty(
            'Mozilla.VPN', 'VPNCurrentServer', 'exitCityName'),
        'Vienna');
    assert.equal(
        await vpn.getMozillaProperty(
            'Mozilla.VPN', 'VPNCurrentServer', 'exitCountryCode'),
        'at');

    await vpn.waitForQuery(queries.screenHome.TUTORIAL_LEAVE.visible());
    await vpn.waitForQueryAndClick(
        queries.screenHome.SERVER_LIST_BUTTON.visible());

    // Final dialog
    await vpn.waitForQuery(
        queries.screenHome.TUTORIAL_POPUP_PRIMARY_BUTTON.visible());
    assert.equal(
        (await vpn.getQueryProperty(
            queries.screenHome.TUTORIAL_POPUP_PRIMARY_BUTTON.visible(),
            'text')),
        'Let’s go!');
    await vpn.clickOnQuery(
        queries.screenHome.TUTORIAL_POPUP_PRIMARY_BUTTON.visible());

    await vpn.waitForCondition(async () => {
      return await vpn.getMozillaProperty(
                 'Mozilla.VPN', 'VPNCurrentServer', 'exitCityName') ===
          exitCityName;
    });

    assert.equal(
        await vpn.getMozillaProperty(
            'Mozilla.VPN', 'VPNCurrentServer', 'exitCityName'),
        exitCityName);
    assert.equal(
        await vpn.getMozillaProperty(
            'Mozilla.VPN', 'VPNCurrentServer', 'exitCountryCode'),
        exitCountryCode);
  });

  it('test only a single update message exists at a time', async () => {
    await vpn.setVersionOverride('1.0.0');

    // Load all production addons.
    // These are loaded all together, so we don't know the exact number of
    // addons.
    await vpn.resetAddons('prod');
    await vpn.waitForCondition(
        async () =>
            (parseInt(
                 await vpn.getMozillaProperty(
                     'Mozilla.Shared', 'MZAddonManager', 'count'),
                 10) > 0));
    const loadedMessages = await vpn.messages();
    const updateMessages =
        loadedMessages.filter(message => message.startsWith('message_update_'));

    assert(
        updateMessages.length === 1,
        `We must only have a single update message at a time. ${
            updateMessages.length} were found: ${updateMessages}`);
  });

  describe('test message_subscription_expiring addon condition', async () => {
    const testCases = [
      ...Array.from(
          {length: 7},
          // 1 to 7 days out from expiring.
          (_, i) =>
              [true, () => Date.now() + 1000 * 60 * 60 * 24 * (i + 1), true,
               `is ${i + 1} day(s) away (expiring)`]),
      ...Array.from(
          {length: 7},
          // 1 to 7 days out from auto-renewing
          (_, i) =>
              [false, () => Date.now() + 1000 * 60 * 60 * 24 * (i + 1), false,
               `is ${i + 1} day(s) away (renewing)`]),
      // Seven days out + five minutes  from expiring.
      [
        true, () => Date.now() + 1000 * 60 * 60 * 24 * 7 + 1000 * 60 * 5, false,
        'is 7 days and five minutes away'
      ],
      // Seven days out + five minutes  from auto-renewing
      [
        false, () => Date.now() + 1000 * 60 * 60 * 24 * 7 + 1000 * 60 * 5,
        false, 'is 7 days and five minutes away'
      ],
      // Eight days from expiring.
      [
        true, () => Date.now() + 1000 * 60 * 60 * 24 * 8, false,
        'is 8 days away'
      ],
      // Eight days from auto-renewing
      [
        false, () => Date.now() + 1000 * 60 * 60 * 24 * 8, false,
        'is 8 days away'
      ],
      // One month from expiring.
      [
        true, () => Date.now() + 1000 * 60 * 60 * 24 * 30, false,
        'is one month away'
      ],
      // One month from auto-renewing
      [
        false, () => Date.now() + 1000 * 60 * 60 * 24 * 30, false,
        'is one month away'
      ],
      // Literally, has just expired.
      [true, () => Date.now(), false, 'just happened'],
      // Has been expired for a day.
      [true, () => Date.now() - 1000 * 60 * 60 * 24, false, 'was yesterday'],
      // Has been expired for 30 days.
      [
        true, () => Date.now() - 1000 * 60 * 60 * 24 * 30, false,
        'was last month'
      ],
    ];

    const getNextTestCase = testCases[Symbol.iterator]();
    function setNextSubscriptionExpiry(ctx) {
      const mockDetails = {...SubscriptionDetails};
      const nextTestCase = getNextTestCase.next().value;

      if (nextTestCase) {
        const [expiring, expiresOn] = nextTestCase;
        // We are faking a Stripe subscription, so this value is expected to be
        // in seconds.
        mockDetails.subscription.current_period_end = expiresOn() / 1000;
        mockDetails.subscription.cancel_at_period_end = expiring;

        ctx.guardianSubscriptionDetailsCallback = () => {
          ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
              .status = 200;
          ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
              .body = mockDetails;
        };
      }
    }

    // We call this once before all tests to set up the first test,
    // we can't use beforeEach because that is executed after the guardian
    // endpoints are overriden.
    //
    // We need to setup for the next test before it even starts for the
    // overrides to apply.
    setNextSubscriptionExpiry(this.ctx);
    afterEach(() => setNextSubscriptionExpiry(this.ctx));

    testCases.forEach(([_1, _2, shouldBeAvailable, testCase]) => {
      it(`message display is correct when subscription expiration ${testCase}`,
         async () => {
           await vpn.setVersionOverride('101.0.0');

           // Load all production addons.
           // These are loaded all together, so we don't know the exact number
           // of addons.
           await vpn.resetAddons('prod');
           await vpn.waitForCondition(
               async () =>
                   (parseInt(
                        await vpn.getMozillaProperty(
                            'Mozilla.Shared', 'MZAddonManager', 'count'),
                        10) > 0));

           // Check if the message is there or not.
           await vpn.waitForCondition(async () => {
             const loadedMessages = await vpn.messages();
             const isSubscriptionExpiringMessageAvailable =
                 loadedMessages.includes('message_subscription_expiring');
             return shouldBeAvailable ? isSubscriptionExpiringMessageAvailable :
                                        !isSubscriptionExpiringMessageAvailable;
           });
         });
    });

    it.only(
        `message display is correct when subscription expiration (no addon reload)`,
        async () => {
          if (!(await vpn.isFeatureFlippedOn('subscriptionManagement'))) {
            await vpn.flipFeatureOn('subscriptionManagement');
          }
          if ((await vpn.isFeatureFlippedOn('accountDeletion'))) {
            await vpn.flipFeatureOff('accountDeletion');
          }

          // Load all production addons.
          // These are loaded all together, so we don't know the exact number of
          // addons.
          await vpn.resetAddons('prod');
          await vpn.waitForCondition(
              async () =>
                  (parseInt(
                       await vpn.getMozillaProperty(
                           'Mozilla.Shared', 'MZAddonManager', 'count'),
                       10) > 0));

          for (const testCase of testCases) {
            const [expiring, expiresOn, shouldBeAvailable, message] = testCase;

            console.log(` - Running ${message}`);

            const mockDetails = {...SubscriptionDetails};
            mockDetails.subscription.current_period_end = expiresOn() / 1000;
            mockDetails.subscription.cancel_at_period_end = expiring;

            this.ctx.guardianSubscriptionDetailsCallback = () => {
              this.ctx.guardianOverrideEndpoints
                  .GETs['/api/v1/vpn/subscriptionDetails']
                  .status = 200;
              this.ctx.guardianOverrideEndpoints
                  .GETs['/api/v1/vpn/subscriptionDetails']
                  .body = mockDetails;
            };


            await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
            await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

            await vpn.waitForQuery(
                queries.screenSettings.USER_PROFILE.visible());
            await vpn.waitForQuery(
                queries.screenSettings.USER_PROFILE_DISPLAY_NAME.visible().prop(
                    'text', 'Test'));
            await vpn.waitForQuery(
                queries.screenSettings.USER_PROFILE_EMAIL_ADDRESS.visible()
                    .prop('text', 'test@mozilla.com'));
            await vpn.waitForQueryAndClick(
                queries.screenSettings.USER_PROFILE.visible());
            await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

            await vpn.waitForQuery(
                queries.screenSettings.SUBSCRIPTION_MANAGMENT_VIEW.visible());

            // TODO: Uncomment the assertion below once we re-enable
            // "Subscription expiring" message
            //  const loadedMessages = await vpn.messages();
            //  assert.equal(
            //      shouldBeAvailable,
            //      loadedMessages.includes('message_subscription_expiring'));

            await vpn.waitForQueryAndClick(
                queries.screenSettings.BACK.visible());
            await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

            await vpn.waitForQuery(
                queries.screenSettings.USER_PROFILE.visible());

            await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
            await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
          }
        });
  });

  it('Translations threshold', async () => {
    await vpn.resetAddons('06_translation_threshold');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getMozillaProperty(
                     'Mozilla.Shared', 'MZAddonManager', 'count'),
                 10) === 2;
    });

    await vpn.setSetting('languageCode', 'it');

    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getMozillaProperty(
                     'Mozilla.Shared', 'MZAddonManager', 'count'),
                 10) === 1;
    });

    await vpn.setSetting('languageCode', '');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getMozillaProperty(
                     'Mozilla.Shared', 'MZAddonManager', 'count'),
                 10) === 2;
    });
  });

  it('test message dismiss when the addon is disabled', async () => {
    await vpn.resetAddons('08_message_disabled');
    await vpn.waitForCondition(
        async () =>
            (parseInt(
                 await vpn.getMozillaProperty(
                     'Mozilla.Shared', 'MZAddonManager', 'count'),
                 10) > 0));
    const loadedMessages = await vpn.messages();
    const updateMessages = loadedMessages.filter(
        message => message.startsWith('message_disabled'));

    await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
    await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());
    await vpn.waitForQueryAndClick(
        queries.screenMessaging.messageItem('message_disabled').visible());
    await vpn.waitForQuery(
        queries.screenMessaging.messageView('message_disabled').visible());

    // Changing the language to change the addon condition state.
    await vpn.setSetting('languageCode', 'it');
    await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());

    // Resetting the language to 'en' to show the message again
    await vpn.setSetting('languageCode', 'en');
    await vpn.waitForQueryAndClick(
        queries.screenMessaging.messageItem('message_disabled').visible());
  });
});
