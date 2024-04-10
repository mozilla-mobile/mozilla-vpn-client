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
    status: 'active'
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
    status: 'active'
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

  it('Authentication needed - sample', async () => {
    this.ctx.fxaLoginCallback = (req) => {
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
        sessionToken: 'session',
        verified: true,
        verificationMethod: ''
      };
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;
    };
    this.ctx.guardianSubscriptionDetailsCallback = req => {
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .status = 401;
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .body = {}
    };

    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());
    await vpn.waitForQuery(
        queries.screenSettings.USER_PROFILE_DISPLAY_NAME.visible().prop(
            'text', 'Test'));
    await vpn.waitForQuery(
        queries.screenSettings.USER_PROFILE_EMAIL_ADDRESS.visible().prop(
            'text', 'test@mozilla.com'));
    await vpn.waitForQueryAndClick(
        queries.screenSettings.USER_PROFILE.visible());

    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

    await vpn.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible());
    await vpn.setQueryProperty(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible(),
        'text', 'P4ssw0rd!!');
    await vpn.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
            .enabled());

    this.ctx.guardianSubscriptionDetailsCallback = req => {
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .status = 200;
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .body = SUBSCRIPTION_DETAILS;
    };

    await vpn.waitForQueryAndClick(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
            .enabled());

    await vpn.waitForQuery(
        queries.screenSettings.subscriptionView.SCREEN.visible());
  });

  it('Authentication needed - totp', async () => {
    this.ctx.guardianSubscriptionDetailsCallback = req => {
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .status = 401;
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .body = {}
    };

    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());
    await vpn.waitForQuery(
        queries.screenSettings.USER_PROFILE_DISPLAY_NAME.visible().prop(
            'text', 'Test'));
    await vpn.waitForQuery(
        queries.screenSettings.USER_PROFILE_EMAIL_ADDRESS.visible().prop(
            'text', 'test@mozilla.com'));
    await vpn.waitForQueryAndClick(
        queries.screenSettings.USER_PROFILE.visible());

    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

    await vpn.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible());
    await vpn.setQueryProperty(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible(),
        'text', 'P4ssw0rd!!');

    await vpn.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
            .enabled());

    this.ctx.fxaLoginCallback = (req) => {
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
        sessionToken: 'session',
        verified: false,
        verificationMethod: 'totp-2fa'
      };
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;
    };

    await vpn.waitForQueryAndClick(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
            .enabled());

    await vpn.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible());
    await vpn.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
            .disabled());
    await vpn.setQueryProperty(
        queries.screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible(),
        'text', '123456');
    await vpn.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible().enabled());

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

    await vpn.waitForQueryAndClick(
        queries.screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible().enabled());

    await vpn.waitForQuery(
        queries.screenSettings.subscriptionView.SCREEN.visible());
  });

  it('Playing with the subscription view', async () => {
    this.ctx.fxaLoginCallback = (req) => {
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
        sessionToken: 'session',
        verified: true,
        verificationMethod: ''
      };
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;
    };

    const tests = [
      {
        name: 'plan: USD - yearly',
        plan: {
          value: {
            amount: 123,
            currency: 'usd',
            interval: 'year',
            interval_count: 1
          },
          expected: '$1.23 Yearly + tax'
        }
      },
      {
        name: 'plan: USD - half-yearly',
        plan: {
          value: {
            amount: 123,
            currency: 'usd',
            interval: 'month',
            interval_count: 6
          },
          expected: '$1.23 Half-yearly + tax'
        }
      },
      {
        name: 'plan: USD - monthly',
        plan: {
          value: {
            amount: 123,
            currency: 'usd',
            interval: 'month',
            interval_count: 1
          },
          expected: '$1.23 Monthly + tax'
        }
      },
      {
        name: 'plan: EUR - yearly',
        plan: {
          value: {
            amount: 4567,
            currency: 'eur',
            interval: 'year',
            interval_count: 1
          },
          expected: '€45.67 Yearly'
        }
      },
      {
        name: 'plan: EUR - half-yearly',
        plan: {
          value: {
            amount: 1,
            currency: 'eur',
            interval: 'month',
            interval_count: 6
          },
          expected: '€0.01 Half-yearly'
        }
      },
      {
        name: 'plan: EUR - monthly',
        plan: {
          value: {
            amount: 1234567,
            currency: 'eur',
            interval: 'month',
            interval_count: 1
          },
          expected: '€12,345.67 Monthly'
        }
      },
      {
        name: 'payment: Mastercard',
        payment: {
          value: {
            payment_provider: 'stripe',
            payment_type: 'credit',
            last4: '0000',
            exp_month: 1,
            exp_year: 2022,
            brand: 'mastercard'
          },
          expected: {
            card: 'Card ending in 0000',
            expires: 'January 2022',
            brand: 'Mastercard'
          }
        }
      },
      {
        name: 'payment: Mastercard (debit)',
        payment: {
          value: {
            payment_provider: 'stripe',
            payment_type: 'debit',
            last4: '0000',
            exp_month: 1,
            exp_year: 2022,
            brand: 'mastercard'
          },
          expected: {
            card: 'Card ending in 0000',
            expires: 'January 2022',
            brand: 'Mastercard'
          }
        }
      },
      {
        name: 'payment: Mastercard (prepaid)',
        payment: {
          value: {
            payment_provider: 'stripe',
            payment_type: 'prepaid',
            last4: '0000',
            exp_month: 1,
            exp_year: 2022,
            brand: 'mastercard'
          },
          expected: {
            card: 'Card ending in 0000',
            expires: 'January 2022',
            brand: 'Mastercard'
          }
        }
      },
      {
        name: 'payment: Visa',
        payment: {
          value: {
            payment_provider: 'stripe',
            payment_type: 'credit',
            last4: '0000',
            exp_month: 1,
            exp_year: 2022,
            brand: 'visa'
          },
          expected: {
            card: 'Card ending in 0000',
            expires: 'January 2022',
            brand: 'Visa'
          }
        }
      },
      {
        name: 'payment: Visa (prepaid)',
        payment: {
          value: {
            payment_provider: 'stripe',
            payment_type: 'prepaid',
            last4: '0000',
            exp_month: 1,
            exp_year: 2022,
            brand: 'visa'
          },
          expected: {
            card: 'Card ending in 0000',
            expires: 'January 2022',
            brand: 'Visa'
          }
        }
      },
      {
        name: 'payment: american express',
        payment: {
          value: {
            payment_provider: 'stripe',
            payment_type: 'credit',
            last4: '0000',
            exp_month: 1,
            exp_year: 2022,
            brand: 'amex'
          },
          expected: {
            card: 'Card ending in 0000',
            expires: 'January 2022',
            brand: 'American Express'
          }
        }
      },
      {
        name: 'payment: piece of eight',
        payment: {
          value: {
            payment_provider: 'stripe',
            payment_type: 'credit',
            last4: '9999',
            exp_month: 10,
            exp_year: 1492,
            brand: 'piece of eight'
          },
          expected:
              {card: 'Card ending in 9999', expires: 'October 1492', brand: ''}
        }
      },
      {
        name: 'payment: apple',
        payment: {
          value: {
            payment_provider: 'iap_apple',
          },
          expected: {payment: 'Apple subscription'}
        }
      },
      {
        name: 'payment: google',
        payment: {
          value: {
            payment_provider: 'iap_google',
          },
          expected: {payment: 'Google subscription'}
        }
      },
      {
        name: 'web subscription: cancelled',
        subscription: {
          value: {
            _subscription_type: 'web',
            created: 23677200,
            current_period_end: 2147483647,
            cancel_at_period_end: false,
            status: 'inactive'
          },
          expected: {
            activated: '10/2/70',
            cancelled: '1/19/38',
            label: 'Next billed',
            status: 'Inactive'
          }
        },
        manageSubscriptionLink:
            'https://accounts.stage.mozaws.net/subscriptions',
      },
      {
        name: 'web subscription: not cancelled',
        subscription: {
          value: {
            _subscription_type: 'web',
            created: 23677200,
            current_period_end: 2147483647,
            cancel_at_period_end: true,
            status: 'inactive'
          },
          expected: {
            activated: '10/2/70',
            cancelled: '1/19/38',
            label: 'Expires',
            status: 'Inactive'
          }
        },
        manageSubscriptionLink:
            'https://accounts.stage.mozaws.net/subscriptions',
      },
      {
        name: 'web subscription: active',
        subscription: {
          value: {
            _subscription_type: 'web',
            created: 23677200,
            current_period_end: 2147483647,
            cancel_at_period_end: true,
            status: 'active'
          },
          expected: {
            activated: '10/2/70',
            cancelled: '1/19/38',
            label: 'Expires',
            status: 'Active'
          }
        },
        manageSubscriptionLink:
            'https://accounts.stage.mozaws.net/subscriptions',
      },
      {
        name: 'web subscription: inactive',
        subscription: {
          value: {
            _subscription_type: 'web',
            created: 23677200,
            current_period_end: 2147483647,
            cancel_at_period_end: true,
            status: 'inactive'
          },
          expected: {
            activated: '10/2/70',
            cancelled: '1/19/38',
            label: 'Expires',
            status: 'Inactive'
          }
        },
        manageSubscriptionLink:
            'https://accounts.stage.mozaws.net/subscriptions',
      },
      {
        name: 'web subscription: trailing',
        subscription: {
          value: {
            _subscription_type: 'web',
            created: 23677200,
            current_period_end: 2147483647,
            cancel_at_period_end: true,
            status: 'trailing'
          },
          expected: {
            activated: '10/2/70',
            cancelled: '1/19/38',
            label: 'Expires',
            status: 'Inactive'
          }
        },
        manageSubscriptionLink:
            'https://accounts.stage.mozaws.net/subscriptions',
      },
      {
        name: 'apple subscription: auto renew',
        subscription: {
          value: {
            _subscription_type: 'iap_apple',
            expiry_time_millis: 2147483647,
            auto_renewing: true,
          },
          expected:
              {cancelled: '1/25/70', label: 'Next billed', status: 'Inactive'}
        },
        manageSubscriptionLink: 'https://apps.apple.com/account/subscriptions',
      },
      {
        name: 'apple subscription: no auto renew',
        subscription: {
          value: {
            _subscription_type: 'iap_apple',
            expiry_time_millis: 2147483647,
            auto_renewing: false,
          },
          expected: {cancelled: '1/25/70', label: 'Expires', status: 'Inactive'}
        },
        manageSubscriptionLink: 'https://apps.apple.com/account/subscriptions',
      },
      {
        name: 'google subscription: auto renew',
        subscription: {
          value: {
            _subscription_type: 'iap_google',
            expiry_time_millis: 2147483647000,
            auto_renewing: true,
          },
          expected:
              {cancelled: '1/19/38', label: 'Next billed', status: 'Active'}
        },
        manageSubscriptionLink:
            'https://play.google.com/store/account/subscriptions',
      },
      {
        name: 'google subscription: no auto renew',
        subscription: {
          value: {
            _subscription_type: 'iap_google',
            expiry_time_millis: 2147483647000,
            auto_renewing: false,
          },
          expected: {cancelled: '1/19/38', label: 'Expires', status: 'Active'}
        },
        manageSubscriptionLink:
            'https://play.google.com/store/account/subscriptions',
      },
      {
        name: 'apple subscription: no payment data',
        subscription: {
          value: {
            _subscription_type: 'iap_apple',
            expiry_time_millis: 2147483647,
            auto_renewing: true,
          },
          expected:
              {cancelled: '1/25/70', label: 'Next billed', status: 'Inactive'}
        },
        payment: {value: {}, expected: {payment: 'Apple subscription'}},
      },
      {
        name: 'google subscription: no payment data',
        subscription: {
          value: {
            _subscription_type: 'iap_google',
            expiry_time_millis: 2147483647,
            auto_renewing: true,
          },
          expected:
              {cancelled: '1/25/70', label: 'Next billed', status: 'Inactive'}
        },
        payment: {value: {}, expected: {payment: 'Google subscription'}},
      },
    ];

    for (let data of tests) {
      // Default values
      if (!('plan' in data)) {
        data.plan = {
          value: {
            amount: 123,
            currency: 'usd',
            interval: 'year',
            interval_count: 1
          },
          expected: '$1.23 Yearly + tax'
        };
      }
      if (!('payment' in data)) {
        data.payment = {
          value: {
            payment_provider: 'stripe',
            payment_type: 'credit',
            last4: '1234',
            exp_month: 12,
            exp_year: 2022,
            brand: 'visa',
          },
          expected: {
            card: 'Card ending in 1234',
            expires: 'December 2022',
            brand: 'Visa'
          }
        };
      }
      if (!('subscription' in data)) {
        data.subscription = {
          value: {
            _subscription_type: 'web',
            created: 1,
            current_period_end: 2,
            cancel_at_period_end: true,
            status: 'active'
          },
          expected: {
            activated: '1/1/70',
            cancelled: '1/1/70',
            label: 'Expires',
            status: 'Active'
          }
        };
      }

      console.log(' -> Running config: ' + data.name);
      this.ctx.guardianSubscriptionDetailsCallback = req => {
        this.ctx.guardianOverrideEndpoints
            .GETs['/api/v1/vpn/subscriptionDetails']
            .status = 200;
        this.ctx.guardianOverrideEndpoints
            .GETs['/api/v1/vpn/subscriptionDetails']
            .body = {
          plan: data.plan.value,
          payment: data.payment.value,
          subscription: data.subscription.value,
        }
      };

      await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

      await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());
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
          queries.screenSettings.subscriptionView.SCREEN.visible());

      if (data.subscription.expected.status) {
        assert.equal(
            await vpn.getQueryProperty(
                queries.screenSettings.subscriptionView.STATUS_PILL, 'text'),
            data.subscription.expected.status);
      }

      if (data.plan.expected &&
          data.subscription.value._subscription_type == 'web') {
        await vpn.waitForQuery(
            queries.screenSettings.subscriptionView.PLAN.visible());
        assert.equal(
            await vpn.getQueryProperty(
                queries.screenSettings.subscriptionView.PLAN.visible(), 'text'),
            data.plan.expected);
      }

      if (data.subscription.expected.activated) {
        await vpn.waitForQuery(
            queries.screenSettings.subscriptionView.ACTIVATED.visible());
        assert.equal(
            await vpn.getQueryProperty(
                queries.screenSettings.subscriptionView.ACTIVATED.visible(),
                'text'),
            data.subscription.expected.activated);
      }

      await vpn.waitForQuery(
          queries.screenSettings.subscriptionView.CANCELLED.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenSettings.subscriptionView.CANCELLED.visible(),
              'text'),
          data.subscription.expected.cancelled);
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenSettings.subscriptionView.CANCELLED_LABEL.visible(),
              'text'),
          data.subscription.expected.label);

      if (data.subscription.value._subscription_type == 'web') {
        if (data.payment.expected.card) {
          await vpn.waitForQuery(
              queries.screenSettings.subscriptionView.BRAND.visible());
          assert.equal(
              await vpn.getQueryProperty(
                  queries.screenSettings.subscriptionView.BRAND.visible(),
                  'text'),
              data.payment.expected.card);
        }
        if (data.payment.expected.expires) {
          await vpn.waitForQuery(
              queries.screenSettings.subscriptionView.EXPIRES.visible());
          assert.equal(
              await vpn.getQueryProperty(
                  queries.screenSettings.subscriptionView.EXPIRES.visible(),
                  'text'),
              data.payment.expected.expires);
        }
        if (data.payment.expected.brand) {
          await vpn.waitForQuery(
              queries.screenSettings.subscriptionView.PAYMENT_METHOD.visible());
          assert.equal(
              await vpn.getQueryProperty(
                  queries.screenSettings.subscriptionView.PAYMENT_METHOD
                      .visible(),
                  'text'),
              data.payment.expected.brand);
        }
        if (data.payment.expected.payment) {
          await vpn.waitForQuery(queries.screenSettings.subscriptionView
                                     .PAYMENT_METHOD_LABEL.visible());
          assert.equal(
              await vpn.getQueryProperty(
                  queries.screenSettings.subscriptionView.PAYMENT_METHOD_LABEL
                      .visible(),
                  'text'),
              data.payment.expected.payment);
        }
      }

      await vpn.waitForQuery(queries.screenSettings.subscriptionView
                                 .SUBSCRIPTION_USER_PROFILE_BUTTON_ACCOUNT.visible());
      await vpn.waitForQuery(
          queries.screenSettings.subscriptionView
              .SUBSCRIPTION_USER_PROFILE_DISPLAY_NAME.visible()
              .prop('text', 'Test'));
      await vpn.waitForQuery(
          queries.screenSettings.subscriptionView
              .SUBSCRIPTION_USER_PROFILE_EMAIL_ADDRESS.visible()
              .prop('text', 'test@mozilla.com'));
      await vpn.waitForQueryAndClick(
          queries.screenSettings.subscriptionView
              .SUBSCRIPTION_USER_PROFILE_BUTTON_ACCOUNT.visible());

      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('https://accounts.stage.mozaws.net') &&
            url.includes('?email=test@mozilla.com');
      });

      if (data.manageSubscriptionLink) {
        await vpn.waitForQuery(
            queries.screenSettings.subscriptionView
                .SUBSCRIPTION_USER_PROFILE_BUTTON_SUB.visible());
        await vpn.scrollToQuery(
            queries.screenSettings.subscriptionView.FLICKABLE,
            queries.screenSettings.subscriptionView
                .SUBSCRIPTION_USER_PROFILE_BUTTON_SUB.visible());
        await vpn.waitForQueryAndClick(
            queries.screenSettings.subscriptionView
                .SUBSCRIPTION_USER_PROFILE_BUTTON_SUB.visible());
        await vpn.waitForCondition(async () => {
          const url = await vpn.getLastUrl();
          return url.includes(data.manageSubscriptionLink);
        });
      }

      await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

      await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());

      await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    }
  });

  it('Account deletion - simple', async () => {
    this.ctx.fxaLoginCallback = (req) => {
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
        sessionToken: 'session',
        verified: true,
        verificationMethod: ''
      };
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;
    };
    this.ctx.guardianSubscriptionDetailsCallback = req => {
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .status = 200;
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .body = SUBSCRIPTION_DETAILS;
    };

    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());
    await vpn.waitForQuery(
        queries.screenSettings.USER_PROFILE_DISPLAY_NAME.visible().prop(
            'text', 'Test'));
    await vpn.waitForQuery(
        queries.screenSettings.USER_PROFILE_EMAIL_ADDRESS.visible().prop(
            'text', 'test@mozilla.com'));
    await vpn.waitForQueryAndClick(
        queries.screenSettings.USER_PROFILE.visible());

    await vpn.waitForQuery(
        queries.screenSettings.subscriptionView.SCREEN.visible());

    await vpn.waitForQuery(
        queries.screenSettings.subscriptionView.ACCOUNT_DELETION.hidden());
    await vpn.flipFeatureOn('accountDeletion');
    await vpn.waitForQuery(
        queries.screenSettings.subscriptionView.ACCOUNT_DELETION.visible());

    await vpn.scrollToQuery(
        queries.screenSettings.subscriptionView.FLICKABLE,
        queries.screenSettings.subscriptionView.ACCOUNT_DELETION.visible());

    await vpn.waitForQueryAndClick(
        queries.screenSettings.subscriptionView.ACCOUNT_DELETION.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

    await vpn.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible());
    await vpn.setQueryProperty(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible(),
        'text', 'P4ssw0rd!!');

    await vpn.waitForQueryAndClick(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
            .enabled());

    await vpn.waitForQuery(
        queries.screenDeleteAccount.BUTTON.visible().disabled());
    await vpn.waitForQuery(queries.screenDeleteAccount.LABEL.visible());

    await vpn.setQueryProperty(
        queries.screenDeleteAccount.SCREEN, 'contentY',
        parseInt(await vpn.getQueryProperty(
            queries.screenDeleteAccount.CHECKBOX1, 'y')));
    await vpn.waitForQuery(queries.screenDeleteAccount.CHECKBOX1.visible().prop(
        'isChecked', false));
    await vpn.waitForQueryAndClick(
        queries.screenDeleteAccount.CHECKBOX1_CB.visible());
    await vpn.waitForQuery(queries.screenDeleteAccount.CHECKBOX1.visible().prop(
        'isChecked', true));
    await vpn.waitForQuery(
        queries.screenDeleteAccount.BUTTON.visible().disabled());

    await vpn.setQueryProperty(
        queries.screenDeleteAccount.SCREEN, 'contentY',
        parseInt(await vpn.getQueryProperty(
            queries.screenDeleteAccount.CHECKBOX2, 'y')));
    await vpn.waitForQuery(queries.screenDeleteAccount.CHECKBOX2.visible().prop(
        'isChecked', false));
    await vpn.waitForQueryAndClick(
        queries.screenDeleteAccount.CHECKBOX2_CB.visible());
    await vpn.waitForQuery(queries.screenDeleteAccount.CHECKBOX2.visible().prop(
        'isChecked', true));
    await vpn.waitForQuery(
        queries.screenDeleteAccount.BUTTON.visible().disabled());

    await vpn.setQueryProperty(
        queries.screenDeleteAccount.SCREEN, 'contentY',
        parseInt(await vpn.getQueryProperty(
            queries.screenDeleteAccount.CHECKBOX3, 'y')));
    await vpn.waitForQuery(queries.screenDeleteAccount.CHECKBOX3.visible().prop(
        'isChecked', false));
    await vpn.waitForQueryAndClick(
        queries.screenDeleteAccount.CHECKBOX3_CB.visible());
    await vpn.waitForQuery(queries.screenDeleteAccount.CHECKBOX3.visible().prop(
        'isChecked', true));
    await vpn.waitForQuery(
        queries.screenDeleteAccount.BUTTON.visible().disabled());

    await vpn.setQueryProperty(
        queries.screenDeleteAccount.SCREEN, 'contentY',
        parseInt(await vpn.getQueryProperty(
            queries.screenDeleteAccount.CHECKBOX3, 'y')));
    await vpn.waitForQuery(queries.screenDeleteAccount.CHECKBOX4.visible().prop(
        'isChecked', false));
    await vpn.waitForQueryAndClick(
        queries.screenDeleteAccount.CHECKBOX4_CB.visible());
    await vpn.waitForQuery(queries.screenDeleteAccount.CHECKBOX4.visible().prop(
        'isChecked', true));
    await vpn.waitForQuery(
        queries.screenDeleteAccount.BUTTON.visible().enabled());

    this.ctx.fxaDestroyCallback = (req) => {
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/destroy'].status = 200;
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/destroy'].body = {}
    };

    await vpn.wait();

    await vpn.waitForQueryAndClick(
        queries.screenDeleteAccount.BUTTON.visible().enabled());

    await vpn.waitForQuery(queries.screenInitialize.GET_HELP_LINK.visible());
    await vpn.waitForQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
    await vpn.waitForQuery(
        queries.screenInitialize.ALREADY_A_SUBSCRIBER_LINK.visible());
  });

  it('Account deletion - totp', async () => {
    this.ctx.fxaLoginCallback = (req) => {
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
        sessionToken: 'session',
        verified: false,
        verificationMethod: 'totp-2fa'
      };
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;
    };

    this.ctx.guardianSubscriptionDetailsCallback = req => {
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .status = 200;
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .body = SUBSCRIPTION_DETAILS;
    };

    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

    await vpn.waitForQuery(queries.screenSettings.USER_PROFILE.visible());
    await vpn.waitForQuery(
        queries.screenSettings.USER_PROFILE_DISPLAY_NAME.visible().prop(
            'text', 'Test'));
    await vpn.waitForQuery(
        queries.screenSettings.USER_PROFILE_EMAIL_ADDRESS.visible().prop(
            'text', 'test@mozilla.com'));
    await vpn.waitForQueryAndClick(
        queries.screenSettings.USER_PROFILE.visible());

    await vpn.waitForQuery(
        queries.screenSettings.subscriptionView.SCREEN.visible());

    await vpn.waitForQuery(
        queries.screenSettings.subscriptionView.ACCOUNT_DELETION.hidden());
    await vpn.flipFeatureOn('accountDeletion');
    await vpn.waitForQuery(
        queries.screenSettings.subscriptionView.ACCOUNT_DELETION.visible());

    await vpn.scrollToQuery(
        queries.screenSettings.subscriptionView.FLICKABLE,
        queries.screenSettings.subscriptionView.ACCOUNT_DELETION.visible());

    await vpn.waitForQueryAndClick(
        queries.screenSettings.subscriptionView.ACCOUNT_DELETION.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

    await vpn.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible());
    await vpn.setQueryProperty(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible(),
        'text', 'P4ssw0rd!!');

    await vpn.waitForQueryAndClick(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
            .enabled());

    await vpn.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible());
    await vpn.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
            .disabled());
    await vpn.setQueryProperty(
        queries.screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible(),
        'text', '123456');
    await vpn.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible().enabled());

    this.ctx.fxaTotpCallback = (req) => {
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify/totp'].body = {
        success: true
      }
    };

    await vpn.waitForQueryAndClick(
        queries.screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible().enabled());

    await vpn.waitForQuery(
        queries.screenDeleteAccount.BUTTON.visible().disabled());
    await vpn.waitForQuery(queries.screenDeleteAccount.LABEL.visible());

    await vpn.setQueryProperty(
        queries.screenDeleteAccount.SCREEN, 'contentY',
        parseInt(await vpn.getQueryProperty(
            queries.screenDeleteAccount.CHECKBOX1, 'y')));
    await vpn.waitForQuery(queries.screenDeleteAccount.CHECKBOX1.visible().prop(
        'isChecked', false));
    await vpn.waitForQueryAndClick(
        queries.screenDeleteAccount.CHECKBOX1_CB.visible());
    await vpn.waitForQuery(queries.screenDeleteAccount.CHECKBOX1.visible().prop(
        'isChecked', true));
    await vpn.waitForQuery(
        queries.screenDeleteAccount.BUTTON.visible().disabled());

    await vpn.setQueryProperty(
        queries.screenDeleteAccount.SCREEN, 'contentY',
        parseInt(await vpn.getQueryProperty(
            queries.screenDeleteAccount.CHECKBOX2, 'y')));
    await vpn.waitForQuery(queries.screenDeleteAccount.CHECKBOX2.visible().prop(
        'isChecked', false));
    await vpn.waitForQueryAndClick(
        queries.screenDeleteAccount.CHECKBOX2_CB.visible());
    await vpn.waitForQuery(queries.screenDeleteAccount.CHECKBOX2.visible().prop(
        'isChecked', true));
    await vpn.waitForQuery(
        queries.screenDeleteAccount.BUTTON.visible().disabled());

    await vpn.setQueryProperty(
        queries.screenDeleteAccount.SCREEN, 'contentY',
        parseInt(await vpn.getQueryProperty(
            queries.screenDeleteAccount.CHECKBOX3, 'y')));
    await vpn.waitForQuery(queries.screenDeleteAccount.CHECKBOX3.visible().prop(
        'isChecked', false));
    await vpn.waitForQueryAndClick(
        queries.screenDeleteAccount.CHECKBOX3_CB.visible());
    await vpn.waitForQuery(queries.screenDeleteAccount.CHECKBOX3.visible().prop(
        'isChecked', true));
    await vpn.waitForQuery(
        queries.screenDeleteAccount.BUTTON.visible().disabled());

    await vpn.setQueryProperty(
        queries.screenDeleteAccount.SCREEN, 'contentY',
        parseInt(await vpn.getQueryProperty(
            queries.screenDeleteAccount.CHECKBOX3, 'y')));
    await vpn.waitForQuery(queries.screenDeleteAccount.CHECKBOX4.visible().prop(
        'isChecked', false));
    await vpn.waitForQueryAndClick(
        queries.screenDeleteAccount.CHECKBOX4_CB.visible());
    await vpn.waitForQuery(queries.screenDeleteAccount.CHECKBOX4.visible().prop(
        'isChecked', true));
    await vpn.waitForQuery(
        queries.screenDeleteAccount.BUTTON.visible().enabled());

    this.ctx.fxaDestroyCallback = (req) => {
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/destroy'].status = 200;
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/destroy'].body = {}
    };

    await vpn.wait();

    await vpn.waitForQueryAndClick(
        queries.screenDeleteAccount.BUTTON.visible().enabled());

    await vpn.waitForQuery(queries.screenInitialize.GET_HELP_LINK.visible());
    await vpn.waitForQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
    await vpn.waitForQuery(
        queries.screenInitialize.ALREADY_A_SUBSCRIBER_LINK.visible());
  });

  async function openSubscriptionManagement() {
    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
    await vpn.waitForQueryAndClick(
        queries.screenSettings.USER_PROFILE.visible());
    await vpn.waitForQuery(queries.screenSettings.subscriptionView.SCREEN.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
  }

  it('Shows annual upgrade UI based on billing interval and purchase type',
     async () => {
       this.ctx.fxaLoginCallback = (req) => {
         this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
           sessionToken: 'session',
           verified: true,
           verificationMethod: '',
         };
         this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;
       };
       this.ctx.guardianSubscriptionDetailsCallback =
           req => {
             this.ctx.guardianOverrideEndpoints
                 .GETs['/api/v1/vpn/subscriptionDetails']
                 .status = 200;

             this.ctx.guardianOverrideEndpoints
                 .GETs['/api/v1/vpn/subscriptionDetails']
                 .body = SUBSCRIPTION_DETAILS_MONTHLY
           };

       if (!(await vpn.isFeatureFlippedOn('annualUpgrade'))) {
         await vpn.flipFeatureOn('annualUpgrade');
       }

       await openSubscriptionManagement();

       await vpn.waitForQuery(
           queries.screenSettings.subscriptionView.ANNUAL_UPGRADE.visible());
     });

  it('Hides upgrade UI based on billing interval and purchase type',
     async () => {
       this.ctx.fxaLoginCallback = (req) => {
         this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
           sessionToken: 'session',
           verified: true,
           verificationMethod: '',
         };
         this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;
       };
       this.ctx.guardianSubscriptionDetailsCallback = req => {
         this.ctx.guardianOverrideEndpoints
             .GETs['/api/v1/vpn/subscriptionDetails']
             .status = 200;

         this.ctx.guardianOverrideEndpoints
             .GETs['/api/v1/vpn/subscriptionDetails']
             .body = SUBSCRIPTION_DETAILS;
       };

       if (!(await vpn.isFeatureFlippedOn('annualUpgrade'))) {
         await vpn.flipFeatureOn('annualUpgrade');
       }

       await openSubscriptionManagement();

       await vpn.waitForQuery(
           queries.screenSettings.subscriptionView.ANNUAL_UPGRADE.hidden());
     });

  it('Hides annual upgrade UI when the "annualUpgrade" feature flag is off',
     async () => {
       if ((await vpn.isFeatureFlippedOn('annualUpgrade'))) {
         await vpn.flipFeatureOff('annualUpgrade');
       }

       this.ctx.fxaLoginCallback = (req) => {
         this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
           sessionToken: 'session',
           verified: true,
           verificationMethod: ''
         };
         this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;
       };
       this.ctx.guardianSubscriptionDetailsCallback = req => {
         this.ctx.guardianOverrideEndpoints
             .GETs['/api/v1/vpn/subscriptionDetails']
             .status = 200;
         this.ctx.guardianOverrideEndpoints
             .GETs['/api/v1/vpn/subscriptionDetails']
             .body = SUBSCRIPTION_DETAILS;
       };

       await openSubscriptionManagement();

       await vpn.waitForQuery(
           queries.screenSettings.subscriptionView.ANNUAL_UPGRADE.hidden());
     });

  describe('Subscription management telemetry tests', () => {
    // No Glean on WASM.
    if(vpn.runningOnWasm()) {
      return;
    }

    const testCases = [["annual plan", "account"], ["monthly plan", "account_with_change_plan"]];

    testCases.forEach(([testCase, subscriptionManagementScreenTelemetryId]) => {
      describe(`Subscription management view events are recorded (${testCase})`, () => {
        const isMonthlyPlanTest = testCase === "monthly plan"

        //Check if we are testing the monthly plan so we can mock the guardian response
        beforeEach(async () => {
          if (isMonthlyPlanTest) {
            if (!(await vpn.isFeatureFlippedOn('annualUpgrade'))) {
              await vpn.flipFeatureOn('annualUpgrade');
            }

            this.ctx.fxaLoginCallback = (req) => {
             this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
               sessionToken: 'session',
               verified: true,
               verificationMethod: '',
             };
             this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;
           };
           this.ctx.guardianSubscriptionDetailsCallback =
           req => {
             this.ctx.guardianOverrideEndpoints
             .GETs['/api/v1/vpn/subscriptionDetails']
             .status = 200;

             this.ctx.guardianOverrideEndpoints
             .GETs['/api/v1/vpn/subscriptionDetails']
             .body = SUBSCRIPTION_DETAILS_MONTHLY
           };
         }

         await openSubscriptionManagement();
       });

        it('accountScreen impression event is recorded', async () => {
          //Open the subscription management view and record accountScreen telemetry
          const accountScreenEvents = await vpn.gleanTestGetValue("impression", "accountScreen", "main");
          assert.equal(accountScreenEvents.length, 1);
          const accountScreenEventExtras = accountScreenEvents[0].extra;
          assert.equal(subscriptionManagementScreenTelemetryId, accountScreenEventExtras.screen);
        });

        it('editSelected interaction event is recorded', async () => {
          //Click the user profile button and record editSelected telemetry
          await vpn.waitForQueryAndClick(queries.screenSettings.subscriptionView.SUBSCRIPTION_USER_PROFILE_BUTTON_ACCOUNT.visible());
          const editSelectedEvents = await vpn.gleanTestGetValue("interaction", "editSelected", "main");
          assert.equal(editSelectedEvents.length, 1);
          const editSelectedEventsExtras = editSelectedEvents[0].extra;
          assert.equal(subscriptionManagementScreenTelemetryId, editSelectedEventsExtras.screen);
        });

        if (isMonthlyPlanTest) {
          it('changePlanSelected interaction event is recorded with correct screen', async () => {
            //Click the change plan button and record changePlanSelected telemetry
            await vpn.waitForQueryAndClick(queries.screenSettings.subscriptionView.ANNUAL_UPGRADE_BUTTON.visible());
            const changePlanSelectedEvents = await vpn.gleanTestGetValue("interaction", "changePlanSelected", "main");
            assert.equal(changePlanSelectedEvents.length, 1);
            const changePlanSelectedEventsExtras = changePlanSelectedEvents[0].extra;
            assert.equal(subscriptionManagementScreenTelemetryId, changePlanSelectedEventsExtras.screen);
          });
        }

        it('manageSubscriptionSelected interaction event is recorded', async () => {
          //Click the manage subscription button and record manageSubscriptionSelected telemetry
          await vpn.scrollToQuery(queries.screenSettings.subscriptionView.FLICKABLE, queries.screenSettings.subscriptionView.SUBSCRIPTION_USER_PROFILE_BUTTON_SUB.visible());
          await vpn.waitForQueryAndClick(queries.screenSettings.subscriptionView.SUBSCRIPTION_USER_PROFILE_BUTTON_SUB.visible());
          const manageSubscriptionSelectedEvents = await vpn.gleanTestGetValue("interaction", "manageSubscriptionSelected", "main");
          assert.equal(manageSubscriptionSelectedEvents.length, 1);
          const manageSubscriptionSelectedEventsExtras = manageSubscriptionSelectedEvents[0].extra;
          assert.equal(subscriptionManagementScreenTelemetryId, manageSubscriptionSelectedEventsExtras.screen);
        });
      });
    });
  });
});
