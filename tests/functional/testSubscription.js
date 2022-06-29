/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');

describe('Subscription view', function() {
  this.timeout(120000);
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

  beforeEach(async () => {
    if (!(await vpn.isFeatureFlippedOn('subscriptionManagement'))) {
      await vpn.flipFeatureOn('subscriptionManagement');
    }
  });

  it('Playing with the subscription view', async () => {
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
        payment: {
          value: {},
          expected: {payment: 'Apple subscription'}
        },
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
        payment: {
          value: {},
          expected: {payment: 'Google subscription'}
        },
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
            .body = {
          plan: data.plan.value,
          payment: data.payment.value,
          subscription: data.subscription.value,
        }
      };

      await vpn.waitForElement('settingsButton');
      await vpn.clickOnElement('settingsButton');

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

      await vpn.waitForElement(
          'subscriptionItem/subscriptionItem-plan/subscriptionItem-plan-parent/subscriptionItem-plan-container/subscriptionItem-plan-valueText');
      await vpn.waitForElementProperty(
          'subscriptionItem/subscriptionItem-plan/subscriptionItem-plan-parent/subscriptionItem-plan-container/subscriptionItem-plan-valueText',
          'visible', 'true');

      assert(
          await vpn.getElementProperty(
              'subscriptionItem/subscriptionItem-plan/subscriptionItem-plan-parent/subscriptionItem-plan-container/subscriptionItem-plan-valueText',
              'text') === data.plan.expected);
      assert(
          await vpn.getElementProperty(
              'subscriptionItem/subscriptionItem-status/subscriptionItem-status-parent/subscriptionItem-status-container/subscriptionItem-status-pillWrapper/subscriptionItem-status-pill',
              'text') === data.subscription.expected.status);

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
        return url.includes('r/vpn/account?email=test@mozilla.com');
      });

      if (data.manageSubscriptionLink) {
        await vpn.waitForElement('manageSubscriptionButton');
        await vpn.waitForElementProperty(
            'manageSubscriptionButton', 'visible', 'true');
        await vpn.setElementProperty(
            'subscriptionManagmentView', 'contentY', 'i',
            parseInt(
                await vpn.getElementProperty('manageSubscriptionButton', 'y')));
        await vpn.clickOnElement('manageSubscriptionButton');
        await vpn.waitForCondition(async () => {
          const url = await vpn.getLastUrl();
          return url.includes(data.manageSubscriptionLink);
        });
      }

      await vpn.waitForElement('settingsBackButton');
      await vpn.waitForElementProperty('settingsBackButton', 'visible', 'true');
      await vpn.clickOnElement('settingsBackButton');
      await vpn.wait();

      await vpn.waitForElement('settingsUserProfile');
      await vpn.waitForElementProperty(
          'settingsUserProfile', 'visible', 'true');

      await vpn.clickOnElement('settingsBackButton');
      await vpn.wait();

      await vpn.waitForElement('controllerTitle');
      await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
    }
  });
});
