
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 import { strictEqual } from 'assert';
 import { runningOnWasm, authenticateInApp, gleanTestGetValue, waitForQueryAndClick, testLastInteractionEvent, setQueryProperty, waitForQuery, getOneEventOfType } from './helper.js';
 import { screenSubscriptionNeeded, screenInBrowserSubscriptionLoading } from './queries.js';
 import { startAndConnect } from './setupVpn.js';

describe("subscription needed tests", function() {
  // TODO (VPN-5711): Create tests for this screen.

  describe('subscription needed related telemetry tests', function() {
    if(runningOnWasm()) {
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
        await authenticateInApp();
      });

      it("impression event is recorded", async () => {
        const  subNeededImpression = await gleanTestGetValue("impression", "subscriptionNeededScreen", "main");
        strictEqual(subNeededImpression.length, 1)
        const enterEmailViewEventExtras = subNeededImpression[0].extra;
        strictEqual(screen, enterEmailViewEventExtras.screen);
      })

      it("get help event is recorded", async () => {
        // Click the "Get help" button
        await waitForQueryAndClick(screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_GET_HELP.visible());
        await testLastInteractionEvent({
            eventName: "getHelpSelected",
            screen
        });
      });

      it("legal disclaimer events are recorded", async () => {
        // Click the "Terms of Service" button
        await waitForQueryAndClick(screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_TERMS_OF_SERVICE.visible());
        await testLastInteractionEvent({
            eventName: "termsOfServiceSelected",
            screen
        });

        // Click the "Privacy Notice" button
        await waitForQueryAndClick(screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_PRIVACY_NOTICE.visible());
        await testLastInteractionEvent({
            eventName: "privacyNoticeSelected",
            screen
        });
      });

      it("sign out event is recorded", async () => {
        // Click the "Sign out" button
        await waitForQueryAndClick(screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_SIGN_OUT.visible());
        await testLastInteractionEvent({
            eventName: "signOutSelected",
            screen
        });
      });

      it("subscribe now button event is recorded", async () => {
        // Click the "Subscribe now" button
        await waitForQueryAndClick(screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_BUTTON.visible());
        const subscriptionNeededViewEvents = await gleanTestGetValue("impression", "subscriptionNeededScreen", "main");
        const subscriptionNeededViewEventExtras = subscriptionNeededViewEvents[0].extra;
        strictEqual(screen, subscriptionNeededViewEventExtras.screen);
      });

      it("restore purchase event is recorded", async () => {
        // Make the "Already a subscriber?" button visible
        await setQueryProperty(screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_RESTORE_PURCHASE, "visible", true)
        // Click the "Already a subscriber?" button
        await waitForQueryAndClick(screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_RESTORE_PURCHASE.visible());
        await testLastInteractionEvent({
            eventName: "alreadyASubscriberSelected",
            screen
        });
      });
    });

    describe('loading screens', function() {
      const screen = "continue_in_browser"

      it("impression event is recorded for in browser screen", async () => {
        await authenticateInApp();
        // Click the "Subscribe now" button
        await waitForQueryAndClick(screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_BUTTON.visible());
        // Wait for the loading screen to show up
        await waitForQuery(screenInBrowserSubscriptionLoading.SUBSCRIPTION_LOADING_VIEW.visible());

        const subscriptionNeededViewEvent = await getOneEventOfType({
          eventCategory: "impression",
          eventName: "continueInBrowserScreen",
          screen,
          expectedEventCount: 1
        });
        strictEqual(subscriptionNeededViewEvent.extra.screen, screen);
      });

      // TODO (VPN-4784, VPN-4783): This cannot be tested util we are able to run
      // functional tests in mobile. In app purchase screens are mobile only.
      it.skip("impression event is recorded for in app screen");

      it("cancel button event is recorded (in browser only)", async () => {
        await authenticateInApp();
        // Click the "Subscribe now" button
        await waitForQueryAndClick(screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_BUTTON.visible());
        // Click on the "Cancel" button
        await waitForQueryAndClick(screenInBrowserSubscriptionLoading.SUBSCRIPTION_LOADING_CANCEL.visible());
        await testLastInteractionEvent({
          eventName: "cancelSelected",
          screen
        });
      });
    });
  });
});
