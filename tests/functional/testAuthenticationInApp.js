/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');

describe('User authentication', function() {
  this.timeout(60000);

  it('Completes authentication int app', async () => {
    await vpn.authenticateInApp();
  });

  describe('Account creation', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {status: 200, body: {exists: false}},
        '/v1/account/create': {
          status: 200,
          body: {
            sessionToken: 'sessionToken',
            'verified': true,
            verificationMethod: ''
          }
        }
      },
      DELETEs: {},
    };

    it('Account creation', async () => {
      if (!(await vpn.isFeatureFlippedOn('inAppAuthentication'))) {
        await vpn.flipFeatureOn('inAppAuthentication');
        await vpn.flipFeatureOn('inAppAccountCreate');
      }

      await vpn.waitForMainView();

      await vpn.clickOnElement('getStarted');
      await vpn.waitForElement('authStart-textInput');
      await vpn.setElementProperty(
          'authStart-textInput', 'text', 's', 'test@test');
      await vpn.waitForElement('authStart-button');
      await vpn.clickOnElement('authStart-button');

      await vpn.waitForElement('authSignUp-passwordInput');

      await vpn.waitForElement('passwordConditionLength');
      await vpn.waitForElementProperty(
          'passwordConditionLength', '_passwordConditionIsSatisfied', 'false');
      await vpn.waitForElement('passwordConditionEmailAddress');
      await vpn.waitForElementProperty(
          'passwordConditionEmailAddress', '_passwordConditionIsSatisfied',
          'false');
      await vpn.waitForElement('passwordConditionCommon');
      await vpn.waitForElementProperty(
          'passwordConditionCommon', '_passwordConditionIsSatisfied', 'false');

      // Condition 1
      await vpn.setElementProperty(
          'authSignUp-passwordInput', 'text', 's', '12345');
      await vpn.waitForElementProperty(
          'passwordConditionLength', '_passwordConditionIsSatisfied', 'false');
      await vpn.waitForElementProperty(
          'passwordConditionEmailAddress', '_passwordConditionIsSatisfied',
          'false');
      await vpn.waitForElementProperty(
          'passwordConditionCommon', '_passwordConditionIsSatisfied', 'false');

      // Condition 2
      await vpn.setElementProperty(
          'authSignUp-passwordInput', 'text', 's', 'test@test');
      await vpn.waitForElementProperty(
          'passwordConditionLength', '_passwordConditionIsSatisfied', 'true');
      await vpn.waitForElementProperty(
          'passwordConditionEmailAddress', '_passwordConditionIsSatisfied',
          'false');
      await vpn.waitForElementProperty(
          'passwordConditionCommon', '_passwordConditionIsSatisfied', 'true');

      // Condition 3
      await vpn.setElementProperty(
          'authSignUp-passwordInput', 'text', 's', '12345678');
      await vpn.waitForElementProperty(
          'passwordConditionLength', '_passwordConditionIsSatisfied', 'true');
      await vpn.waitForElementProperty(
          'passwordConditionEmailAddress', '_passwordConditionIsSatisfied',
          'true');
      await vpn
          .waitForElementProperty(
              'passwordConditionCommon', '_passwordConditionIsSatisfied',
              'false')

              await vpn.setElementProperty(
                  'authSignUp-passwordInput', 'text', 's', 'P4ass0rd!!');
      await vpn.clickOnElement('authSignUp-button');

      await vpn.waitForElementProperty('VPN', 'userState', 'UserAuthenticated');
      await vpn.waitForElement('postAuthenticationButton');

      await vpn.clickOnElement('postAuthenticationButton');
      await vpn.wait();

      await vpn.waitForElement('telemetryPolicyButton');
      await vpn.clickOnElement('telemetryPolicyButton');
      await vpn.waitForElement('controllerTitle');
    });
  });
});
