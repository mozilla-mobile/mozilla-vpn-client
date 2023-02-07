/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');  // TODO: add asserts to each block
const vpn = require('./helper.js');
const queries = require('./queries.js');
const fxaEndpoints = require('./servers/fxa_endpoints.js')

describe('User authentication', function() {
  this.timeout(300000);

  it('Completes authentication int app', async () => {
    await vpn.authenticateInApp();
  });

  describe('Account creation', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 200,
          bodyValidator: fxaEndpoints.validators.fxaStatus,
          body: {exists: false}
        },

        '/v1/account/create': {
          status: 200,
          bodyValidator: fxaEndpoints.validators.fxaCreate,
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

      await vpn.waitForInitialView();

      await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .disabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_SIGNUP_PASSWORD_INPUT.visible());

      // Condition 1
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT
              .visible(),
          'text', '12345');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp
              .AUTH_SIGNUP_PASSWORD_LENGTH_CONDITION.visible()
              .prop('_passwordConditionIsSatisfied', false));
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_EMAIL_CONDITION
              .visible()
              .prop('_passwordConditionIsSatisfied', false));
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp
              .AUTH_SIGNUP_PASSWORD_COMMON_CONDITION.visible()
              .prop('_passwordConditionIsSatisfied', false));

      // Condition 2
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT
              .visible(),
          'text', 'test@test.com');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp
              .AUTH_SIGNUP_PASSWORD_LENGTH_CONDITION.visible()
              .prop('_passwordConditionIsSatisfied', true));
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_EMAIL_CONDITION
              .visible()
              .prop('_passwordConditionIsSatisfied', false));
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp
              .AUTH_SIGNUP_PASSWORD_COMMON_CONDITION.visible()
              .prop('_passwordConditionIsSatisfied', true));

      // Condition 3
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT
              .visible(),
          'text', '12345678');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp
              .AUTH_SIGNUP_PASSWORD_LENGTH_CONDITION.visible()
              .prop('_passwordConditionIsSatisfied', true));
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_EMAIL_CONDITION
              .visible()
              .prop('_passwordConditionIsSatisfied', true));
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp
              .AUTH_SIGNUP_PASSWORD_COMMON_CONDITION.visible()
              .prop('_passwordConditionIsSatisfied', false));

      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
              .disabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT
              .visible(),
          'text', 'P4ass0rd!!');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
              .enabled());

      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT
              .visible(),
          'text', '');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
              .disabled());
      await vpn.copyToClipboard('P4ass0rd!!');
      await vpn.clickOnQuery(queries.screenAuthenticationInApp
                                 .AUTH_SIGNUP_PASSWORD_PASTE_BUTTON.visible());

      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
              .enabled());

      await vpn.waitForVPNProperty('VPN', 'userState', 'UserAuthenticated');

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(
          queries.screenPostAuthentication.BUTTON.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(queries.screenTelemetry.BUTTON.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    });
  });

  describe('Account creation with email verification', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 200,
          bodyValidator: fxaEndpoints.validators.fxaStatus,
          body: {exists: false}
        },

        '/v1/account/create': {
          status: 200,
          bodyValidator: fxaEndpoints.validators.fxaCreate,
          body: {
            sessionToken: 'sessionToken',
            'verified': false,
            verificationMethod: 'email-otp'
          }
        },

        '/v1/session/verify_code': {
          status: 200,
          requiredHeaders: ['Authorization'],
          bodyValidator: fxaEndpoints.validators.fxaVerifyCode,
          body: {}
        },
      },
      DELETEs: {},
    };

    it('Account creation with email verification', async () => {
      if (!(await vpn.isFeatureFlippedOn('inAppAuthentication'))) {
        await vpn.flipFeatureOn('inAppAuthentication');
        await vpn.flipFeatureOn('inAppAccountCreate');
      }

      await vpn.waitForInitialView();

      await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .disabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_SIGNUP_PASSWORD_INPUT.visible());

      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
              .disabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT
              .visible(),
          'text', 'P4ass0rd!!');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_EMAILVER_TEXT_INPUT.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_EMAILVER_BUTTON.visible()
              .disabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_EMAILVER_TEXT_INPUT.visible(),
          'text', '12345');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_EMAILVER_BUTTON.visible()
              .disabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_EMAILVER_TEXT_INPUT.visible(),
          'text', '123456');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_EMAILVER_BUTTON.visible()
              .enabled());

      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_EMAILVER_TEXT_INPUT.visible(),
          'text', '');
      await vpn.copyToClipboard('654321');
      await vpn.clickOnQuery(queries.screenAuthenticationInApp
                                 .AUTH_EMAILVER_PASTE_BUTTON.visible());

      await vpn.clickOnQuery(
          queries.screenAuthenticationInApp.AUTH_EMAILVER_BUTTON.visible()
              .enabled());

      await vpn.waitForVPNProperty('VPN', 'userState', 'UserAuthenticated');

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(
          queries.screenPostAuthentication.BUTTON.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(queries.screenTelemetry.BUTTON.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    });
  });

  describe('Account creation with TOTP', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 200,
          bodyValidator: fxaEndpoints.validators.fxaStatus,
          body: {exists: false}
        },

        '/v1/account/create': {
          status: 200,
          bodyValidator: fxaEndpoints.validators.fxaCreate,
          body: {
            sessionToken: 'sessionToken',
            'verified': false,
            verificationMethod: 'totp-2fa'
          }
        },

        '/v1/session/verify/totp': {
          status: 200,
          requiredHeaders: ['Authorization'],
          bodyValidator: fxaEndpoints.validators.fxaVerifyTotp,
          body: {success: true}
        },
      },
      DELETEs: {},
    };

    it('Account creation with TOTP', async () => {
      if (!(await vpn.isFeatureFlippedOn('inAppAuthentication'))) {
        await vpn.flipFeatureOn('inAppAuthentication');
        await vpn.flipFeatureOn('inAppAccountCreate');
      }

      await vpn.waitForInitialView();

      await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .disabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_SIGNUP_PASSWORD_INPUT.visible());

      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
              .disabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT
              .visible(),
          'text', 'P4ass0rd!!');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
              .disabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible(),
          'text', '12345');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
              .disabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible(),
          'text', '123456');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
              .enabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible(),
          'text', '');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
              .disabled());
      await vpn.copyToClipboard('123456');
      await vpn.clickOnQuery(
          queries.screenAuthenticationInApp.AUTH_TOTP_PASTE_BUTTON.visible());
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
              .enabled());

      await vpn.waitForVPNProperty('VPN', 'userState', 'UserAuthenticated');

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(
          queries.screenPostAuthentication.BUTTON.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(queries.screenTelemetry.BUTTON.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    });
  });

  describe('Authentication with unblock code', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 400,
          bodyValidator: fxaEndpoints.validators.fxaStatus,
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

      await vpn.waitForInitialView();

      await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .disabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_UNBLOCKCODE_TEXT_INPUT.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_BUTTON.visible()
              .disabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_TEXT_INPUT
              .visible(),
          'text', '1234567');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_BUTTON.visible()
              .disabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_TEXT_INPUT
              .visible(),
          'text', '12345678');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_BUTTON.visible()
              .enabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_TEXT_INPUT
              .visible(),
          'text', '');
      await vpn.copyToClipboard("12345678");
      await vpn.clickOnQuery(queries.screenAuthenticationInApp
                                 .AUTH_UNBLOCKCODE_PASTE_BUTTON.visible());
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_BUTTON.visible()
              .enabled());

      await vpn.waitForVPNProperty('VPN', 'userState', 'UserAuthenticated');

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(
          queries.screenPostAuthentication.BUTTON.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(queries.screenTelemetry.BUTTON.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    });
  });

  describe('Account creation', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 200,
          bodyValidator: fxaEndpoints.validators.fxaStatus,
          body: null,
          callback: (req) => this.ctx.fxaStatusCallback(req)
        },

        '/v1/account/login': {
          status: 200,
          bodyValidator: fxaEndpoints.validators.fxaLogin,
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
      await vpn.waitForInitialView();
      await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BACK_BUTTON.visible());
      await vpn.waitForInitialView();

      // Step 2: main -> start -> help -> start
      await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_GET_HELP_LINK.visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON);
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());

      // Step 3: start -> sign-up -> help -> sign-up
      this.ctx.fxaStatusCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].body = {
          exists: false
        }
      };
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

      await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp
                                         .AUTH_SIGNUP_GET_HELP_LINK.visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON);
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_BACK_BUTTON.visible());

      // Step 4: sign-up -> start
      await vpn.clickOnQuery(
          queries.screenAuthenticationInApp.AUTH_SIGNUP_BACK_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());

      // Step 5: start -> sign-in -> help -> sign-in
      this.ctx.fxaStatusCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].body = {
          exists: true
        }
      };
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp
                                         .AUTH_SIGNIN_GET_HELP_LINK.visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON);
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_BACK_BUTTON.visible());

      // Step 6: sign-in -> start
      await vpn.clickOnQuery(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_BACK_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());

      // Step 7: start -> sign-in -> email verification -> help -> email
      // verification
      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          sessionToken: '',
          verified: false,
          verificationMethod: 'email-otp'
        }
      };
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_SIGNIN_PASSWORD_INPUT.visible());

      await vpn.copyToClipboard('pa$$vv0rd');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_PASTE_BUTTON
              .visible());

      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
              .enabled());

      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_EMAILVER_GET_HELP_LINK
              .visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON);
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_EMAILVER_BACK_BUTTON.visible());

      // Step 8: email verification -> start
      await vpn.clickOnQuery(queries.screenAuthenticationInApp
                                 .AUTH_EMAILVER_BACK_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());

      // Step 9: start -> sign-in -> totp -> help -> totp
      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          sessionToken: '',
          verified: false,
          verificationMethod: 'totp-2fa'
        }
      };
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_SIGNIN_PASSWORD_INPUT.visible());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT
              .visible(),
          'text', 'P4ass0rd!!');

      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
              .enabled());

      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_TOTP_GET_HELP_LINK.visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON);
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_TOTP_BACK_BUTTON.visible());

      // Step 10: totp -> main
      await vpn.clickOnQuery(
          queries.screenAuthenticationInApp.AUTH_TOTP_BACK_BUTTON.visible());
      await vpn.waitForInitialView();

      // Step 11: main -> start -> sign-in -> unblock code -> help -> unblock
      // code
      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          errno: 107,
          validation: {keys: ['unblockCode']}
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 400;
      };
      await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_SIGNIN_PASSWORD_INPUT.visible());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT
              .visible(),
          'text', 'P4ass0rd!!');

      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
              .enabled());

      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_GET_HELP_LINK
              .visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON);
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_UNBLOCKCODE_BACK_BUTTON.visible());

      // Step 12: unblock code -> start
      await vpn.clickOnQuery(queries.screenAuthenticationInApp
                                 .AUTH_UNBLOCKCODE_BACK_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
    });
  });

  describe('Error handling', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 200,
          bodyValidator: fxaEndpoints.validators.fxaStatus,
          body: null,
          callback: (req) => this.ctx.fxaStatusCallback(req)
        },

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

        '/v1/session/verify_code': {
          status: 200,
          requiredHeaders: ['Authorization'],
          bodyValidator: fxaEndpoints.validators.fxaVerifyCode,
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

      await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());

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

        await vpn.waitForQuery(
            queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
        await vpn.setQueryProperty(
            queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
            'text', 'test@test.com-' + errorCode);
        await vpn.waitForQueryAndClick(
            queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
                .enabled());

        await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp
                                           .AUTH_ERROR_POPUP_BUTTON.visible());
        await vpn.waitForQueryAndClick(
            queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
                .disabled());
      }

      // Step 2: start -> email error
      this.ctx.fxaStatusCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].body = {
          errno: 107,
          validation: {keys: ['email']}
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].status = 400;
      };

      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .disabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@testtest.com');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible()
              .prop('hasError', false));
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible()
              .prop('hasError', true));

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

      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .disabled());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible()
              .prop('hasError', true));
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible()
              .prop('hasError', false));
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_SIGNIN_PASSWORD_INPUT.visible());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT
              .visible(),
          'text', 'P4ass0rd!!');

      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
              .enabled());

      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_ERROR_POPUP_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
              .disabled());

      // Step 4: start -> sign-in -> password invalid
      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          errno: 103
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 400;
      };

      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT
              .visible(),
          'text', 'P4ass0rd!!!');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
              .enabled());
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible()
              .prop('hasError', false));
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
              .enabled());
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible()
              .prop('hasError', true));

      // Step 5: sign-in -> email code -> wrong code
      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          sessionToken: '',
          verified: false,
          verificationMethod: 'email-otp'
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;
      };

      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT
              .visible(),
          'text', 'P4ass0rd!!!!');

      this.ctx.fxaEmailCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify_code'].status =
            400;
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify_code'].body = {
          errno: 107,
          validation: {keys: ['code']}
        }
      };

      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_EMAILVER_TEXT_INPUT.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_EMAILVER_BUTTON.visible()
              .disabled());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_EMAILVER_TEXT_INPUT.visible(),
          'text', '123456');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_EMAILVER_BUTTON.visible()
              .enabled());

      // Step 6: email code -> back -> start -> totp -> error code
      await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp
                                         .AUTH_EMAILVER_BACK_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_SIGNIN_PASSWORD_INPUT.visible());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT
              .visible(),
          'text', 'P4ass0rd!!');

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
          queries.screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible().prop(
              'hasError', false));
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
              .enabled());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible().prop(
              'hasError', true));

      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_TOTP_BACK_BUTTON.visible());
      await vpn.waitForInitialView();

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

      await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_SIGNIN_PASSWORD_INPUT.visible());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT
              .visible(),
          'text', 'P4ass0rd!!');

      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_UNBLOCKCODE_TEXT_INPUT.visible());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_TEXT_INPUT
              .visible(),
          'text', '12345678');
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_TEXT_INPUT
              .visible()
              .prop('hasError', false));
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_BUTTON.visible()
              .enabled());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_TEXT_INPUT
              .visible()
              .prop('hasError', true));
    });
  });
});
