/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert'); // TODO: add asserts to each block
const { initialView, telemetryView, generalElements, settingsView, authView } = require('./elements.js');
const vpn = require('./helper.js');

describe('User authentication', function() {
  this.timeout(300000);

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

    it.only('Account creation', async () => {
      if (!(await vpn.isFeatureFlippedOn('inAppAuthentication'))) {
        await vpn.flipFeatureOn('inAppAuthentication');
        await vpn.flipFeatureOn('inAppAccountCreate');
      }

      await vpn.waitForMainView();

      await vpn.clickOnElement(initialView.GET_STARTED);
      await vpn.waitForElement(authView.EMAIL_INPUT);
      await vpn.waitForElementProperty(
          authView.EMAIL_INPUT, 'visible', 'true');
      await vpn.waitForElementProperty(authView.START_BUTTON, 'enabled', 'false');            
      await vpn.setElementProperty(authView.EMAIL_INPUT, 'text', 's', 'test@test');
      await vpn.waitForElementProperty(authView.START_BUTTON, 'enabled', 'true');
      await vpn.waitForElement(authView.START_BUTTON);
      await vpn.clickOnElement(authView.START_BUTTON);

      console.log('wait for pass: ');
      await vpn.wait(6000)
      await vpn.waitForElement(authView.PASS_INPUT);
      await vpn.waitForElementProperty(
          authView.PASS_INPUT, 'visible', 'true');

      await vpn.waitForElement(authView.PASS_LENGTH_CONDITION);
      await vpn.waitForElementProperty(
          authView.PASS_LENGTH_CONDITION, '_passwordConditionIsSatisfied',
          'false');
      await vpn.waitForElement(authView.EMAIL_INPUT_CONDITIOIN);
      await vpn.waitForElementProperty(
          authView.EMAIL_INPUT_CONDITIOIN,
          '_passwordConditionIsSatisfied', 'false');
      await vpn.waitForElement(authView.PASS_INPUT_CONDITION);
      await vpn.waitForElementProperty(
          authView.PASS_INPUT_CONDITION, '_passwordConditionIsSatisfied',
          'false');

      // Condition 1
      await vpn.setElementProperty(
          authView.PASS_INPUT, 'text', 's', '12345');
      await vpn.waitForElementProperty(
          authView.PASS_LENGTH_CONDITION, '_passwordConditionIsSatisfied',
          'false');
      await vpn.waitForElementProperty(
          authView.EMAIL_INPUT_CONDITIOIN,
          '_passwordConditionIsSatisfied', 'false');
      await vpn.waitForElementProperty(
          authView.PASS_INPUT_CONDITION, '_passwordConditionIsSatisfied',
          'false');

      // Condition 2
      await vpn.setElementProperty(
          authView.PASS_INPUT, 'text', 's', 'test@test');
      await vpn.waitForElementProperty(authView.PASS_LENGTH_CONDITION, '_passwordConditionIsSatisfied',
          'true');
      await vpn.waitForElementProperty(authView.EMAIL_INPUT_CONDITIOIN,
          '_passwordConditionIsSatisfied', 'false');
      await vpn.waitForElementProperty(authView.PASS_INPUT_CONDITION, '_passwordConditionIsSatisfied',
          'true');

      // Condition 3
      await vpn.setElementProperty(
          authView.PASS_INPUT, 'text', 's', '12345678');
      await vpn.waitForElementProperty(authView.PASS_LENGTH_CONDITION, '_passwordConditionIsSatisfied',
          'true');
      await vpn.waitForElementProperty(authView.EMAIL_INPUT_CONDITIOIN,
          '_passwordConditionIsSatisfied', 'true');
      await vpn.waitForElementProperty(authView.PASS_INPUT_CONDITION, '_passwordConditionIsSatisfied',
          'false');

      await vpn.waitForElementProperty(authView.SIGNUP_BUTTON, 'enabled', 'false');
      await vpn.setElementProperty(
          authView.PASS_INPUT, 'text', 's', 'P4ass0rd!!');
      await vpn.waitForElementProperty(authView.SIGNUP_BUTTON, 'enabled', 'true');
      await vpn.clickOnElement(authView.SIGNUP_BUTTON);

      await vpn.waitForElementProperty('VPN', 'userState', 'UserAuthenticated');
      await vpn.waitForElementAndClick(telemetryView.POST_AUTHENTICATION_BUTTON);

      await vpn.waitForElement(telemetryView.TELEMETRY_POLICY_BUTTON);
      await vpn.clickOnElement(telemetryView.TELEMETRY_POLICY_BUTTON);
      await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
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

      await vpn.clickOnElement(initialView.GET_STARTED);
      await vpn.waitForElement(authView.EMAIL_INPUT);
      await vpn.waitForElementProperty(
          authView.EMAIL_INPUT, 'visible', 'true');
      await vpn.setElementProperty(
          authView.EMAIL_INPUT, 'text', 's', 'test@test');
      await vpn.waitForElement(authView.START_BUTTON);
      await vpn.clickOnElement(authView.START_BUTTON);

      await vpn.waitForElement(authView.PASS_INPUT);
      await vpn.waitForElementProperty(
          authView.PASS_INPUT, 'visible', 'true');

      await vpn.waitForElement(authView.PASS_LENGTH_CONDITION);
      await vpn.waitForElementProperty(authView.PASS_LENGTH_CONDITION, '_passwordConditionIsSatisfied',
          'false');
      await vpn.waitForElement(authView.EMAIL_INPUT_CONDITIOIN);
      await vpn.waitForElementProperty(authView.EMAIL_INPUT_CONDITIOIN,
          '_passwordConditionIsSatisfied', 'false');
      await vpn.waitForElement(authView.PASS_INPUT_CONDITION);
      await vpn.waitForElementProperty(
        authView.PASS_INPUT_CONDITION, '_passwordConditionIsSatisfied',
          'false');

      await vpn.setElementProperty(authView.PASS_INPUT, 'text', 's', 'P4ass0rd!!');
      await vpn.clickOnElement(authView.SIGNUP_BUTTON);

      await vpn.waitForElement(
          authView.verificationByEmailView.TEXT_INPUT);
      await vpn.waitForElementProperty(
          authView.verificationByEmailView.TEXT_INPUT, 'visible', 'true');
      await vpn.waitForElementProperty(
          authView.verificationByEmailView.EMAIL_BUTTON, 'enabled', 'false');
      await vpn.setElementProperty(
          authView.verificationByEmailView.TEXT_INPUT, 'text', 's',
          '12345');
      await vpn.waitForElementProperty(
          authView.verificationByEmailView.EMAIL_BUTTON, 'enabled', 'false');
      await vpn.setElementProperty(
          authView.verificationByEmailView.TEXT_INPUT, 'text', 's',
          '123456');
      await vpn.waitForElementProperty(
          authView.verificationByEmailView.EMAIL_BUTTON, 'enabled', 'true');
      await vpn.waitForElement(authView.verificationByEmailView.EMAIL_BUTTON);
      await vpn.clickOnElement(authView.verificationByEmailView.EMAIL_BUTTON);

      await vpn.waitForElementProperty('VPN', 'userState', 'UserAuthenticated');
      await vpn.waitForElementAndClick(telemetryView.POST_AUTHENTICATION_BUTTON);

      await vpn.waitForElement(telemetryView.TELEMETRY_POLICY_BUTTON);
      await vpn.clickOnElement(telemetryView.TELEMETRY_POLICY_BUTTON);
      await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
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

      await vpn.clickOnElement(initialView.GET_STARTED);
      await vpn.waitForElement(authView.EMAIL_INPUT);
      await vpn.waitForElementProperty(
          authView.EMAIL_INPUT, 'visible', 'true');
      await vpn.setElementProperty(
          authView.EMAIL_INPUT, 'text', 's', 'test@test');
      await vpn.waitForElement(authView.START_BUTTON);
      await vpn.clickOnElement(authView.START_BUTTON);

      await vpn.waitForElement(authView.PASS_INPUT);
      await vpn.waitForElementProperty(authView.PASS_INPUT, 'visible', 'true');

      await vpn.waitForElement(authView.PASS_LENGTH_CONDITION);
      await vpn.waitForElementProperty(authView.PASS_LENGTH_CONDITION, '_passwordConditionIsSatisfied',
          'false');
      await vpn.waitForElement(authView.EMAIL_INPUT_CONDITIOIN);
      await vpn.waitForElementProperty(authView.EMAIL_INPUT_CONDITIOIN, '_passwordConditionIsSatisfied', 'false');
      await vpn.waitForElement(authView.PASS_INPUT_CONDITION);
      await vpn.waitForElementProperty(authView.PASS_INPUT_CONDITION, '_passwordConditionIsSatisfied',
          'false');

      await vpn.setElementProperty(authView.PASS_INPUT, 'text', 's', 'P4ass0rd!!');
      await vpn.clickOnElement(authView.SIGNUP_BUTTON);

      await vpn.waitForElement(authView.verificationBySessionTotpView.TEXT_INPUT);
      await vpn.waitForElementProperty(
          authView.verificationBySessionTotpView.TEXT_INPUT, 'visible', 'true');
      await vpn.waitForElementProperty(
          authView.verificationBySessionTotpView.TOTP_NEEDED_BUTTON, 'enabled', 'false');
      await vpn.setElementProperty(
          authView.verificationBySessionTotpView.TEXT_INPUT, 'text', 's',
          '12345');
      await vpn.waitForElementProperty(
          authView.verificationBySessionTotpView.TOTP_NEEDED_BUTTON, 'enabled', 'false');
      await vpn.setElementProperty(
          authView.verificationBySessionTotpView.TEXT_INPUT, 'text', 's',
          '123456');
      await vpn.waitForElementProperty(
          authView.verificationBySessionTotpView.TOTP_NEEDED_BUTTON, 'enabled', 'true');
      await vpn.waitForElement(authView.verificationBySessionTotpView.TOTP_NEEDED_BUTTON);
      await vpn.clickOnElement(authView.verificationBySessionTotpView.TOTP_NEEDED_BUTTON);

      await vpn.waitForElementProperty('VPN', 'userState', 'UserAuthenticated');
      await vpn.waitForElementAndClick(telemetryView.POST_AUTHENTICATION_BUTTON);
      await vpn.wait();

      await vpn.waitForElement(telemetryView.TELEMETRY_POLICY_BUTTON);
      await vpn.clickOnElement(telemetryView.TELEMETRY_POLICY_BUTTON);
      await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
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

      await vpn.clickOnElement(initialView.GET_STARTED);
      await vpn.waitForElement(authView.EMAIL_INPUT);
      await vpn.waitForElementProperty(
          authView.EMAIL_INPUT, 'visible', 'true');
      await vpn.setElementProperty(
          authView.EMAIL_INPUT, 'text', 's', 'test@test');
      await vpn.waitForElement(authView.START_BUTTON);
      await vpn.clickOnElement(authView.START_BUTTON);

      await vpn.waitForElement(authView.authUnblockCodeView.CODE_INPUT);
      await vpn.waitForElementProperty(
          authView.authUnblockCodeView.CODE_INPUT, 'visible', 'true');
      await vpn.waitForElementProperty(
          authView.authUnblockCodeView.CODE_BUTTON, 'enabled', 'false');
      await vpn.setElementProperty(
          authView.authUnblockCodeView.CODE_INPUT, 'text', 's', '1234567');
      await vpn.waitForElementProperty(
          authView.authUnblockCodeView.CODE_BUTTON, 'enabled', 'false');
      await vpn.setElementProperty(
          authView.authUnblockCodeView.CODE_INPUT, 'text', 's', '12345678');
      await vpn.waitForElementProperty(
          authView.authUnblockCodeView.CODE_BUTTON, 'enabled', 'true');      
      await vpn.waitForElementAndClick(authView.authUnblockCodeView.CODE_BUTTON);

      await vpn.waitForElementProperty('VPN', 'userState', 'UserAuthenticated');
      await vpn.waitForElementAndClick(telemetryView.POST_AUTHENTICATION_BUTTON);

      await vpn.waitForElementAndClick(telemetryView.TELEMETRY_POLICY_BUTTON);
      await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
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
      await vpn.clickOnElement(initialView.GET_STARTED);
      await vpn.waitForElement(authView.START_BUTTON);
      await vpn.waitForElementProperty(
          authView.START_BUTTON, 'visible', 'true');
      await vpn.clickOnElement(authView.START_BUTTON);
      await vpn.waitForMainView();

      // Step 2: main -> start -> help -> start
      await vpn.clickOnElement(initialView.GET_STARTED);
      await vpn.waitForElement(authView.EMAIL_INPUT);
      await vpn.waitForElementProperty(
          authView.EMAIL_INPUT, 'visible', 'true');
      await vpn.waitForElement(authView.GET_HELP_LINK);
      await vpn.clickOnElement(authView.GET_HELP_LINK);
      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');
      await vpn.waitForElement(settingsView.getHelpView.BACK);
      await vpn.clickOnElement(settingsView.getHelpView.BACK);
      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');
      await vpn.waitForElement(authView.EMAIL_INPUT);
      await vpn.waitForElementProperty(
          authView.EMAIL_INPUT, 'visible', 'true');

      // Step 3: start -> sign-up -> help -> sign-up
      this.ctx.fxaStatusCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].body = {
          exists: false
        }
      };
      await vpn.setElementProperty(
          authView.EMAIL_INPUT, 'text', 's', 'test@test');
      await vpn.waitForElement(authView.START_BUTTON);
      await vpn.clickOnElement(authView.START_BUTTON);

      await vpn.waitForElement(authView.GET_HELP_LINK);
      await vpn.clickOnElement(authView.GET_HELP_LINK);
      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');
      await vpn.waitForElement(settingsView.getHelpView.BACK);
      await vpn.clickOnElement(settingsView.getHelpView.BACK);
      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');
      await vpn.waitForElement(authView.SIGNUP_BACK);
      await vpn.waitForElementProperty(authView.SIGNUP_BACK, 'visible', 'true');

      // Step 4: sign-up -> start
      await vpn.clickOnElement(authView.SIGNUP_BACK);
      await vpn.waitForElement(authView.EMAIL_INPUT);
      await vpn.waitForElementProperty(
          authView.EMAIL_INPUT, 'visible', 'true');

      // Step 5: start -> sign-in -> help -> sign-in
      this.ctx.fxaStatusCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].body = {
          exists: true
        }
      };
      await vpn.setElementProperty(
          authView.EMAIL_INPUT, 'text', 's', 'test@test');
      await vpn.waitForElement(authView.START_BUTTON);
      await vpn.clickOnElement(authView.START_BUTTON);

      await vpn.waitForElement(authView.GET_HELP_LINK);
      await vpn.clickOnElement(authView.GET_HELP_LINK);
      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');
      await vpn.waitForElement(settingsView.getHelpView.BACK);
      await vpn.clickOnElement(settingsView.getHelpView.BACK);
      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');
      await vpn.waitForElement(authView.SIGNIN_BACK);
      await vpn.waitForElementProperty(authView.SIGNIN_BACK, 'visible', 'true');

      // Step 6: sign-in -> start
      await vpn.clickOnElement(authView.SIGNIN_BACK);
      await vpn.waitForElement(authView.EMAIL_INPUT);
      await vpn.waitForElementProperty(
          authView.EMAIL_INPUT, 'visible', 'true');

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
          authView.EMAIL_INPUT, 'text', 's', 'test@test');
      await vpn.waitForElement(authView.START_BUTTON);
      await vpn.clickOnElement(authView.START_BUTTON);

      await vpn.waitForElement(authView.PASS_INPUT);
      await vpn.waitForElementProperty(
          authView.PASS_INPUT, 'visible', 'true');
      await vpn.setElementProperty(
          authView.PASS_INPUT, 'text', 's', 'P4ass0rd!!');

      await vpn.waitForElementProperty(authView.SIGNIN_BUTTON, 'enabled', 'true');
      await vpn.clickOnElement(authView.SIGNIN_BUTTON);

      await vpn.waitForElement(authView.verificationByEmailView.GET_HELP);
      await vpn.clickOnElement(authView.verificationByEmailView.GET_HELP);
      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');
      await vpn.waitForElement(settingsView.getHelpView.BACK);
      await vpn.clickOnElement(settingsView.getHelpView.BACK);
      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');
      await vpn.waitForElement(
          authView.verificationByEmailView.BACK);
      await vpn.waitForElementProperty(
          authView.verificationByEmailView.BACK, 'visible', 'true');

      // Step 8: email verification -> start
      await vpn.clickOnElement(
          authView.verificationByEmailView.BACK);
      await vpn.waitForElement(authView.EMAIL_INPUT);
      await vpn.waitForElementProperty(
          authView.EMAIL_INPUT, 'visible', 'true');

      // Step 9: start -> sign-in -> totp -> help -> totp
      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          sessionToken: '',
          verified: false,
          verificationMethod: 'totp-2fa'
        }
      };
      await vpn.setElementProperty(
          authView.EMAIL_INPUT, 'text', 's', 'test@test');
      await vpn.waitForElement(authView.START_BUTTON);
      await vpn.clickOnElement(authView.START_BUTTON);

      await vpn.waitForElement(authView.PASS_INPUT);
      await vpn.waitForElementProperty(
          authView.PASS_INPUT, 'visible', 'true');
      await vpn.setElementProperty(
          authView.PASS_INPUT, 'text', 's', 'P4ass0rd!!');

      await vpn.waitForElementProperty(authView.SIGNIN_BUTTON, 'enabled', 'true');
      await vpn.clickOnElement(authView.SIGNIN_BUTTON);

      await vpn.waitForElement(authView.verificationBySessionTotpView.GET_HELP_LINK);
      await vpn.clickOnElement(authView.verificationBySessionTotpView.GET_HELP_LINK);
      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');
      await vpn.waitForElement(settingsView.getHelpView.BACK);
      await vpn.clickOnElement(settingsView.getHelpView.BACK);
      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');
      await vpn.waitForElement(authView.verificationBySessionTotpView.BACK);
      await vpn.waitForElementProperty(authView.verificationBySessionTotpView.BACK, 'visible', 'true');

      // Step 10: totp -> main
      await vpn.waitForElement(authView.verificationBySessionTotpView.BACK);
      await vpn.waitForElementProperty(authView.verificationBySessionTotpView.BACK, 'visible', 'true');
      await vpn.clickOnElement(authView.verificationBySessionTotpView.BACK);
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
      await vpn.clickOnElement(initialView.GET_STARTED);
      await vpn.waitForElement(authView.EMAIL_INPUT);
      await vpn.waitForElementProperty(
          authView.EMAIL_INPUT, 'visible', 'true');
      await vpn.setElementProperty(
          authView.EMAIL_INPUT, 'text', 's', 'test@test');
      await vpn.waitForElement(authView.START_BUTTON);
      await vpn.clickOnElement(authView.START_BUTTON);

      await vpn.waitForElement(authView.PASS_INPUT);
      await vpn.waitForElementProperty(
          authView.PASS_INPUT, 'visible', 'true');
      await vpn.setElementProperty(
          authView.PASS_INPUT, 'text', 's', 'P4ass0rd!!');

      await vpn.waitForElementProperty(authView.SIGNIN_BUTTON, 'enabled', 'true');
      await vpn.clickOnElement(authView.SIGNIN_BUTTON);

      await vpn.waitForElement(authView.authUnblockCodeView.GET_HELP_LINK);
      await vpn.clickOnElement(authView.authUnblockCodeView.GET_HELP_LINK);
      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');
      await vpn.waitForElement(settingsView.getHelpView.BACK);
      await vpn.clickOnElement(settingsView.getHelpView.BACK);
      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');
      await vpn.waitForElement(authView.authUnblockCodeView.BACK);
      await vpn.waitForElementProperty(
          authView.authUnblockCodeView.BACK, 'visible', 'true');

      // Step 12: unblock code -> start
      await vpn.waitForElement(authView.authUnblockCodeView.BACK);
      await vpn.waitForElementProperty(
          authView.authUnblockCodeView.BACK, 'visible', 'true');
      await vpn.clickOnElement(authView.authUnblockCodeView.BACK);
      await vpn.waitForElement(authView.EMAIL_INPUT);
      await vpn.waitForElementProperty(
          authView.EMAIL_INPUT, 'visible', 'true');
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

      await vpn.clickOnElement(initialView.GET_STARTED);

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

        await vpn.waitForElement(authView.EMAIL_INPUT);
        await vpn.waitForElementProperty(
            authView.EMAIL_INPUT, 'visible', 'true');
        await vpn.setElementProperty(
            authView.EMAIL_INPUT, 'text', 's', 'test@test-' + errorCode);
        await vpn.waitForElementProperty(authView.START_BUTTON, 'enabled', 'true');
        await vpn.waitForElement(authView.START_BUTTON);
        await vpn.clickOnElement(authView.START_BUTTON);

        await vpn.waitForElement(authView.ERROR_POPUP_BUTTON);
        await vpn.waitForElementProperty(
            authView.ERROR_POPUP_BUTTON, 'visible', 'true');
        await vpn.clickOnElement(authView.ERROR_POPUP_BUTTON);
        await vpn.waitForElementProperty(
            authView.ERROR_POPUP_BUTTON, 'visible', 'false');
        await vpn.waitForElementProperty(
            authView.START_BUTTON, 'enabled', 'false');
      }

      // Step 2: start -> email error
      this.ctx.fxaStatusCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].body = {
          errno: 107,
          validation: {keys: ['email']}
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].status = 400;
      };

      await vpn.waitForElementProperty(authView.START_BUTTON, 'enabled', 'false');
      await vpn.setElementProperty(
          authView.EMAIL_INPUT, 'text', 's', 'test@testtest');
      await vpn.waitForElementProperty(authView.START_BUTTON, 'enabled', 'true');
      await vpn.waitForElementProperty(
          authView.EMAIL_INPUT, 'hasError', 'false');
      await vpn.waitForElement(authView.START_BUTTON);
      await vpn.clickOnElement(authView.START_BUTTON);

      await vpn.waitForElementProperty(
          authView.EMAIL_INPUT, 'hasError', 'true');

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

      await vpn.waitForElementProperty(authView.START_BUTTON, 'enabled', 'false');
      await vpn.setElementProperty(
          authView.EMAIL_INPUT, 'text', 's', 'test@test');
      await vpn.waitForElementProperty(authView.START_BUTTON, 'enabled', 'true');
      await vpn.waitForElement(authView.START_BUTTON);
      await vpn.clickOnElement(authView.START_BUTTON);

      await vpn.waitForElement(authView.PASS_INPUT);
      await vpn.waitForElementProperty(
          authView.PASS_INPUT, 'visible', 'true');
      await vpn.setElementProperty(
          authView.PASS_INPUT, 'text', 's', 'P4ass0rd!!');

      await vpn.waitForElementProperty(authView.SIGNIN_BUTTON, 'enabled', 'true');
      await vpn.clickOnElement(authView.SIGNIN_BUTTON);

      await vpn.waitForElement(authView.ERROR_POPUP_BUTTON);
      await vpn.waitForElementProperty(
          authView.ERROR_POPUP_BUTTON, 'visible', 'true');
      await vpn.clickOnElement(authView.ERROR_POPUP_BUTTON);
      await vpn.waitForElementProperty(
          authView.ERROR_POPUP_BUTTON, 'visible', 'false');
      await vpn.waitForElementProperty(authView.SIGNIN_BUTTON, 'enabled', 'false');

      // Step 4: start -> sign-in -> password invalid
      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          errno: 103
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 400;
      };

      await vpn.setElementProperty(
          authView.PASS_INPUT, 'text', 's', 'P4ass0rd!!!');
      await vpn.waitForElementProperty(authView.SIGNIN_BUTTON, 'enabled', 'true');
      await vpn.waitForElementProperty(
          authView.PASS_INPUT, 'hasError', 'false');
      await vpn.clickOnElement(authView.SIGNIN_BUTTON);
      await vpn.waitForElementProperty(
          authView.PASS_INPUT, 'hasError', 'true');

      // Step 5: sign-in -> email code -> wrong code
      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          sessionToken: '',
          verified: false,
          verificationMethod: 'email-otp'
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;
      };
      await vpn.waitForElement(authView.PASS_INPUT);
      await vpn.waitForElementProperty(
          authView.PASS_INPUT, 'visible', 'true');
      await vpn.setElementProperty(
          authView.PASS_INPUT, 'text', 's', 'P4ass0rd!!!!');

      this.ctx.fxaEmailCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify_code'].status =
            400;
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify_code'].body = {
          errno: 107,
          validation: {keys: ['code']}
        }
      };

      await vpn.waitForElementProperty(authView.SIGNIN_BUTTON, 'enabled', 'true');
      await vpn.clickOnElement(authView.SIGNIN_BUTTON);

      await vpn.waitForElement(
          authView.verificationByEmailView.TEXT_INPUT);
      await vpn.waitForElementProperty(
          authView.verificationByEmailView.TEXT_INPUT, 'visible', 'true');
      await vpn.waitForElementProperty(
          authView.verificationByEmailView.EMAIL_BUTTON, 'enabled', 'false');
      await vpn.setElementProperty(
          authView.verificationByEmailView.TEXT_INPUT, 'text', 's',
          '123456');
      await vpn.waitForElementProperty(
          authView.verificationByEmailView.EMAIL_BUTTON, 'enabled', 'true');

      // Step 6: email code -> back -> start -> totp -> error code
      await vpn.waitForElement(
          authView.verificationByEmailView.BACK);
      await vpn.waitForElementProperty(
          authView.verificationByEmailView.BACK, 'visible', 'true');

      await vpn.clickOnElement(
          authView.verificationByEmailView.BACK);
      await vpn.waitForElement(authView.EMAIL_INPUT);
      await vpn.setElementProperty(
          authView.EMAIL_INPUT, 'text', 's', 'test@test');
      await vpn.waitForElementProperty(authView.START_BUTTON, 'enabled', 'true');
      await vpn.waitForElement(authView.START_BUTTON);
      await vpn.clickOnElement(authView.START_BUTTON);

      await vpn.waitForElement(authView.PASS_INPUT);
      await vpn.waitForElementProperty(
          authView.PASS_INPUT, 'visible', 'true');
      await vpn.setElementProperty(
          authView.PASS_INPUT, 'text', 's', 'P4ass0rd!!');

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

      await vpn.waitForElementProperty(authView.SIGNIN_BUTTON, 'enabled', 'true');
      await vpn.clickOnElement(authView.SIGNIN_BUTTON);

      await vpn.waitForElement(authView.verificationBySessionTotpView.TEXT_INPUT);
      await vpn.waitForElementProperty(
          authView.verificationBySessionTotpView.TEXT_INPUT, 'visible', 'true');
      await vpn.waitForElementProperty(
          authView.verificationBySessionTotpView.TOTP_NEEDED_BUTTON, 'enabled', 'false');
      await vpn.setElementProperty(
          authView.verificationBySessionTotpView.TEXT_INPUT, 'text', 's',
          '123456');
      await vpn.waitForElementProperty(
          authView.verificationBySessionTotpView.TOTP_NEEDED_BUTTON, 'enabled', 'true');

      await vpn.waitForElementProperty(
          authView.verificationBySessionTotpView.TEXT_INPUT, 'hasError', 'false');
      await vpn.waitForElement(authView.verificationBySessionTotpView.TOTP_NEEDED_BUTTON);
      await vpn.clickOnElement(authView.verificationBySessionTotpView.TOTP_NEEDED_BUTTON);
      await vpn.waitForElementProperty(
          authView.verificationBySessionTotpView.TEXT_INPUT, 'hasError', 'true');

      await vpn.waitForElement(
          authView.verificationBySessionTotpView.BACK);
      await vpn.waitForElementProperty(
          authView.verificationBySessionTotpView.BACK, 'visible', 'true');
      await vpn.clickOnElement(
          authView.verificationBySessionTotpView.BACK);
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

      await vpn.clickOnElement(initialView.GET_STARTED);
      await vpn.waitForElement(authView.EMAIL_INPUT);
      await vpn.waitForElementProperty(
          authView.EMAIL_INPUT, 'visible', 'true');
      await vpn.setElementProperty(
          authView.EMAIL_INPUT, 'text', 's', 'test@test');
      await vpn.waitForElement(authView.START_BUTTON);
      await vpn.clickOnElement(authView.START_BUTTON);

      await vpn.waitForElement(authView.PASS_INPUT);
      await vpn.waitForElementProperty(
          authView.PASS_INPUT, 'visible', 'true');
      await vpn.setElementProperty(
          authView.PASS_INPUT, 'text', 's', 'P4ass0rd!!');

      await vpn.waitForElementProperty(authView.SIGNIN_BUTTON, 'enabled', 'true');
      await vpn.clickOnElement(authView.SIGNIN_BUTTON);

      await vpn.waitForElement(authView.authUnblockCodeView.CODE_INPUT);
      await vpn.waitForElementProperty(
          authView.authUnblockCodeView.CODE_INPUT, 'visible', 'true');
      await vpn.setElementProperty(
          authView.authUnblockCodeView.CODE_INPUT, 'text', 's', '12345678');
      await vpn.waitForElementProperty(
          authView.authUnblockCodeView.CODE_INPUT, 'hasError', 'false');
      await vpn.waitForElement(authView.authUnblockCodeView.CODE_BUTTON);
      await vpn.clickOnElement(authView.authUnblockCodeView.CODE_BUTTON);
      await vpn.waitForElementProperty(
          authView.authUnblockCodeView.CODE_INPUT, 'hasError', 'true');
    });
  });
});
