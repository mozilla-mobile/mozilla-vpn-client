/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');
const queries = require('./queries.js');
const fxaEndpoints = require('./servers/fxa_endpoints.js')
const {validators} = require('./servers/guardian_endpoints.js');
const http = require('http')

const SUBSCRIPTION_DETAILS = {
  plan: {amount: 123, currency: 'usd', interval: 'year', interval_count: 1},
  payment: {
    payment_provider: 'stripe',
    payment_type: 'credit',
    last4: '1234',
    exp_month: 12,
    exp_year: 2022,
    brand: 'visa',
  },
  subscription: {
    _subscription_type: 'web',
    created: 1,
    current_period_end: 2,
    cancel_at_period_end: true,
    status: 'active',
    product_id: 'testId'
  },
};

const SUBSCRIPTION_DETAILS_MONTHLY = {
  plan: {amount: 123, currency: 'usd', interval: 'month', interval_count: 1},
  payment: {
    payment_provider: 'stripe',
    payment_type: 'credit',
    last4: '1234',
    exp_month: 12,
    exp_year: 2022,
    brand: 'visa',
  },
  subscription: {
    _subscription_type: 'web',
    created: 1,
    current_period_end: 2,
    cancel_at_period_end: true,
    status: 'active',
    product_id: 'testId'
  },
};

describe('Subscription manager', function() {
  describe('Expired subscription', function() {
    this.timeout(30000);

    // There are two constants defined below: userDataActive and
    // userDataInactive which are used to override the Guardian endpoint to mock
    // a subscription expiration by changing the VPN subscription from true to
    // false. Because this override should be happening on the same device, the
    // device is defined here and used later in both userDataActive and
    // userDataInactive. Simply copying the same device definition in both cases
    // will actually result in two distinct objects, causing the test to fail.
    const device = {
      name: 'Current device',
      unique_id: '',
      pubkey: '',
      ipv4_address: '127.0.0.1',
      ipv6_address: '::1',
      created_at: new Date().toISOString()
    };

    const userDataActive = {
      avatar: '',
      display_name: 'Test',
      email: 'test@mozilla.com',
      max_devices: 5,
      subscriptions: {vpn: {active: true}},
      devices: [device],
    };
    const userDataInactive = {
      avatar: '',
      display_name: 'Test test',
      email: 'test@mozilla.com',
      max_devices: 5,
      subscriptions: {vpn: {active: false}},
      devices: [device]
    };

    this.ctx.guardianOverrideEndpoints = {
      GETs: {
        '/api/v1/vpn/account': {
          status: 200,
          requiredHeaders: ['Authorization'],
          body: userDataActive
        }
      },
      POSTs: {
        '/api/v1/vpn/device': {
          status: 201,
          requiredHeaders: ['Authorization'],
          bodyValidator: validators.guardianDevice,
          callback: (req) => {
            device.name = req.body.name;
            device.pubkey = req.body.pubkey;
            device.unique_id = req.body.unique_id;
          },
          body: {}
        },
      }
    };

    it('Prompts SubscriptionNeeded if a user\'s subscription is inactive on connection',
       async () => {
         // This test verifies the case where a user is logged in
         // but the VPN is off when their subscription expires.
         // When they try to turn the VPN on, they get the
         // "Subscribe to Mozilla VPN" screen.

         await vpn.authenticateInApp();

         // Step 1: Override the Guardian endpoint to mock an expired
         // subscription.
         this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/account'].body =
             userDataInactive;

         await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
         await vpn.clickOnQuery(queries.screenHome.CONTROLLER_TOGGLE.visible());

         // Step 2: Verify that user gets the "Subscribe to Mozilla VPN" screen.
         await vpn.waitForQuery(queries.screenSubscriptionNeeded
                                    .SUBSCRIPTION_NEEDED_VIEW.visible());

         // Reset guardian endpoint for the next test
         this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/account'].body =
             userDataActive;
       });

    it('Returns user to Main Screen after user successfully completes Web Subscription flow',
       async () => {
         // This test verifies the case where a user without an active
         // subscription logs in and is taken to the "Subscribtion Needed"
         // screen. Once they click on the "Subscribe Now" button, they will be
         // taken to the browser to finish subscription and then will be then
         // redirected back to the controller home screen.

         if (!this.ctx.wasm) {
           await vpn.authenticateInApp();

           // Mark the user subscription as inactive
           this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/account'].body =
               userDataInactive;

           await vpn.waitForQuery(
               queries.screenHome.CONTROLLER_TITLE.visible());
           await vpn.clickOnQuery(
               queries.screenHome.CONTROLLER_TOGGLE.visible());

           // Verify that user gets the "Subscribe to Mozilla VPN" screen.
           await vpn.waitForQuery(queries.screenSubscriptionNeeded
                                      .SUBSCRIPTION_NEEDED_VIEW.visible());


           // Click on the Subscribe Now button.
           await vpn.waitForQueryAndClick(
               queries.screenSubscriptionNeeded.SUBSCRIPTION_NEEDED_BUTTON
                   .visible());

           await vpn.waitForCondition(async () => {
             const url = await vpn.getLastUrl();
             return url.includes('/api/v2/vpn/login');
           });

           // Mark the user subscription as active
           this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/account'].body =
               userDataActive;

           // We don't really want to go through the
           // authentication flow because we
           // are mocking everything. So this next chunk of code manually
           // makes a call to the DesktopAuthenticationListener to mock
           // a successful authentication in browser.
           const url = await vpn.getLastUrl();
           const authListenerPort = (new URL(url)).searchParams.get('port');
           const options = {
             // We hardcode 127.0.0.1 to match listening on
             // QHostAddress:LocalHost
             // and hardcoded in guardian's vpnClientPixelImageAuthUrl
             hostname: '127.0.0.1',
             port: parseInt(authListenerPort, 10),
             path: '/?code=the_code',
             method: 'GET',
           };

           await new Promise(resolve => {
             const req = http.request(options, res => {});
             req.on('close', resolve);
             req.on('error', error => {
               throw new Error(
                   `Unable to connect to ${urlObj.hostname} to complete the
                  auth. ${error.name}, ${error.message}, ${error.stack}`);
             });
             req.end();
           });

           // Wait for VPN client screen to move from spinning wheel to next
           // screen
           await vpn.waitForQuery(
               queries.screenHome.CONTROLLER_TITLE.visible());
           assert.equal(
               await vpn.getQueryProperty(
                   queries.screenHome.CONTROLLER_TITLE, 'text'),
               'VPN is off');
         }
       });

    it('Continues to try connecting if call to check subscription status fails',
       async () => {
         // This test verifies the case where user is logged in
         // but the VPN is off when their subscription expires,
         // and the client encounters an HTTP error when completing
         // the API call to check subscription status.
         // The expectation here is that the VPN toggles on successfully.

         this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/account'].body =
             userDataActive;

         await vpn.authenticateInApp();

         // Step 1: Override the Guardian endpoint to mock an expired
         // subscription. Set the error status to 500.
         this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/account'].body =
             userDataInactive;
         this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/account'].status =
             500;

         await vpn.activate();
         await vpn.waitForCondition(async () => {
           return await vpn.getQueryProperty(
                      queries.screenHome.CONTROLLER_TITLE, 'text') ==
               'VPN is on';
         });
         // Reset guardian endpoint for the next test
         this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/account'].body =
             userDataActive;
         this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/account'].status =
             200;
       });

    it('Go to "Subscribe to Mozilla VPN" screen once user toggles off VPN after subscription expires and they enter No Signal',
       async () => {
         // This test verifies the case where a user is logged in
         // and the VPN is on when their subscription expires.
         // They enter No Signal, and once they toggle the VPN off
         // they get the "Subscribe to Mozilla VPN" screen.

         await vpn.authenticateInApp();

         // toggle on VPN here
         await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
         await vpn.clickOnQuery(queries.screenHome.CONTROLLER_TOGGLE.visible());

         await vpn.waitForCondition(async () => {
           return await vpn.getQueryProperty(
                      queries.screenHome.CONTROLLER_TITLE, 'text') ==
               'VPN is on';
         });

         // Override the Guardian endpoint to mock an expired
         // subscription.
         this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/account'].body =
             userDataInactive;

         // Because the subscription has expired, client enter the No Signal
         // state
         await vpn.forceConnectionStabilityStatus('nosignal');

         // Once the VPN is toggled off, we are redirected to the "Subscribe to
         // Mozilla VPN" screen.
         await vpn.clickOnQuery(queries.screenHome.CONTROLLER_TOGGLE.visible());
         await vpn.waitForQuery(queries.screenSubscriptionNeeded
                                    .SUBSCRIPTION_NEEDED_VIEW.visible());
       });
  });
});

describe('Subscription view', function() {
  this.timeout(3000000);
  this.ctx.authenticationNeeded = true;
  this.ctx.guardianOverrideEndpoints = {
    GETs: {
      '/api/v1/vpn/subscriptionDetails': {
        status: 200,
        requiredHeaders: ['Authorization'],
        body: null,
        callback: (req) => this.ctx.guardianSubscriptionDetailsCallback(req)
      },
    },
    POSTs: {},
    DELETEs: {},
  };
  this.ctx.fxaOverrideEndpoints = {
    GETs: {
      '/v1/account/attached_clients': {
        requiredHeaders: ['Authorization'],
        status: 200,
        body: [],
      },
    },
    POSTs: {
      '/v1/account/login': {
        status: 200,
        bodyValidator: fxaEndpoints.validators.fxaLogin,
        body: null,
        callback: (req) => this.ctx.fxaLoginCallback(req)
      },

      '/v1/session/verify/totp': {
        status: 200,
        requiredHeaders: ['Authorization'],
        bodyValidator: fxaEndpoints.validators.fxaVerifyTotp,
        body: null,
        callback: (req) => this.ctx.fxaTotpCallback(req)
      },

      '/v1/account/destroy': {
        status: 200,
        requiredHeaders: ['Authorization'],
        body: null,
        callback: (req) => this.ctx.fxaDestroyCallback(req)
      },
    },
    DELETEs: {},
  };

  this.ctx.resetCallbacks = () => {
    this.ctx.fxaLoginCallback = (req) => {
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
        sessionToken: 'session',
        verified: true,
        verificationMethod: '',
      };
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;
    };

    this.ctx.fxaTotpCallback = (req) => {
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify/totp'].body = {
        success: true
      }
    };

    this.ctx.guardianSubscriptionDetailsCallback = req => {
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .status = 200;
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .body = SUBSCRIPTION_DETAILS;
    };
  };

  this.ctx.resetCallbacks();

  beforeEach(async () => {
    if (!(await vpn.isFeatureFlippedOn('subscriptionManagement'))) {
      await vpn.flipFeatureOn('subscriptionManagement');
    }
    if ((await vpn.isFeatureFlippedOn('accountDeletion'))) {
      await vpn.flipFeatureOff('accountDeletion');
    }
  });

  afterEach(() => {
    this.ctx.resetCallbacks();
  });

  it('Checking the logout', async () => {
    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
    await vpn.waitForQueryAndClick(
        queries.screenSettings.USER_PROFILE.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    await vpn.waitForQuery(
        queries.screenSettings.SIGN_OUT.visible());
    await vpn.waitForQueryAndClick(
        queries.screenSettings.SIGN_OUT.visible());
    await vpn.waitForInitialView();
  });

  it('record telemetry when user clicks on Sign out in the Settings screen',
     async () => {
       if (this.ctx.wasm) {
         // This test cannot run in wasm
         return;
       }

       await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
       await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
       await vpn.waitForQueryAndClick(
           queries.screenSettings.USER_PROFILE.visible());
       await vpn.waitForQuery(
           queries.screenSettings.subscriptionView.SCREEN.visible());
       await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

       await vpn.waitForQuery(
           queries.screenSettings.subscriptionView.SIGN_OUT.visible());
       await vpn.scrollToQuery(
           queries.screenSettings.subscriptionView.FLICKABLE,
           queries.screenSettings.subscriptionView.SIGN_OUT.visible());

       await vpn.waitForQueryAndClick(
           queries.screenSettings.subscriptionView.SIGN_OUT.visible());
     });

  async function openSubscriptionManagement() {
    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
    await vpn.waitForQueryAndClick(
        queries.screenSettings.USER_PROFILE.visible());
    await vpn.waitForQuery(queries.screenSettings.subscriptionView.SCREEN.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
  }
});
