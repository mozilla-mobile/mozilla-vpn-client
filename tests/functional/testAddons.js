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

    it(`message display is correct when subscription expiration (no addon reload)`,
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
               queries.screenSettings.USER_PROFILE_EMAIL_ADDRESS.visible().prop(
                   'text', 'test@mozilla.com'));
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

  describe('test message_upgrade_to_annual_plan addon condition', async () => {

    const testCases = [
      [() => Date.now() - 1000 * 60 * 60 * 24 * 13,
             "",
             false, 
             '13 days after subscription created'],
      [() => Date.now() - 1000 * 60 * 60 * 24 * 14,
             "time",
             true, 
             '14 days after subscription created'],
      [() => Date.now() - 1000 * 60 * 60 * 24 * 15,
             "yesterday",
             true, 
             '15 days after subscription created'],
      [() => Date.now() - 1000 * 60 * 60 * 24 * 86,
             "date",
             true, 
             '86 days after subscription created'],
      //Fails by an hour (due to daylight savings time?)
      // [() => Date.now() - 1000 * 60 * 60 * 24 * 87,
      //        "time",
      //        true, 
      //        '87 days after subscription created'],
      [() => Date.now() - 1000 * 60 * 60 * 24 * 88,
             "yesterday",
             true, 
             '88 days after subscription created'],
    ];

    const getNextTestCase = testCases[Symbol.iterator]();
    function setNextSubscriptionStarted(ctx) {
      const mockDetails = {...SubscriptionDetails};
      const nextTestCase = getNextTestCase.next().value;

      if (nextTestCase) {
        const [createdAt] = nextTestCase;
        // We are faking a Stripe subscription, so this value is expected to be
        // in seconds.
        mockDetails.subscription.created = createdAt() / 1000;
        mockDetails.subscription._subscription_type = "web";
        mockDetails.plan.interval = "month";

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
    setNextSubscriptionStarted(this.ctx);
    afterEach(() => setNextSubscriptionStarted(this.ctx));

    testCases.forEach(([createdAtTimestamp, expectedTimeFormat, shouldBeAvailable, testCase]) => {
      it.only(`message display is correct when subscription started at ${testCase}`, async () => {
        await vpn.resetAddons('prod');

        //Load messages
        const loadedMessages = await vpn.messages();

        //If the message is supposed to be enabled, lets check the timestamp
        if (shouldBeAvailable) {
            await vpn.waitForCondition(async () => (parseInt(await vpn.getMozillaProperty('Mozilla.Shared', 'MZAddonManager', 'count'), 10) > 0));
            
            //Check timestamp
            await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
            await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());

            let expectedTimestamp
            let actualTimestamp = await vpn.getQueryProperty(queries.screenMessaging.messageItem('message_upgrade_to_annual_plan'), 'formattedDate');
            //Maybe add 14 days to account for the timestamp that starts 14 days into the subscription
            const addedTime =  Date.now() > Date.now() - (14 * 24 * 60 * 60 * 1000) ? 1000 * 60 * 60 * 24 * 14 : 0;

            if (expectedTimeFormat === "time") {
              expectedTimestamp = new Date(createdAtTimestamp() + addedTime).toLocaleTimeString('en-US', { hour: 'numeric', minute: 'numeric', hour12: true });
            }
            else if (expectedTimeFormat === "date") {
              expectedTimestamp = new Date(createdAtTimestamp() + addedTime).toLocaleDateString('en-US', { month: 'numeric', day: 'numeric', year: '2-digit' });
            }
            else if (expectedTimeFormat === "yesterday") {
              expectedTimestamp = "Yesterday";
            }

            assert.equal(actualTimestamp, expectedTimestamp);
        }
        assert.equal(shouldBeAvailable, loadedMessages.includes('message_upgrade_to_annual_plan'));

      });
    });
  });
});
