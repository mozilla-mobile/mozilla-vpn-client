/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');

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
        status: 200,
        body: [],
      },
    },
    POSTs: {
      '/v1/account/login': {
        status: 200,
        body: null,
        callback: (req) => this.ctx.fxaLoginCallback(req)
      },
      '/v1/session/verify/totp': {
        status: 200,
        body: null,
        callback: (req) => this.ctx.fxaTotpCallback(req)
      },
      '/v1/account/destroy': {
        status: 200,
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

    await vpn.waitForElement('navigationLayout/navButton-settings');
    await vpn.clickOnElement('navigationLayout/navButton-settings');
    await vpn.waitForElementProperty('screenLoader', 'busy', 'false');

    await vpn.waitForElement('settingsUserProfile');
    await vpn.waitForElementProperty('settingsUserProfile', 'visible', 'true');

    await vpn.waitForElement('settingsUserProfile-displayName');
    await vpn.waitForElementProperty(
        'settingsUserProfile-displayName', 'visible', 'true');
    await vpn.waitForElementProperty(
        'settingsUserProfile-displayName', 'text', 'Test test');

    await vpn.waitForElement('settingsUserProfile-emailAddress');
    await vpn.waitForElementProperty(
        'settingsUserProfile-emailAddress', 'visible', 'true');
    await vpn.waitForElementProperty(
        'settingsUserProfile-emailAddress', 'text', 'test@mozilla.com');

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
        'settingsUserProfile-manageAccountButton', 'visible', 'true');
    await vpn.clickOnElement('settingsUserProfile-manageAccountButton');

    await vpn.waitForElementProperty('screenLoader', 'busy', 'false');

    await vpn.waitForElement('authSignIn-passwordInput');
    await vpn.waitForElementProperty(
        'authSignIn-passwordInput', 'visible', 'true');
    await vpn.setElementProperty(
        'authSignIn-passwordInput', 'text', 's', 'P4ass0rd!!');

    await vpn.waitForElementProperty('authSignIn-button', 'enabled', 'true');

    this.ctx.guardianSubscriptionDetailsCallback = req => {
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .status = 200;
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .body = SUBSCRIPTION_DETAILS;
    };

    await vpn.wait();
    await vpn.clickOnElement('authSignIn-button');

    await vpn.waitForElement('subscriptionManagmentView');
    await vpn.waitForElementProperty(
        'subscriptionManagmentView', 'visible', 'true');
    await vpn.wait();
  });

  it('Authentication needed - totp', async () => {
    this.ctx.guardianSubscriptionDetailsCallback = req => {
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .status = 401;
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .body = {}
    };

    await vpn.waitForElement('navigationLayout/navButton-settings');
    await vpn.clickOnElement('navigationLayout/navButton-settings');

    await vpn.waitForElement('settingsUserProfile');
    await vpn.waitForElementProperty('settingsUserProfile', 'visible', 'true');

    await vpn.waitForElement('settingsUserProfile-displayName');
    await vpn.waitForElementProperty(
        'settingsUserProfile-displayName', 'visible', 'true');
    await vpn.waitForElementProperty(
        'settingsUserProfile-displayName', 'text', 'Test test');

    await vpn.waitForElement('settingsUserProfile-emailAddress');
    await vpn.waitForElementProperty(
        'settingsUserProfile-emailAddress', 'visible', 'true');
    await vpn.waitForElementProperty(
        'settingsUserProfile-emailAddress', 'text', 'test@mozilla.com');

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
        'settingsUserProfile-manageAccountButton', 'visible', 'true');
    await vpn.clickOnElement('settingsUserProfile-manageAccountButton');

    await vpn.waitForElementProperty('screenLoader', 'busy', 'false');

    await vpn.waitForElement('authSignIn-passwordInput');
    await vpn.waitForElementProperty(
        'authSignIn-passwordInput', 'visible', 'true');
    await vpn.setElementProperty(
        'authSignIn-passwordInput', 'text', 's', 'P4ass0rd!!');

    await vpn.wait();
    await vpn.waitForElementProperty('authSignIn-button', 'enabled', 'true');

    this.ctx.fxaLoginCallback = (req) => {
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
        sessionToken: 'session',
        verified: false,
        verificationMethod: 'totp-2fa'
      };
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;
    };

    await vpn.wait();
    await vpn.clickOnElement('authSignIn-button');

    await vpn.waitForElement('authVerificationSessionByTotpNeeded-textInput');
    await vpn.waitForElementProperty(
        'authVerificationSessionByTotpNeeded-textInput', 'visible', 'true');
    await vpn.waitForElementProperty(
        'authVerificationSessionByTotpNeeded-button', 'enabled', 'false');
    await vpn.setElementProperty(
        'authVerificationSessionByTotpNeeded-textInput', 'text', 's', '123456');
    await vpn.waitForElementProperty(
        'authVerificationSessionByTotpNeeded-button', 'enabled', 'true');

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

    await vpn.wait();
    await vpn.clickOnElement('authVerificationSessionByTotpNeeded-button');

    await vpn.waitForElement('subscriptionManagmentView');
    await vpn.waitForElementProperty(
        'subscriptionManagmentView', 'visible', 'true');
    await vpn.wait();
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
          expected: '$1.23 Yearly'
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
          expected: '$1.23 Half-yearly'
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
          expected: '$1.23 Monthly'
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
          expected: '$1.23 Yearly'
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

      await vpn.waitForElement('navigationLayout/navButton-settings');
      await vpn.clickOnElement('navigationLayout/navButton-settings');

      await vpn.waitForElement('settingsUserProfile');
      await vpn.waitForElementProperty(
          'settingsUserProfile', 'visible', 'true');

      await vpn.waitForElement('settingsUserProfile-displayName');
      await vpn.waitForElementProperty(
          'settingsUserProfile-displayName', 'visible', 'true');
      await vpn.waitForElementProperty(
          'settingsUserProfile-displayName', 'text', 'Test test');

      await vpn.waitForElement('settingsUserProfile-emailAddress');
      await vpn.waitForElementProperty(
          'settingsUserProfile-emailAddress', 'visible', 'true');
      await vpn.waitForElementProperty(
          'settingsUserProfile-emailAddress', 'text', 'test@mozilla.com');

      await vpn.waitForElement('settingsUserProfile-manageAccountButton');
      await vpn.waitForElementProperty(
          'settingsUserProfile-manageAccountButton', 'visible', 'true');
      await vpn.clickOnElement('settingsUserProfile-manageAccountButton');

      await vpn.waitForElement('subscriptionManagmentView');
      await vpn.waitForElementProperty(
          'subscriptionManagmentView', 'visible', 'true');
      await vpn.wait();

      if (data.subscription.expected.status) {
        assert(
          await vpn.getElementProperty(
              'subscriptionItem/subscriptionItem-status/subscriptionItem-status-parent/subscriptionItem-status-container/subscriptionItem-status-pillWrapper/subscriptionItem-status-pill',
              'text') === data.subscription.expected.status);
      }

      if (data.plan.expected &&
          data.subscription.value._subscription_type == "web") {
        await vpn.waitForElement(
            'subscriptionItem/subscriptionItem-plan/subscriptionItem-plan-parent/subscriptionItem-plan-container/subscriptionItem-plan-valueText');
        await vpn.waitForElementProperty(
            'subscriptionItem/subscriptionItem-plan/subscriptionItem-plan-parent/subscriptionItem-plan-container/subscriptionItem-plan-valueText',
            'visible', 'true');
        assert(
          await vpn.getElementProperty(
              'subscriptionItem/subscriptionItem-plan/subscriptionItem-plan-parent/subscriptionItem-plan-container/subscriptionItem-plan-valueText',
              'text') === data.plan.expected);
      }

      if (data.subscription.expected.activated) {
        await vpn.waitForElement(
            'subscriptionItem/subscriptionItem-activated/subscriptionItem-activated-parent/subscriptionItem-activated-container/subscriptionItem-activated-valueText');
        assert(
            await vpn.getElementProperty(
                'subscriptionItem/subscriptionItem-activated/subscriptionItem-activated-parent/subscriptionItem-activated-container/subscriptionItem-activated-valueText',
                'text') === data.subscription.expected.activated);
      }

      await vpn.waitForElement(
          'subscriptionItem/subscriptionItem-cancelled/subscriptionItem-cancelled-parent/subscriptionItem-cancelled-container/subscriptionItem-cancelled-valueText');
      assert(
          await vpn.getElementProperty(
              'subscriptionItem/subscriptionItem-cancelled/subscriptionItem-cancelled-parent/subscriptionItem-cancelled-container/subscriptionItem-cancelled-valueText',
              'text') === data.subscription.expected.cancelled);
      assert(
          await vpn.getElementProperty(
              'subscriptionItem/subscriptionItem-cancelled/subscriptionItem-cancelled-parent/subscriptionItem-cancelled-container/subscriptionItem-cancelled-labelText',
              'text') === data.subscription.expected.label);

      if (data.subscription.value._subscription_type == "web") {
        if (data.payment.expected.card) {
          await vpn.waitForElement(
              'subscriptionItem/subscriptionItem-brand/subscriptionItem-brand-parent/subscriptionItem-brand-container/subscriptionItem-brand-valueText');
          assert(
              await vpn.getElementProperty(
                  'subscriptionItem/subscriptionItem-brand/subscriptionItem-brand-parent/subscriptionItem-brand-container/subscriptionItem-brand-valueText',
                  'text') === data.payment.expected.card);
        }
        if (data.payment.expected.expires) {
          await vpn.waitForElement(
              'subscriptionItem/subscriptionItem-expires/subscriptionItem-expires-parent/subscriptionItem-expires-container/subscriptionItem-expires-valueText');
          assert(
              await vpn.getElementProperty(
                  'subscriptionItem/subscriptionItem-expires/subscriptionItem-expires-parent/subscriptionItem-expires-container/subscriptionItem-expires-valueText',
                  'text') === data.payment.expected.expires);
        }
        if (data.payment.expected.brand) {
          await vpn.waitForElement(
              'subscriptionItem/subscriptionItem-brand/subscriptionItem-brand-parent/subscriptionItem-brand-container/subscriptionItem-brand-paymentMethod/paymentLabel');
          assert(
              await vpn.getElementProperty(
                  'subscriptionItem/subscriptionItem-brand/subscriptionItem-brand-parent/subscriptionItem-brand-container/subscriptionItem-brand-paymentMethod/paymentLabel',
                  'text') === data.payment.expected.brand);
        }
        if (data.payment.expected.payment) {
          await vpn.waitForElement(
              'subscriptionItem/subscriptionItem-payment/subscriptionItem-payment-parent/subscriptionItem-payment-container/subscriptionItem-payment-paymentMethod/paymentLabel');
          assert(
              await vpn.getElementProperty(
                  'subscriptionItem/subscriptionItem-payment/subscriptionItem-payment-parent/subscriptionItem-payment-container/subscriptionItem-payment-paymentMethod/paymentLabel',
                  'text') === data.payment.expected.payment);
        }
      }

      await vpn.waitForElement('subscriptionUserProfile');
      await vpn.waitForElementProperty(
          'subscriptionUserProfile', 'visible', 'true');

      await vpn.waitForElement('subscriptionUserProfile-displayName');
      await vpn.waitForElementProperty(
          'subscriptionUserProfile-displayName', 'visible', 'true');
      await vpn.waitForElementProperty(
          'subscriptionUserProfile-displayName', 'text', 'Test test');

      await vpn.waitForElement('subscriptionUserProfile-emailAddress');
      await vpn.waitForElementProperty(
          'subscriptionUserProfile-emailAddress', 'visible', 'true');
      await vpn.waitForElementProperty(
          'subscriptionUserProfile-emailAddress', 'text', 'test@mozilla.com');

      await vpn.waitForElement('subscriptionUserProfile-manageAccountButton');
      await vpn.waitForElementProperty(
          'subscriptionUserProfile-manageAccountButton', 'visible', 'true');
      await vpn.clickOnElement('subscriptionUserProfile-manageAccountButton');
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('https://accounts.stage.mozaws.net') &&
            url.includes('?email=test@mozilla.com');
      });

      if (data.manageSubscriptionLink) {
        await vpn.waitForElement('manageSubscriptionButton');
        await vpn.waitForElementProperty(
            'manageSubscriptionButton', 'visible', 'true');
        await vpn.setElementProperty(
            'subscriptionManagmentView-flickable', 'contentY', 'i',
            parseInt(
                await vpn.getElementProperty('manageSubscriptionButton', 'y')));
        await vpn.clickOnElement('manageSubscriptionButton');
        await vpn.waitForCondition(async () => {
          const url = await vpn.getLastUrl();
          return url.includes(data.manageSubscriptionLink);
        });
      }

      await vpn.waitForElement('settings-back');
      await vpn.waitForElementProperty('settings-back', 'visible', 'true');
      await vpn.clickOnElement('settings-back');
      await vpn.wait();

      await vpn.waitForElement('settingsUserProfile');
      await vpn.waitForElementProperty(
          'settingsUserProfile', 'visible', 'true');

      await vpn.waitForElement('navigationLayout/navButton-home');
      await vpn.clickOnElement('navigationLayout/navButton-home');
      await vpn.wait();

      await vpn.waitForElement('controllerTitle');
      await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
      await vpn.wait();
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

    await vpn.waitForElement('navigationLayout/navButton-settings');
    await vpn.clickOnElement('navigationLayout/navButton-settings');

    await vpn.waitForElement('settingsUserProfile');
    await vpn.waitForElementProperty('settingsUserProfile', 'visible', 'true');

    await vpn.waitForElement('settingsUserProfile-displayName');
    await vpn.waitForElementProperty(
        'settingsUserProfile-displayName', 'visible', 'true');
    await vpn.waitForElementProperty(
        'settingsUserProfile-displayName', 'text', 'Test test');

    await vpn.waitForElement('settingsUserProfile-emailAddress');
    await vpn.waitForElementProperty(
        'settingsUserProfile-emailAddress', 'visible', 'true');
    await vpn.waitForElementProperty(
        'settingsUserProfile-emailAddress', 'text', 'test@mozilla.com');

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
        'settingsUserProfile-manageAccountButton', 'visible', 'true');
    await vpn.clickOnElement('settingsUserProfile-manageAccountButton');

    await vpn.waitForElement('subscriptionManagmentView');
    await vpn.waitForElementProperty(
        'subscriptionManagmentView', 'visible', 'true');

    await vpn.waitForElement('accountDeletionButton');
    await vpn.waitForElementProperty(
        'accountDeletionButton', 'visible', 'false');
    await vpn.wait();

    await vpn.flipFeatureOn('accountDeletion');
    await vpn.waitForElementProperty(
        'accountDeletionButton', 'visible', 'true');
    await vpn.wait();

    await vpn.setElementProperty(
        'subscriptionManagmentView-flickable', 'contentY', 'i',
        parseInt(await vpn.getElementProperty('accountDeletionButton', 'y')));
    await vpn.wait();

    await vpn.clickOnElement('accountDeletionButton');

    await vpn.waitForElementProperty('screenLoader', 'busy', 'false');

    await vpn.waitForElement('authSignIn-passwordInput');
    await vpn.waitForElementProperty(
        'authSignIn-passwordInput', 'visible', 'true');
    await vpn.setElementProperty(
        'authSignIn-passwordInput', 'text', 's', 'P4ass0rd!!');

    await vpn.wait();
    await vpn.waitForElementProperty('authSignIn-button', 'enabled', 'true');
    await vpn.clickOnElement('authSignIn-button');

    await vpn.waitForElement('deleteAccountForRealButton');
    await vpn.waitForElementProperty(
        'deleteAccountForRealButton', 'visible', 'true');
    await vpn.waitForElementProperty(
        'deleteAccountForRealButton', 'enabled', 'false');

    await vpn.waitForElement('accountDeletionLabel');
    await vpn.waitForElementProperty('accountDeletionLabel', 'visible', 'true');

    await vpn.setElementProperty(
        'viewDeleteAccountRequest', 'contentY', 'i',
        parseInt(await vpn.getElementProperty(
            'accountDeletionLayout/accountDeletionCheckbox-check1', 'y')));

    await vpn.waitForElement(
        'accountDeletionLayout/accountDeletionCheckbox-check1');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check1', 'visible',
        'true');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check1', 'isChecked',
        'false');
    await vpn.waitForElement(
        'accountDeletionLayout/accountDeletionCheckbox-check1/checkbox');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check1/checkbox',
        'visible', 'true');
    await vpn.clickOnElement(
        'accountDeletionLayout/accountDeletionCheckbox-check1/checkbox');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check1', 'isChecked',
        'true');
    await vpn.waitForElementProperty(
        'deleteAccountForRealButton', 'enabled', 'false');

    await vpn.setElementProperty(
        'viewDeleteAccountRequest', 'contentY', 'i',
        parseInt(await vpn.getElementProperty(
            'accountDeletionLayout/accountDeletionCheckbox-check2', 'y')));

    await vpn.waitForElement(
        'accountDeletionLayout/accountDeletionCheckbox-check2');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check2', 'visible',
        'true');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check2', 'isChecked',
        'false');
    await vpn.waitForElement(
        'accountDeletionLayout/accountDeletionCheckbox-check2/checkbox');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check2/checkbox',
        'visible', 'true');
    await vpn.clickOnElement(
        'accountDeletionLayout/accountDeletionCheckbox-check2/checkbox');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check2', 'isChecked',
        'true');
    await vpn.waitForElementProperty(
        'deleteAccountForRealButton', 'enabled', 'false');

    await vpn.setElementProperty(
        'viewDeleteAccountRequest', 'contentY', 'i',
        parseInt(await vpn.getElementProperty(
            'accountDeletionLayout/accountDeletionCheckbox-check3', 'y')));

    await vpn.waitForElement(
        'accountDeletionLayout/accountDeletionCheckbox-check3');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check3', 'visible',
        'true');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check3', 'isChecked',
        'false');
    await vpn.waitForElement(
        'accountDeletionLayout/accountDeletionCheckbox-check3/checkbox');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check3/checkbox',
        'visible', 'true');
    await vpn.clickOnElement(
        'accountDeletionLayout/accountDeletionCheckbox-check3/checkbox');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check3', 'isChecked',
        'true');
    await vpn.waitForElementProperty(
        'deleteAccountForRealButton', 'enabled', 'false');

    await vpn.setElementProperty(
        'viewDeleteAccountRequest', 'contentY', 'i',
        parseInt(await vpn.getElementProperty(
            'accountDeletionLayout/accountDeletionCheckbox-check4', 'y')));

    await vpn.waitForElement(
        'accountDeletionLayout/accountDeletionCheckbox-check4');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check4', 'visible',
        'true');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check4', 'isChecked',
        'false');
    await vpn.waitForElement(
        'accountDeletionLayout/accountDeletionCheckbox-check4/checkbox');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check4/checkbox',
        'visible', 'true');
    await vpn.clickOnElement(
        'accountDeletionLayout/accountDeletionCheckbox-check4/checkbox');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check4', 'isChecked',
        'true');
    await vpn.waitForElementProperty(
        'deleteAccountForRealButton', 'enabled', 'true');

    this.ctx.fxaDestroyCallback = (req) => {
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/destroy'].status = 200;
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/destroy'].body = {}
    };

    await vpn.clickOnElement('deleteAccountForRealButton');

    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
    assert(await vpn.getElementProperty('getStarted', 'visible') === 'true');
    assert(await vpn.getElementProperty('learnMoreLink', 'visible') === 'true');
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

    await vpn.waitForElement('navigationLayout/navButton-settings');
    await vpn.clickOnElement('navigationLayout/navButton-settings');

    await vpn.waitForElement('settingsUserProfile');
    await vpn.waitForElementProperty('settingsUserProfile', 'visible', 'true');

    await vpn.waitForElement('settingsUserProfile-displayName');
    await vpn.waitForElementProperty(
        'settingsUserProfile-displayName', 'visible', 'true');
    await vpn.waitForElementProperty(
        'settingsUserProfile-displayName', 'text', 'Test test');

    await vpn.waitForElement('settingsUserProfile-emailAddress');
    await vpn.waitForElementProperty(
        'settingsUserProfile-emailAddress', 'visible', 'true');
    await vpn.waitForElementProperty(
        'settingsUserProfile-emailAddress', 'text', 'test@mozilla.com');

    await vpn.waitForElement('settingsUserProfile-manageAccountButton');
    await vpn.waitForElementProperty(
        'settingsUserProfile-manageAccountButton', 'visible', 'true');
    await vpn.clickOnElement('settingsUserProfile-manageAccountButton');

    await vpn.waitForElement('subscriptionManagmentView');
    await vpn.waitForElementProperty(
        'subscriptionManagmentView', 'visible', 'true');

    await vpn.waitForElement('accountDeletionButton');
    await vpn.waitForElementProperty(
        'accountDeletionButton', 'visible', 'false');
    await vpn.wait();

    await vpn.flipFeatureOn('accountDeletion');
    await vpn.waitForElementProperty(
        'accountDeletionButton', 'visible', 'true');
    await vpn.wait();

    await vpn.setElementProperty(
        'subscriptionManagmentView-flickable', 'contentY', 'i',
        parseInt(await vpn.getElementProperty('accountDeletionButton', 'y')));
    await vpn.wait();

    await vpn.clickOnElement('accountDeletionButton');

    await vpn.waitForElementProperty('screenLoader', 'busy', 'false');

    await vpn.waitForElement('authSignIn-passwordInput');
    await vpn.waitForElementProperty(
        'authSignIn-passwordInput', 'visible', 'true');
    await vpn.setElementProperty(
        'authSignIn-passwordInput', 'text', 's', 'P4ass0rd!!');

    await vpn.wait();
    await vpn.waitForElementProperty('authSignIn-button', 'enabled', 'true');
    await vpn.clickOnElement('authSignIn-button');

    await vpn.waitForElement('authVerificationSessionByTotpNeeded-textInput');
    await vpn.waitForElementProperty(
        'authVerificationSessionByTotpNeeded-textInput', 'visible', 'true');
    await vpn.waitForElementProperty(
        'authVerificationSessionByTotpNeeded-button', 'enabled', 'false');
    await vpn.setElementProperty(
        'authVerificationSessionByTotpNeeded-textInput', 'text', 's', '123456');
    await vpn.waitForElementProperty(
        'authVerificationSessionByTotpNeeded-button', 'enabled', 'true');

    this.ctx.fxaTotpCallback = (req) => {
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify/totp'].body = {
        success: true
      }
    };

    await vpn.wait();
    await vpn.clickOnElement('authVerificationSessionByTotpNeeded-button');

    await vpn.waitForElement('deleteAccountForRealButton');
    await vpn.waitForElementProperty(
        'deleteAccountForRealButton', 'visible', 'true');
    await vpn.waitForElementProperty(
        'deleteAccountForRealButton', 'enabled', 'false');

    await vpn.waitForElement('accountDeletionLabel');
    await vpn.waitForElementProperty('accountDeletionLabel', 'visible', 'true');

    await vpn.setElementProperty(
        'viewDeleteAccountRequest', 'contentY', 'i',
        parseInt(await vpn.getElementProperty(
            'accountDeletionLayout/accountDeletionCheckbox-check1', 'y')));

    await vpn.waitForElement(
        'accountDeletionLayout/accountDeletionCheckbox-check1');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check1', 'visible',
        'true');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check1', 'isChecked',
        'false');
    await vpn.waitForElement(
        'accountDeletionLayout/accountDeletionCheckbox-check1/checkbox');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check1/checkbox',
        'visible', 'true');
    await vpn.clickOnElement(
        'accountDeletionLayout/accountDeletionCheckbox-check1/checkbox');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check1', 'isChecked',
        'true');
    await vpn.waitForElementProperty(
        'deleteAccountForRealButton', 'enabled', 'false');

    await vpn.setElementProperty(
        'viewDeleteAccountRequest', 'contentY', 'i',
        parseInt(await vpn.getElementProperty(
            'accountDeletionLayout/accountDeletionCheckbox-check2', 'y')));

    await vpn.waitForElement(
        'accountDeletionLayout/accountDeletionCheckbox-check2');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check2', 'visible',
        'true');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check2', 'isChecked',
        'false');
    await vpn.waitForElement(
        'accountDeletionLayout/accountDeletionCheckbox-check2/checkbox');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check2/checkbox',
        'visible', 'true');
    await vpn.clickOnElement(
        'accountDeletionLayout/accountDeletionCheckbox-check2/checkbox');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check2', 'isChecked',
        'true');
    await vpn.waitForElementProperty(
        'deleteAccountForRealButton', 'enabled', 'false');

    await vpn.setElementProperty(
        'viewDeleteAccountRequest', 'contentY', 'i',
        parseInt(await vpn.getElementProperty(
            'accountDeletionLayout/accountDeletionCheckbox-check3', 'y')));

    await vpn.waitForElement(
        'accountDeletionLayout/accountDeletionCheckbox-check3');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check3', 'visible',
        'true');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check3', 'isChecked',
        'false');
    await vpn.waitForElement(
        'accountDeletionLayout/accountDeletionCheckbox-check3/checkbox');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check3/checkbox',
        'visible', 'true');
    await vpn.clickOnElement(
        'accountDeletionLayout/accountDeletionCheckbox-check3/checkbox');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check3', 'isChecked',
        'true');
    await vpn.waitForElementProperty(
        'deleteAccountForRealButton', 'enabled', 'false');

    await vpn.setElementProperty(
        'viewDeleteAccountRequest', 'contentY', 'i',
        parseInt(await vpn.getElementProperty(
            'accountDeletionLayout/accountDeletionCheckbox-check4', 'y')));

    await vpn.waitForElement(
        'accountDeletionLayout/accountDeletionCheckbox-check4');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check4', 'visible',
        'true');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check4', 'isChecked',
        'false');
    await vpn.waitForElement(
        'accountDeletionLayout/accountDeletionCheckbox-check4/checkbox');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check4/checkbox',
        'visible', 'true');
    await vpn.clickOnElement(
        'accountDeletionLayout/accountDeletionCheckbox-check4/checkbox');
    await vpn.waitForElementProperty(
        'accountDeletionLayout/accountDeletionCheckbox-check4', 'isChecked',
        'true');
    await vpn.waitForElementProperty(
        'deleteAccountForRealButton', 'enabled', 'true');

    this.ctx.fxaDestroyCallback = (req) => {
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/destroy'].status = 200;
      this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/destroy'].body = {}
    };

    await vpn.clickOnElement('deleteAccountForRealButton');

    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
    assert(await vpn.getElementProperty('getStarted', 'visible') === 'true');
    assert(await vpn.getElementProperty('learnMoreLink', 'visible') === 'true');
  });

  const relayUpsell =
      'subscriptionItem/subscriptionItem-plan/subscriptionItem-plan-parent/subscriptionItem-plan-relayUpsell-layout';

  async function clickSettingsIcon() {
    await vpn.waitForElement('navigationLayout/navButton-settings');
    await vpn.clickOnElement('navigationLayout/navButton-settings');
  }
  async function openSubscriptionManagement() {
    await vpn.waitForElement('settingsUserProfile');
    await vpn.clickOnElement('settingsUserProfile');
  }

  it('Correctly shows or hides the VPN / Relay upgrade UI based on locale',
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
         this.ctx.guardianOverrideEndpoints
             .GETs['/api/v1/vpn/subscriptionDetails']
             .body = SUBSCRIPTION_DETAILS;
       };

       if (!(await vpn.isFeatureFlippedOn('bundleUpgrade'))) {
         await vpn.flipFeatureOn('bundleUpgrade');
       }

       await clickSettingsIcon();
       await openSubscriptionManagement();

       await vpn.waitForElement(relayUpsell);
       await vpn.waitForElementProperty(relayUpsell, 'visible', 'true');


       await clickSettingsIcon();

       await vpn.setSetting('language-code', 'de');
       await openSubscriptionManagement();

       await vpn.waitForElement(relayUpsell);
       await vpn.waitForElementProperty(relayUpsell, 'visible', 'false');
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

    await vpn.waitForElement(relayUpsell);
    await vpn.waitForElementProperty(relayUpsell, 'visible', 'false');
  });


  it('Hides Relay upsell if  subscription type is not "web"', async () => {
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

    await vpn.waitForElement(
        'subscriptionItem/subscriptionItem-status/subscriptionItem-status-parent/subscriptionItem-status-relayUpsell-layout');
    assert(
        await vpn.getElementProperty(
            'subscriptionItem/subscriptionItem-status/subscriptionItem-status-parent/subscriptionItem-status-relayUpsell-layout',
            'visible') === 'false');
  });

  // TODO:
  // Upgrade button works
  // Learn more link works
  
});
