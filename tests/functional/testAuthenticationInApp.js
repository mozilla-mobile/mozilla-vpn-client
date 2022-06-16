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
      await vpn.waitForElementProperty(
          'authStart-textInput', 'visible', 'true');
      await vpn.waitForElementProperty('authStart-button', 'enabled', 'false');
      await vpn.setElementProperty(
          'authStart-textInput', 'text', 's', 'test@test');
      await vpn.waitForElementProperty('authStart-button', 'enabled', 'true');
      await vpn.waitForElement('authStart-button');
      await vpn.clickOnElement('authStart-button');

      await vpn.waitForElement('authSignUp-passwordInput');
      await vpn.waitForElementProperty(
          'authSignUp-passwordInput', 'visible', 'true');

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
      await vpn.waitForElementProperty(
          'passwordConditionCommon', '_passwordConditionIsSatisfied', 'false');

      await vpn.waitForElementProperty('authSignUp-button', 'enabled', 'false');
      await vpn.setElementProperty(
          'authSignUp-passwordInput', 'text', 's', 'P4ass0rd!!');
      await vpn.waitForElementProperty('authSignUp-button', 'enabled', 'true');
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

  describe('Account creation with email verification', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {status: 200, body: {exists: false}},
        '/v1/account/create': {
          status: 200,
          body: {
            sessionToken: 'sessionToken',
            'verified': false,
            verificationMethod: 'email-otp'
          }
        },
        '/v1/session/verify_code': {status: 200, body: {}},
      },
      DELETEs: {},
    };

    it('Account creation with email verification', async () => {
      if (!(await vpn.isFeatureFlippedOn('inAppAuthentication'))) {
        await vpn.flipFeatureOn('inAppAuthentication');
        await vpn.flipFeatureOn('inAppAccountCreate');
      }

      await vpn.waitForMainView();

      await vpn.clickOnElement('getStarted');
      await vpn.waitForElement('authStart-textInput');
      await vpn.waitForElementProperty(
          'authStart-textInput', 'visible', 'true');
      await vpn.setElementProperty(
          'authStart-textInput', 'text', 's', 'test@test');
      await vpn.waitForElement('authStart-button');
      await vpn.clickOnElement('authStart-button');

      await vpn.waitForElement('authSignUp-passwordInput');
      await vpn.waitForElementProperty(
          'authSignUp-passwordInput', 'visible', 'true');

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

      await vpn.setElementProperty(
          'authSignUp-passwordInput', 'text', 's', 'P4ass0rd!!');
      await vpn.clickOnElement('authSignUp-button');

      await vpn.waitForElement(
          'authVerificationSessionByEmailNeeded-textInput');
      await vpn.waitForElementProperty(
          'authVerificationSessionByEmailNeeded-textInput', 'visible', 'true');
      await vpn.waitForElementProperty(
          'authVerificationSessionByEmailNeeded-button', 'enabled', 'false');
      await vpn.setElementProperty(
          'authVerificationSessionByEmailNeeded-textInput', 'text', 's',
          '12345');
      await vpn.waitForElementProperty(
          'authVerificationSessionByEmailNeeded-button', 'enabled', 'false');
      await vpn.setElementProperty(
          'authVerificationSessionByEmailNeeded-textInput', 'text', 's',
          '123456');
      await vpn.waitForElementProperty(
          'authVerificationSessionByEmailNeeded-button', 'enabled', 'true');
      await vpn.waitForElement('authVerificationSessionByEmailNeeded-button');
      await vpn.clickOnElement('authVerificationSessionByEmailNeeded-button');

      await vpn.waitForElementProperty('VPN', 'userState', 'UserAuthenticated');
      await vpn.waitForElement('postAuthenticationButton');

      await vpn.clickOnElement('postAuthenticationButton');
      await vpn.wait();

      await vpn.waitForElement('telemetryPolicyButton');
      await vpn.clickOnElement('telemetryPolicyButton');
      await vpn.waitForElement('controllerTitle');
    });
  });

  describe('Account creation with TOTP', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {status: 200, body: {exists: false}},
        '/v1/account/create': {
          status: 200,
          body: {
            sessionToken: 'sessionToken',
            'verified': false,
            verificationMethod: 'totp-2fa'
          }
        },
        '/v1/session/verify/totp': {status: 200, body: {success: true}},
      },
      DELETEs: {},
    };

    it('Account creation with TOTP', async () => {
      if (!(await vpn.isFeatureFlippedOn('inAppAuthentication'))) {
        await vpn.flipFeatureOn('inAppAuthentication');
        await vpn.flipFeatureOn('inAppAccountCreate');
      }

      await vpn.waitForMainView();

      await vpn.clickOnElement('getStarted');
      await vpn.waitForElement('authStart-textInput');
      await vpn.waitForElementProperty(
          'authStart-textInput', 'visible', 'true');
      await vpn.setElementProperty(
          'authStart-textInput', 'text', 's', 'test@test');
      await vpn.waitForElement('authStart-button');
      await vpn.clickOnElement('authStart-button');

      await vpn.waitForElement('authSignUp-passwordInput');
      await vpn.waitForElementProperty(
          'authSignUp-passwordInput', 'visible', 'true');

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

      await vpn.setElementProperty(
          'authSignUp-passwordInput', 'text', 's', 'P4ass0rd!!');
      await vpn.clickOnElement('authSignUp-button');

      await vpn.waitForElement('authVerificationSessionByTotpNeeded-textInput');
      await vpn.waitForElementProperty(
          'authVerificationSessionByTotpNeeded-textInput', 'visible', 'true');
      await vpn.waitForElementProperty(
          'authVerificationSessionByTotpNeeded-button', 'enabled', 'false');
      await vpn.setElementProperty(
          'authVerificationSessionByTotpNeeded-textInput', 'text', 's',
          '12345');
      await vpn.waitForElementProperty(
          'authVerificationSessionByTotpNeeded-button', 'enabled', 'false');
      await vpn.setElementProperty(
          'authVerificationSessionByTotpNeeded-textInput', 'text', 's',
          '123456');
      await vpn.waitForElementProperty(
          'authVerificationSessionByTotpNeeded-button', 'enabled', 'true');
      await vpn.waitForElement('authVerificationSessionByTotpNeeded-button');
      await vpn.clickOnElement('authVerificationSessionByTotpNeeded-button');

      await vpn.waitForElementProperty('VPN', 'userState', 'UserAuthenticated');
      await vpn.waitForElement('postAuthenticationButton');

      await vpn.clickOnElement('postAuthenticationButton');
      await vpn.wait();

      await vpn.waitForElement('telemetryPolicyButton');
      await vpn.clickOnElement('telemetryPolicyButton');
      await vpn.waitForElement('controllerTitle');
    });
  });

  describe('Authentication with unblock code', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 400,
          body: {errno: 107, validation: {keys: ['unblockCode']}}
        },
      },
      DELETEs: {},
    };

    it('Authentication with unblock code', async () => {
      if (!(await vpn.isFeatureFlippedOn('inAppAuthentication'))) {
        await vpn.flipFeatureOn('inAppAuthentication');
        await vpn.flipFeatureOn('inAppAccountCreate');
      }

      await vpn.waitForMainView();

      await vpn.clickOnElement('getStarted');
      await vpn.waitForElement('authStart-textInput');
      await vpn.waitForElementProperty(
          'authStart-textInput', 'visible', 'true');
      await vpn.setElementProperty(
          'authStart-textInput', 'text', 's', 'test@test');
      await vpn.waitForElement('authStart-button');
      await vpn.clickOnElement('authStart-button');

      await vpn.waitForElement('authUnblockCodeNeeded-textInput');
      await vpn.waitForElementProperty(
          'authUnblockCodeNeeded-textInput', 'visible', 'true');
      await vpn.waitForElementProperty(
          'authUnblockCodeNeeded-button', 'enabled', 'false');
      await vpn.setElementProperty(
          'authUnblockCodeNeeded-textInput', 'text', 's', '1234567');
      await vpn.waitForElementProperty(
          'authUnblockCodeNeeded-button', 'enabled', 'false');
      await vpn.setElementProperty(
          'authUnblockCodeNeeded-textInput', 'text', 's', '12345678');
      await vpn.waitForElementProperty(
          'authUnblockCodeNeeded-button', 'enabled', 'true');
      await vpn.waitForElement('authUnblockCodeNeeded-button');
      await vpn.clickOnElement('authUnblockCodeNeeded-button');

      await vpn.waitForElementProperty('VPN', 'userState', 'UserAuthenticated');
      await vpn.waitForElement('postAuthenticationButton');

      await vpn.clickOnElement('postAuthenticationButton');
      await vpn.wait();

      await vpn.waitForElement('telemetryPolicyButton');
      await vpn.clickOnElement('telemetryPolicyButton');
      await vpn.waitForElement('controllerTitle');
    });
  });

  // TODO:
  // - errors
});
