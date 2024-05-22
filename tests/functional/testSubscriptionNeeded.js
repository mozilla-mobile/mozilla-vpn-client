
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 const assert = require('assert');
 const vpn = require('./helper.js');
 const queries = require('./queries.js');
 const { startAndConnect } = require('./setupVpn.js');

describe("subscription needed tests", function() {
  // TODO (VPN-5711): Create tests for this screen.

  describe('subscription needed related telemetry tests', function() {
    if(vpn.runningOnWasm()) {
      return;
    }

    this.ctx.guardianOverrideEndpoints = {
      POSTs: {
        '/api/v2/vpn/login/verify': {
          status: 200,
          body: {
            user: {
              avatar: '',
              display_name: 'Test',
              email: 'test@mozilla.com',
              max_devices: 5,
              // This is the important bit.
              // The user data has no active subscriptions.
              subscriptions: {},
              devices: [],
            },
            token: 'our-token'
          }
        }
      }
    };

    // Telemetry design is detailed at:
    // https://miro.com/app/board/uXjVM0BZcnc=/?share_link_id=228137467679

    describe('subscription screen', function() {
      const screen = "subscription_needed";

      beforeEach(async () => {
        await vpn.authenticateInApp();
      });

      it("impression event is recorded", async () => {
        const  subNeededImpression = await vpn.waitForGleanValue("impression", "subscriptionNeededScreen", "main");
        assert.strictEqual(subNeededImpression.length, 1)
        const enterEmailViewEventExtras = subNeededImpression[0].extra;
        assert.strictEqual(screen, enterEmailViewEventExtras.screen);
      })

      it('Help event is recorded', async () => {
        // Click the "Help" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_GET_HELP.visible());
        await vpn.testLastInteractionEvent({
            eventName: "getHelpSelected",
            screen
        });
      });

      it("legal disclaimer events are recorded", async () => {
        // Click the "Terms of Service" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_TERMS_OF_SERVICE.visible());
        await vpn.testLastInteractionEvent({
            eventName: "termsOfServiceSelected",
            screen
        });

        // Click the "Privacy Notice" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_PRIVACY_NOTICE.visible());
        await vpn.testLastInteractionEvent({
            eventName: "privacyNoticeSelected",
            screen
        });
      });

      it("sign out event is recorded", async () => {
        // Click the "Sign out" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_SIGN_OUT.visible());
        await vpn.testLastInteractionEvent({
            eventName: "signOutSelected",
            screen
        });
      });

      it("subscribe now button event is recorded", async () => {
        // Click the "Subscribe now" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_BUTTON.visible());
        const subscriptionNeededViewEvents = await vpn.waitForGleanValue("impression", "subscriptionNeededScreen", "main");
        const subscriptionNeededViewEventExtras = subscriptionNeededViewEvents[0].extra;
        assert.strictEqual(screen, subscriptionNeededViewEventExtras.screen);
      });

      it("restore purchase event is recorded", async () => {
        // Make the "Already a subscriber?" button visible
        await vpn.setQueryProperty(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_RESTORE_PURCHASE, "visible", true)
        // Click the "Already a subscriber?" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_RESTORE_PURCHASE.visible());
        await vpn.testLastInteractionEvent({
            eventName: "alreadyASubscriberSelected",
            screen
        });
      })

      it("successfull outcome events are recorded", async () => {
        // Click the "Subscribe now" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_BUTTON.visible());
        const startedEvents = await vpn.waitForGleanValue("outcome", "subscriptionStarted", "main");
        assert.strictEqual(startedEvents.length, 1);

        // Override directly in the running guardian server for this test.
        this.ctx.guardianServer.overrideEndpoints = {
          POSTs: {
            '/api/v2/vpn/login/verify': {
              status: 200,
              body: {
                user: {
                  avatar: '',
                  display_name: 'Test',
                  email: 'test@mozilla.com',
                  max_devices: 5,
                  subscriptions: {vpn: {active: true}},
                  devices: [],
                },
                token: 'our-token'
              }
            }
          }
        };

        // Subscription, for Guardian, is the same as in-browser auth.
        await vpn.mockInBrowserAuthentication();

        // Check subscription completed is recorded.

        const completedEvents = await vpn.waitForGleanValue("outcome", "subscriptionCompleted", "main");
        assert.strictEqual(completedEvents.length, 1);

        // Reset the Gaurdian overrides.
        this.ctx.guardianServer.overrideEndpoints = this.ctx.guardianOverrideEndpoints;
      });

      // TODO (VPN-4784, VPN-4783): This cannot be tested util we are able to run
      // functional tests in mobile. Failure events only happen in mobile,
      // desktop in browser subscription failure state is registered as a cancelation
      // and we don't record cancelation events.
      it.skip("failure outcome events are recorded");
    });

    describe('loading screens', function() {
      const screen = "continue_in_browser"

      it("impression event is recorded for in browser screen", async () => {
        await vpn.authenticateInApp();
        // Click the "Subscribe now" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_BUTTON.visible());
        // Wait for the loading screen to show up
        await vpn.waitForQuery(queries.screenInBrowserSubscriptionLoading.SUBSCRIPTION_LOADING_VIEW.visible());

        const subscriptionNeededViewEvent = await vpn.getOneEventOfType({
          eventCategory: "impression",
          eventName: "continueInBrowserScreen",
          screen,
          expectedEventCount: 1
        });
        assert.strictEqual(subscriptionNeededViewEvent.extra.screen, screen);
      });

      // TODO (VPN-4784, VPN-4783): This cannot be tested util we are able to run
      // functional tests in mobile. In app purchase screens are mobile only.
      it.skip("impression event is recorded for in app screen");

      it("cancel button event is recorded (in browser only)", async () => {
        await vpn.authenticateInApp();
        // Click the "Subscribe now" button
        await vpn.waitForQueryAndClick(queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_BUTTON.visible());
        // Click on the "Cancel" button
        await vpn.waitForQueryAndClick(queries.screenInBrowserSubscriptionLoading.SUBSCRIPTION_LOADING_CANCEL.visible());
        // Check subscription cancelled event is recorded.
        const eventList = await vpn.waitForGleanValue("outcome", "subscriptionFailed", "main");

        assert.strictEqual(eventList.length, 1);
        assert.strictEqual(eventList[0].extra.reason, "SubscriptionCancelled");

      });
    });
  });
});
