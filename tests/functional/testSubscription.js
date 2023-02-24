/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');
const queries = require('./queries.js');
const fxaEndpoints = require('./servers/fxa_endpoints.js')

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
        queries.screenSettings.SUBSCRIPTION_MANAGMENT_VIEW.visible());
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
        queries.screenSettings.SUBSCRIPTION_MANAGMENT_VIEW.visible());
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
          queries.screenSettings.SUBSCRIPTION_MANAGMENT_VIEW.visible());

      if (data.subscription.expected.status) {
        assert(
            await vpn.getQueryProperty(
                queries.screenSettings.subscriptionView.STATUS_PILL, 'text') ===
            data.subscription.expected.status);
      }

      if (data.plan.expected &&
          data.subscription.value._subscription_type == "web") {
        await vpn.waitForQuery(
            queries.screenSettings.subscriptionView.PLAN.visible());
        assert(
            await vpn.getQueryProperty(
                queries.screenSettings.subscriptionView.PLAN.visible(),
                'text') === data.plan.expected);
      }

      if (data.subscription.expected.activated) {
        await vpn.waitForQuery(
            queries.screenSettings.subscriptionView.ACTIVATED.visible());
        assert(
            await vpn.getQueryProperty(
                queries.screenSettings.subscriptionView.ACTIVATED.visible(),
                'text') === data.subscription.expected.activated);
      }

      await vpn.waitForQuery(
          queries.screenSettings.subscriptionView.CANCELLED.visible());
      assert(
          await vpn.getQueryProperty(
              queries.screenSettings.subscriptionView.CANCELLED.visible(),
              'text') === data.subscription.expected.cancelled);
      assert(
          await vpn.getQueryProperty(
              queries.screenSettings.subscriptionView.CANCELLED_LABEL.visible(),
              'text') === data.subscription.expected.label);

      if (data.subscription.value._subscription_type == "web") {
        if (data.payment.expected.card) {
          await vpn.waitForQuery(
              queries.screenSettings.subscriptionView.BRAND.visible());
          assert(
              await vpn.getQueryProperty(
                  queries.screenSettings.subscriptionView.BRAND.visible(),
                  'text') === data.payment.expected.card);
        }
        if (data.payment.expected.expires) {
          await vpn.waitForQuery(
              queries.screenSettings.subscriptionView.EXPIRES.visible());
          assert(
              await vpn.getQueryProperty(
                  queries.screenSettings.subscriptionView.EXPIRES.visible(),
                  'text') === data.payment.expected.expires);
        }
        if (data.payment.expected.brand) {
          await vpn.waitForQuery(
              queries.screenSettings.subscriptionView.PAYMENT_METHOD.visible());
          assert(
              await vpn.getQueryProperty(
                  queries.screenSettings.subscriptionView.PAYMENT_METHOD
                      .visible(),
                  'text') === data.payment.expected.brand);
        }
        if (data.payment.expected.payment) {
          await vpn.waitForQuery(queries.screenSettings.subscriptionView
                                     .PAYMENT_METHOD_LABEL.visible());
          assert(
              await vpn.getQueryProperty(
                  queries.screenSettings.subscriptionView.PAYMENT_METHOD_LABEL
                      .visible(),
                  'text') === data.payment.expected.payment);
        }
      }

      await vpn.waitForQuery(queries.screenSettings.subscriptionView
                                 .SUBSCRIPTION_USER_PROFILE.visible());
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
            queries.screenSettings.subscriptionView.SCREEN,
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
        queries.screenSettings.SUBSCRIPTION_MANAGMENT_VIEW.visible());

    await vpn.waitForQuery(
        queries.screenSettings.subscriptionView.ACCOUNT_DELETION.hidden());
    await vpn.flipFeatureOn('accountDeletion');
    await vpn.waitForQuery(
        queries.screenSettings.subscriptionView.ACCOUNT_DELETION.visible());

    await vpn.scrollToQuery(
        queries.screenSettings.subscriptionView.SCREEN,
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
        queries.screenSettings.SUBSCRIPTION_MANAGMENT_VIEW.visible());

    await vpn.waitForQuery(
        queries.screenSettings.subscriptionView.ACCOUNT_DELETION.hidden());
    await vpn.flipFeatureOn('accountDeletion');
    await vpn.waitForQuery(
        queries.screenSettings.subscriptionView.ACCOUNT_DELETION.visible());

    await vpn.scrollToQuery(
        queries.screenSettings.subscriptionView.SCREEN,
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

  async function clickSettingsIcon() {
    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
  }
  async function openSubscriptionManagement() {
    await vpn.waitForQueryAndClick(
        queries.screenSettings.USER_PROFILE.visible());
  }

  it('Correctly shows the VPN / Relay upgrade UI based on currency',
    async () => {
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

        const SUBSCRIPTION_DETAILS_CLONE = Object.assign({},
            SUBSCRIPTION_DETAILS);
        SUBSCRIPTION_DETAILS_CLONE.plan = { currency: 'cad' };
        this.ctx.guardianOverrideEndpoints
            .GETs['/api/v1/vpn/subscriptionDetails']
            .body = SUBSCRIPTION_DETAILS;
      };

      if (!(await vpn.isFeatureFlippedOn('bundleUpgrade'))) {
        await vpn.flipFeatureOn('bundleUpgrade');
      }

      await clickSettingsIcon();
      await openSubscriptionManagement();

      await vpn.waitForQuery(
          queries.screenSettings.subscriptionView.RELAY_UPSELL_PLAN.visible());
    });

  it(
      'Correctly hides the VPN / Relay upgrade UI based on currency',
      async () => {
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

          const SUBSCRIPTION_DETAILS_CLONE =
              Object.assign({}, SUBSCRIPTION_DETAILS);
          SUBSCRIPTION_DETAILS_CLONE.plan = {currency: 'eur'};
          this.ctx.guardianOverrideEndpoints
              .GETs['/api/v1/vpn/subscriptionDetails']
              .body = SUBSCRIPTION_DETAILS;
        };

        if (!(await vpn.isFeatureFlippedOn('bundleUpgrade'))) {
          await vpn.flipFeatureOn('bundleUpgrade');
        }

        await clickSettingsIcon();
        await openSubscriptionManagement();

        await vpn.waitForQuery(queries.screenSettings.subscriptionView
                                   .RELAY_UPSELL_STATUS.hidden());
      });

  it('Hides Relay upsell if bundleUpgrade feature is not enabled', async () => {
    await vpn.flipFeatureOff('bundleUpgrade');

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

    await clickSettingsIcon();
    await openSubscriptionManagement();

    await vpn.waitForQuery(
        queries.screenSettings.subscriptionView.RELAY_UPSELL_PLAN.hidden());
  });


  it('Hides Relay upsell if subscription type is not "web"', async () => {
    await vpn.flipFeatureOn('bundleUpgrade');

    const SUBSCRIPTION_DETAILS_CLONE = Object.assign({}, SUBSCRIPTION_DETAILS);
    SUBSCRIPTION_DETAILS_CLONE.subscription = {
      _subscription_type: 'iap_apple',
      expiry_time_millis: 2147483647,
      auto_renewing: true,
    };

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
          .body = SUBSCRIPTION_DETAILS_CLONE;
    };

    await clickSettingsIcon();
    await openSubscriptionManagement();

    await vpn.waitForQuery(
        queries.screenSettings.subscriptionView.RELAY_UPSELL_STATUS.hidden());
  });

  // TODO:
  // Upgrade button works
  // Learn more link works
  
});
