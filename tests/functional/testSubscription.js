/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
const assert = require('assert');
const vpn = require('./helper.js');

describe('Subscription view', function() {
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

  async function getToProfileView() {
    await vpn.waitForElement('settingsButton');
    await vpn.clickOnElement('settingsButton');

    await vpn.waitForElement('settingsUserProfile');
    await vpn.waitForElementProperty('settingsUserProfile', 'visible', 'true');

    await vpn.waitForElement('profileDisplayName');
    await vpn.waitForElementProperty('profileDisplayName', 'visible', 'true');
    await vpn.waitForElementProperty('profileDisplayName', 'text', 'Test test');

    await vpn.waitForElement('profileEmailAddress');
    await vpn.waitForElementProperty('profileEmailAddress', 'visible', 'true');
    await vpn.waitForElementProperty(
        'profileEmailAddress', 'text', 'test@mozilla.com');

    await vpn.waitForElement('manageAccountButton');
    await vpn.waitForElementProperty('manageAccountButton', 'visible', 'true');
    await vpn.clickOnElement('manageAccountButton');

    await vpn.waitForElement('subscriptionManagmentView');
    await vpn.waitForElementProperty(
        'subscriptionManagmentView', 'visible', 'true');
    await vpn.wait();
  }

  // it('opening the subscription view (iap apple)', async () => {
  //   this.ctx.guardianSubscriptionDetailsCallback = req => {
  //     this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
  //         .body = {
  //       plan: {amount: 123, currency: 'foo', interval_count: 12},
  //       payment: {
  //         payment_provider: 'iap_apple',
  //         payment_type: 'credit',
  //         last4: '0016',
  //         exp_month: 12,
  //         exp_year: 2022,
  //         brand: 'visa',
  //       },
  //       subscription: {
  //         _subscription_type: 'iap_apple',
  //         auto_renewing: true,
  //         expiry_time_millis: 946681200000,
  //       }
  //     }
  //   };

  //   await getToProfileView();
  // });

  // it('opening the subscription view (iap google)', async () => {
  //   this.ctx.guardianSubscriptionDetailsCallback = req => {
  //     this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
  //         .body = {
  //       plan: {amount: 123, currency: 'foo', interval_count: 12},
  //       payment: {
  //         payment_provider: 'iap_google',
  //         payment_type: 'credit',
  //         last4: '0016',
  //         exp_month: 12,
  //         exp_year: 2022,
  //         brand: 'visa',
  //       },
  //       subscription: {
  //         _subscription_type: 'iap_google',
  //         auto_renewing: true,
  //         expiry_time_millis: 946681200000,
  //       }
  //     }
  //   };

  //   await getToProfileView();
  // });

  it('opening the subscription view (web)', async () => {
    this.ctx.guardianSubscriptionDetailsCallback = req => {
      this.ctx.guardianOverrideEndpoints.GETs['/api/v1/vpn/subscriptionDetails']
          .body = {
        plan: {amount: 123, currency: 'foo', interval_count: 12},
        payment: {
          payment_provider: 'stripe',
          payment_type: 'credit',
          last4: '0016',
          exp_month: 12,
          exp_year: 2022,
          brand: 'visa',
        },
        subscription: {
          _subscription_type: 'web',
          created: 1,
          current_period_end: 2,
          cancel_at_period_end: false,
          status: 'foo'
        }
      }
    };

    await getToProfileView();

    await vpn.waitForElement(
        'subscriptionItem/subscriptionItem-plan/subscriptionItem-plan-parent/subscriptionItem-plan-container/subscriptionItem-plan-valueText');
    await vpn.waitForElementProperty(
        'subscriptionItem/subscriptionItem-plan/subscriptionItem-plan-parent/subscriptionItem-plan-container/subscriptionItem-plan-valueText',
        'visible', 'true');

    assert(
        await vpn.getElementProperty(
            'subscriptionItem/subscriptionItem-plan/subscriptionItem-plan-parent/subscriptionItem-plan-container/subscriptionItem-plan-valueText',
            'text') === 'FOO1.23 Yearly');

    await vpn.waitForElement('subscriptionItem/subscriptionItem-status/subscriptionItem-status-parent/subscriptionItem-status-container/subscriptionItem-status-labelText');
    // TODO: test the status?

    await vpn.waitForElement('subscriptionItem/subscriptionItem-activated/subscriptionItem-activated-parent/subscriptionItem-activated-container/subscriptionItem-activated-valueText');
    assert(
        await vpn.getElementProperty(
            'subscriptionItem/subscriptionItem-activated/subscriptionItem-activated-parent/subscriptionItem-activated-container/subscriptionItem-activated-valueText',
            'text') === '01/01/1970');

    await vpn.waitForElement('subscriptionItem/subscriptionItem-cancelled/subscriptionItem-cancelled-parent/subscriptionItem-cancelled-container/subscriptionItem-cancelled-valueText');
    assert(
      await vpn.getElementProperty(
        'subscriptionItem/subscriptionItem-cancelled/subscriptionItem-cancelled-parent/subscriptionItem-cancelled-container/subscriptionItem-cancelled-valueText',
          'text') === '01/01/1970');

    await vpn.waitForElement('subscriptionItem/subscriptionItem-brand/subscriptionItem-brand-parent/subscriptionItem-brand-container/subscriptionItem-brand-valueText');
    assert(
      await vpn.getElementProperty(
          'subscriptionItem/subscriptionItem-brand/subscriptionItem-brand-parent/subscriptionItem-brand-container/subscriptionItem-brand-valueText',
          'text') === 'Card ending in 0016');

    await vpn.waitForElement('subscriptionItem/subscriptionItem-expires/subscriptionItem-expires-parent/subscriptionItem-expires-container/subscriptionItem-expires-valueText');
    assert(
      await vpn.getElementProperty(
        'subscriptionItem/subscriptionItem-expires/subscriptionItem-expires-parent/subscriptionItem-expires-container/subscriptionItem-expires-valueText',
          'text') === 'Dezember 2022');
  });
});
