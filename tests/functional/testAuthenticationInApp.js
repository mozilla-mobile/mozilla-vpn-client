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

      await vpn.waitForElement('authSignUp-passwordConditionLength');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionLength', '_passwordConditionIsSatisfied',
          'false');
      await vpn.waitForElement('authSignUp-passwordConditionEmailAddress');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionEmailAddress',
          '_passwordConditionIsSatisfied', 'false');
      await vpn.waitForElement('authSignUp-passwordConditionCommon');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionCommon', '_passwordConditionIsSatisfied',
          'false');

      // Condition 1
      await vpn.setElementProperty(
          'authSignUp-passwordInput', 'text', 's', '12345');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionLength', '_passwordConditionIsSatisfied',
          'false');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionEmailAddress',
          '_passwordConditionIsSatisfied', 'false');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionCommon', '_passwordConditionIsSatisfied',
          'false');

      // Condition 2
      await vpn.setElementProperty(
          'authSignUp-passwordInput', 'text', 's', 'test@test');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionLength', '_passwordConditionIsSatisfied',
          'true');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionEmailAddress',
          '_passwordConditionIsSatisfied', 'false');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionCommon', '_passwordConditionIsSatisfied',
          'true');

      // Condition 3
      await vpn.setElementProperty(
          'authSignUp-passwordInput', 'text', 's', '12345678');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionLength', '_passwordConditionIsSatisfied',
          'true');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionEmailAddress',
          '_passwordConditionIsSatisfied', 'true');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionCommon', '_passwordConditionIsSatisfied',
          'false');

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

      await vpn.waitForElement('authSignUp-passwordConditionLength');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionLength', '_passwordConditionIsSatisfied',
          'false');
      await vpn.waitForElement('authSignUp-passwordConditionEmailAddress');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionEmailAddress',
          '_passwordConditionIsSatisfied', 'false');
      await vpn.waitForElement('authSignUp-passwordConditionCommon');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionCommon', '_passwordConditionIsSatisfied',
          'false');

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

      await vpn.waitForElement('authSignUp-passwordConditionLength');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionLength', '_passwordConditionIsSatisfied',
          'false');
      await vpn.waitForElement('authSignUp-passwordConditionEmailAddress');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionEmailAddress',
          '_passwordConditionIsSatisfied', 'false');
      await vpn.waitForElement('authSignUp-passwordConditionCommon');
      await vpn.waitForElementProperty(
          'authSignUp-passwordConditionCommon', '_passwordConditionIsSatisfied',
          'false');

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

  describe('Account creation', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 200,
          body: null,
          callback: (req) => this.ctx.fxaStatusCallback(req)
        },
        '/v1/account/login': {
          status: 200,
          body: null,
          callback: (req) => this.ctx.fxaLoginCallback(req)
        },
      },
      DELETEs: {},
    };

    it('Back and forward', async () => {
      if (!(await vpn.isFeatureFlippedOn('inAppAuthentication'))) {
        await vpn.flipFeatureOn('inAppAuthentication');
        await vpn.flipFeatureOn('inAppAccountCreate');
      }

      // Step 1: main -> start -> main
      await vpn.waitForMainView();
      await vpn.clickOnElement('getStarted');
      await vpn.waitForElement('authStart-backButton');
      await vpn.waitForElementProperty(
          'authStart-backButton', 'visible', 'true');
      await vpn.clickOnElement('authStart-backButton');
      await vpn.waitForMainView();

      // Step 2: main -> start -> help -> start
      await vpn.clickOnElement('getStarted');
      await vpn.waitForElement('authStart-textInput');
      await vpn.waitForElementProperty(
          'authStart-textInput', 'visible', 'true');
      await vpn.waitForElement('authStart-getHelpLink');
      await vpn.clickOnElement('authStart-getHelpLink');
      await vpn.waitForElementProperty('MainStackView', 'busy', 'false');
      await vpn.waitForElement('getHelpBack');
      await vpn.clickOnElement('getHelpBack');
      await vpn.waitForElementProperty('MainStackView', 'busy', 'false');
      await vpn.waitForElement('authStart-textInput');
      await vpn.waitForElementProperty(
          'authStart-textInput', 'visible', 'true');

      // Step 3: start -> sign-up -> help -> sign-up
      this.ctx.fxaStatusCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].body = {
          exists: false
        }
      };
      await vpn.setElementProperty(
          'authStart-textInput', 'text', 's', 'test@test');
      await vpn.waitForElement('authStart-button');
      await vpn.clickOnElement('authStart-button');

      await vpn.waitForElement('authSignUp-getHelpLink');
      await vpn.clickOnElement('authSignUp-getHelpLink');
      await vpn.waitForElementProperty('MainStackView', 'busy', 'false');
      await vpn.waitForElement('getHelpBack');
      await vpn.clickOnElement('getHelpBack');
      await vpn.waitForElementProperty('MainStackView', 'busy', 'false');
      await vpn.waitForElement('authSignUp-backButton');
      await vpn.waitForElementProperty(
          'authSignUp-backButton', 'visible', 'true');

      // Step 4: sign-up -> start
      await vpn.clickOnElement('authSignUp-backButton');
      await vpn.waitForElement('authStart-textInput');
      await vpn.waitForElementProperty(
          'authStart-textInput', 'visible', 'true');

      // Step 5: start -> sign-in -> help -> sign-in
      this.ctx.fxaStatusCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].body = {
          exists: true
        }
      };
      await vpn.setElementProperty(
          'authStart-textInput', 'text', 's', 'test@test');
      await vpn.waitForElement('authStart-button');
      await vpn.clickOnElement('authStart-button');

      await vpn.waitForElement('authSignIn-getHelpLink');
      await vpn.clickOnElement('authSignIn-getHelpLink');
      await vpn.waitForElementProperty('MainStackView', 'busy', 'false');
      await vpn.waitForElement('getHelpBack');
      await vpn.clickOnElement('getHelpBack');
      await vpn.waitForElementProperty('MainStackView', 'busy', 'false');
      await vpn.waitForElement('authSignIn-backButton');
      await vpn.waitForElementProperty(
          'authSignIn-backButton', 'visible', 'true');

      // Step 6: sign-in -> start
      await vpn.clickOnElement('authSignIn-backButton');
      await vpn.waitForElement('authStart-textInput');
      await vpn.waitForElementProperty(
          'authStart-textInput', 'visible', 'true');

      // Step 7: start -> sign-in -> email verification -> help -> email
      // verification
      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          sessionToken: '',
          verified: false,
          verificationMethod: 'email-otp'
        }
      };
      await vpn.setElementProperty(
          'authStart-textInput', 'text', 's', 'test@test');
      await vpn.waitForElement('authStart-button');
      await vpn.clickOnElement('authStart-button');

      await vpn.waitForElement('authSignIn-passwordInput');
      await vpn.waitForElementProperty(
          'authSignIn-passwordInput', 'visible', 'true');
      await vpn.setElementProperty(
          'authSignIn-passwordInput', 'text', 's', 'P4ass0rd!!');

      await vpn.waitForElementProperty('authSignIn-button', 'enabled', 'true');
      await vpn.clickOnElement('authSignIn-button');

      await vpn.waitForElement(
          'authVerificationSessionByEmailNeeded-getHelpLink');
      await vpn.clickOnElement(
          'authVerificationSessionByEmailNeeded-getHelpLink');
      await vpn.waitForElementProperty('MainStackView', 'busy', 'false');
      await vpn.waitForElement('getHelpBack');
      await vpn.clickOnElement('getHelpBack');
      await vpn.waitForElementProperty('MainStackView', 'busy', 'false');
      await vpn.waitForElement(
          'authVerificationSessionByEmailNeeded-backButton');
      await vpn.waitForElementProperty(
          'authVerificationSessionByEmailNeeded-backButton', 'visible', 'true');

      // Step 8: email verification -> start
      await vpn.clickOnElement(
          'authVerificationSessionByEmailNeeded-backButton');
      await vpn.waitForElement('authStart-textInput');
      await vpn.waitForElementProperty(
          'authStart-textInput', 'visible', 'true');

      // Step 9: start -> sign-in -> totp -> help -> totp
      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          sessionToken: '',
          verified: false,
          verificationMethod: 'totp-2fa'
        }
      };
      await vpn.setElementProperty(
          'authStart-textInput', 'text', 's', 'test@test');
      await vpn.waitForElement('authStart-button');
      await vpn.clickOnElement('authStart-button');

      await vpn.waitForElement('authSignIn-passwordInput');
      await vpn.waitForElementProperty(
          'authSignIn-passwordInput', 'visible', 'true');
      await vpn.setElementProperty(
          'authSignIn-passwordInput', 'text', 's', 'P4ass0rd!!');

      await vpn.waitForElementProperty('authSignIn-button', 'enabled', 'true');
      await vpn.clickOnElement('authSignIn-button');

      await vpn.waitForElement(
          'authVerificationSessionByTotpNeeded-getHelpLink');
      await vpn.clickOnElement(
          'authVerificationSessionByTotpNeeded-getHelpLink');
      await vpn.waitForElementProperty('MainStackView', 'busy', 'false');
      await vpn.waitForElement('getHelpBack');
      await vpn.clickOnElement('getHelpBack');
      await vpn.waitForElementProperty('MainStackView', 'busy', 'false');
      await vpn.waitForElement(
          'authVerificationSessionByTotpNeeded-backButton');
      await vpn.waitForElementProperty(
          'authVerificationSessionByTotpNeeded-backButton', 'visible', 'true');

      // Step 10: totp -> main
      await vpn.waitForElement(
          'authVerificationSessionByTotpNeeded-backButton');
      await vpn.waitForElementProperty(
          'authVerificationSessionByTotpNeeded-backButton', 'visible', 'true');
      await vpn.clickOnElement(
          'authVerificationSessionByTotpNeeded-backButton');
      await vpn.waitForMainView();

      // Step 11: main -> start -> sign-in -> unblock code -> help -> unblock
      // code
      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          errno: 107,
          validation: {keys: ['unblockCode']}
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 400;
      };
      await vpn.clickOnElement('getStarted');
      await vpn.waitForElement('authStart-textInput');
      await vpn.waitForElementProperty(
          'authStart-textInput', 'visible', 'true');
      await vpn.setElementProperty(
          'authStart-textInput', 'text', 's', 'test@test');
      await vpn.waitForElement('authStart-button');
      await vpn.clickOnElement('authStart-button');

      await vpn.waitForElement('authSignIn-passwordInput');
      await vpn.waitForElementProperty(
          'authSignIn-passwordInput', 'visible', 'true');
      await vpn.setElementProperty(
          'authSignIn-passwordInput', 'text', 's', 'P4ass0rd!!');

      await vpn.waitForElementProperty('authSignIn-button', 'enabled', 'true');
      await vpn.clickOnElement('authSignIn-button');

      await vpn.waitForElement('authUnblockCodeNeeded-getHelpLink');
      await vpn.clickOnElement('authUnblockCodeNeeded-getHelpLink');
      await vpn.waitForElementProperty('MainStackView', 'busy', 'false');
      await vpn.waitForElement('getHelpBack');
      await vpn.clickOnElement('getHelpBack');
      await vpn.waitForElementProperty('MainStackView', 'busy', 'false');
      await vpn.waitForElement('authUnblockCodeNeeded-backButton');
      await vpn.waitForElementProperty(
          'authUnblockCodeNeeded-backButton', 'visible', 'true');

      // Step 12: unblock code -> start
      await vpn.waitForElement('authUnblockCodeNeeded-backButton');
      await vpn.waitForElementProperty(
          'authUnblockCodeNeeded-backButton', 'visible', 'true');
      await vpn.clickOnElement('authUnblockCodeNeeded-backButton');
      await vpn.waitForElement('authStart-textInput');
      await vpn.waitForElementProperty(
          'authStart-textInput', 'visible', 'true');
    });
  });

  describe('Error handling', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 200,
          body: null,
          callback: (req) => this.ctx.fxaStatusCallback(req)
        },
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
        '/v1/session/verify_code': {
          status: 200,
          body: null,
          callback: (req) => this.ctx.fxaEmailCallback(req)
        },
      },
      DELETEs: {},
    };

    it('Error handling', async () => {
      if (!(await vpn.isFeatureFlippedOn('inAppAuthentication'))) {
        await vpn.flipFeatureOn('inAppAuthentication');
        await vpn.flipFeatureOn('inAppAccountCreate');
      }

      await vpn.clickOnElement('getStarted');

      // Step 1: start -> popup errors
      for (let errorCode
               of [101,  // Account already exists
                   102,  // Unknown account
                   114,  // Client has sent too many requests
                   142,  // Sign in with this email type is not currently
                         // supported
                   149,  // This email can not currently be used to login
                   201,  // Service unavailable
      ]) {
        this.ctx.fxaStatusCallback = (req) => {
          this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].body = {
            errno: errorCode
          };
          this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].status =
              400;
        };

        await vpn.waitForElement('authStart-textInput');
        await vpn.waitForElementProperty(
            'authStart-textInput', 'visible', 'true');
        await vpn.setElementProperty(
            'authStart-textInput', 'text', 's', 'test@test-' + errorCode);
        await vpn.waitForElementProperty('authStart-button', 'enabled', 'true');
        await vpn.waitForElement('authStart-button');
        await vpn.clickOnElement('authStart-button');

        await vpn.waitForElement('authErrorPopup-button');
        await vpn.waitForElementProperty(
            'authErrorPopup-button', 'visible', 'true');
        await vpn.clickOnElement('authErrorPopup-button');
        await vpn.waitForElementProperty(
            'authErrorPopup-button', 'visible', 'false');
        await vpn.waitForElementProperty(
            'authStart-button', 'enabled', 'false');
      }

      // Step 2: start -> email error
      this.ctx.fxaStatusCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].body = {
          errno: 107,
          validation: {keys: ['email']}
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].status = 400;
      };

      await vpn.waitForElementProperty('authStart-button', 'enabled', 'false');
      await vpn.setElementProperty(
          'authStart-textInput', 'text', 's', 'test@testtest');
      await vpn.waitForElementProperty('authStart-button', 'enabled', 'true');
      await vpn.waitForElementProperty(
          'authStart-textInput', 'hasError', 'false');
      await vpn.waitForElement('authStart-button');
      await vpn.clickOnElement('authStart-button');

      await vpn.waitForElementProperty(
          'authStart-textInput', 'hasError', 'true');

      // Step 3: start -> 151 error (failed to send email)
      this.ctx.fxaStatusCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].body = {
          exists: true
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].status = 200;
      };

      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          errno: 151
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 400;
      };

      await vpn.waitForElementProperty('authStart-button', 'enabled', 'false');
      await vpn.setElementProperty(
          'authStart-textInput', 'text', 's', 'test@test');
      await vpn.waitForElementProperty('authStart-button', 'enabled', 'true');
      await vpn.waitForElement('authStart-button');
      await vpn.clickOnElement('authStart-button');

      await vpn.waitForElement('authSignIn-passwordInput');
      await vpn.waitForElementProperty(
          'authSignIn-passwordInput', 'visible', 'true');
      await vpn.setElementProperty(
          'authSignIn-passwordInput', 'text', 's', 'P4ass0rd!!');

      await vpn.waitForElementProperty('authSignIn-button', 'enabled', 'true');
      await vpn.clickOnElement('authSignIn-button');

      await vpn.waitForElement('authErrorPopup-button');
      await vpn.waitForElementProperty(
          'authErrorPopup-button', 'visible', 'true');
      await vpn.clickOnElement('authErrorPopup-button');
      await vpn.waitForElementProperty(
          'authErrorPopup-button', 'visible', 'false');
      await vpn.waitForElementProperty('authSignIn-button', 'enabled', 'false');

      // Step 4: start -> sign-in -> password invalid
      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          errno: 103
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 400;
      };

      await vpn.setElementProperty(
          'authSignIn-passwordInput', 'text', 's', 'P4ass0rd!!!');
      await vpn.waitForElementProperty('authSignIn-button', 'enabled', 'true');
      await vpn.waitForElementProperty(
          'authSignIn-passwordInput', 'hasError', 'false');
      await vpn.clickOnElement('authSignIn-button');
      await vpn.waitForElementProperty(
          'authSignIn-passwordInput', 'hasError', 'true');

      // Step 5: sign-in -> email code -> wrong code
      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          sessionToken: '',
          verified: false,
          verificationMethod: 'email-otp'
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;
      };
      await vpn.waitForElement('authSignIn-passwordInput');
      await vpn.waitForElementProperty(
          'authSignIn-passwordInput', 'visible', 'true');
      await vpn.setElementProperty(
          'authSignIn-passwordInput', 'text', 's', 'P4ass0rd!!!!');

      this.ctx.fxaEmailCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify_code'].status =
            400;
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify_code'].body = {
          errno: 107,
          validation: {keys: ['code']}
        }
      };

      await vpn.waitForElementProperty('authSignIn-button', 'enabled', 'true');
      await vpn.clickOnElement('authSignIn-button');

      await vpn.waitForElement(
          'authVerificationSessionByEmailNeeded-textInput');
      await vpn.waitForElementProperty(
          'authVerificationSessionByEmailNeeded-textInput', 'visible', 'true');
      await vpn.waitForElementProperty(
          'authVerificationSessionByEmailNeeded-button', 'enabled', 'false');
      await vpn.setElementProperty(
          'authVerificationSessionByEmailNeeded-textInput', 'text', 's',
          '123456');
      await vpn.waitForElementProperty(
          'authVerificationSessionByEmailNeeded-button', 'enabled', 'true');

      // Step 6: email code -> back -> start -> totp -> error code
      await vpn.waitForElement(
          'authVerificationSessionByEmailNeeded-backButton');
      await vpn.waitForElementProperty(
          'authVerificationSessionByEmailNeeded-backButton', 'visible', 'true');

      await vpn.clickOnElement(
          'authVerificationSessionByEmailNeeded-backButton');
      await vpn.waitForElement('authStart-textInput');
      await vpn.setElementProperty(
          'authStart-textInput', 'text', 's', 'test@test');
      await vpn.waitForElementProperty('authStart-button', 'enabled', 'true');
      await vpn.waitForElement('authStart-button');
      await vpn.clickOnElement('authStart-button');

      await vpn.waitForElement('authSignIn-passwordInput');
      await vpn.waitForElementProperty(
          'authSignIn-passwordInput', 'visible', 'true');
      await vpn.setElementProperty(
          'authSignIn-passwordInput', 'text', 's', 'P4ass0rd!!');

      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          sessionToken: 'session',
          verified: false,
          verificationMethod: 'totp-2fa'
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;
      };

      this.ctx.fxaTotpCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify/totp'].body = {
          success: false
        }
      };

      await vpn.waitForElementProperty('authSignIn-button', 'enabled', 'true');
      await vpn.clickOnElement('authSignIn-button');

      await vpn.waitForElement('authVerificationSessionByTotpNeeded-textInput');
      await vpn.waitForElementProperty(
          'authVerificationSessionByTotpNeeded-textInput', 'visible', 'true');
      await vpn.waitForElementProperty(
          'authVerificationSessionByTotpNeeded-button', 'enabled', 'false');
      await vpn.setElementProperty(
          'authVerificationSessionByTotpNeeded-textInput', 'text', 's',
          '123456');
      await vpn.waitForElementProperty(
          'authVerificationSessionByTotpNeeded-button', 'enabled', 'true');

      await vpn.waitForElementProperty(
          'authVerificationSessionByTotpNeeded-textInput', 'hasError', 'false');
      await vpn.waitForElement('authVerificationSessionByTotpNeeded-button');
      await vpn.clickOnElement('authVerificationSessionByTotpNeeded-button');
      await vpn.waitForElementProperty(
          'authVerificationSessionByTotpNeeded-textInput', 'hasError', 'true');

      await vpn.waitForElement(
          'authVerificationSessionByTotpNeeded-backButton');
      await vpn.waitForElementProperty(
          'authVerificationSessionByTotpNeeded-backButton', 'visible', 'true');
      await vpn.clickOnElement(
          'authVerificationSessionByTotpNeeded-backButton');
      await vpn.waitForMainView();

      // Step 7: main -> sign up -> code -> error

      this.ctx.fxaStatusCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].body = {
          exists: true
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].status = 200;
      };

      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          errno: 107,
          validation: {keys: ['unblockCode']}
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 400;
      };

      await vpn.clickOnElement('getStarted');
      await vpn.waitForElement('authStart-textInput');
      await vpn.waitForElementProperty(
          'authStart-textInput', 'visible', 'true');
      await vpn.setElementProperty(
          'authStart-textInput', 'text', 's', 'test@test');
      await vpn.waitForElement('authStart-button');
      await vpn.clickOnElement('authStart-button');

      await vpn.waitForElement('authSignIn-passwordInput');
      await vpn.waitForElementProperty(
          'authSignIn-passwordInput', 'visible', 'true');
      await vpn.setElementProperty(
          'authSignIn-passwordInput', 'text', 's', 'P4ass0rd!!');

      await vpn.waitForElementProperty('authSignIn-button', 'enabled', 'true');
      await vpn.clickOnElement('authSignIn-button');

      await vpn.waitForElement('authUnblockCodeNeeded-textInput');
      await vpn.waitForElementProperty(
          'authUnblockCodeNeeded-textInput', 'visible', 'true');
      await vpn.setElementProperty(
          'authUnblockCodeNeeded-textInput', 'text', 's', '12345678');
      await vpn.waitForElementProperty(
          'authUnblockCodeNeeded-textInput', 'hasError', 'false');
      await vpn.waitForElement('authUnblockCodeNeeded-button');
      await vpn.clickOnElement('authUnblockCodeNeeded-button');
      await vpn.waitForElementProperty(
          'authUnblockCodeNeeded-textInput', 'hasError', 'true');
    });
  });
});
