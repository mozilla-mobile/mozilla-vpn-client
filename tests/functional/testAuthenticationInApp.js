/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');  // TODO: add asserts to each block
const vpn = require('./helper.js');
const queries = require('./queries.js');
const fxaEndpoints = require('./servers/fxa_endpoints.js')

describe('User authentication', function() {
  this.timeout(300000);

  beforeEach(async () => {
    if (!(await vpn.isFeatureEnabled('inAppAuthentication'))) {
        await vpn.flipFeatureOn('inAppAuthentication');
        await vpn.flipFeatureOn('inAppAccountCreate');
      }
  });

  it('Completes authentication in app', async () => {
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

      //skip onboarding. normally done in helper::authenticateInApp(), but this test logs in manually
      await vpn.skipOnboarding();

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

      await vpn.waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userAuthenticated', 'true');

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

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
      //skip onboarding. normally done in helper::authenticateInApp(), but this test logs in manually
      await vpn.skipOnboarding();

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

      await vpn.waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userAuthenticated', 'true');

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

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
      //skip onboarding. normally done in helper::authenticateInApp(), but this test logs in manually
      await vpn.skipOnboarding();

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

      await vpn.waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userAuthenticated', 'true');

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

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
      //skip onboarding. normally done in helper::authenticateInApp(), but this test logs in manually
      await vpn.skipOnboarding();
      
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

      await vpn.waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userAuthenticated', 'true');

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    });
  });

  describe('Account creation with stub account', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 200,
          bodyValidator: fxaEndpoints.validators.fxaStatus,
          body: {exists: true, hasLinkedAccount: false, hasPassword: false}
        },
      },
      DELETEs: {},
    };

    it('Account creation with stub account', async () => {
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

      await vpn.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_STUB_SET_PASSWORD_HEADLINE.visible());

      // User goes and sets a password on FxA -- server now reports the user has a password
      this.ctx.fxaServer.overrideEndpoints.POSTs['/v1/account/status'].body = {
        exists: true, hasLinkedAccount: false, hasPassword: true
      }

      await vpn.clickOnQuery(
        queries.screenAuthenticationInApp.AUTH_STUB_SET_PASSWORD_SIGN_IN_BUTTON.visible());
      await vpn.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible());
    });
  });

  describe('Account creation with SSO account', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 200,
          bodyValidator: fxaEndpoints.validators.fxaStatus,
          body: {exists: true, hasLinkedAccount: true, hasPassword: false}
        },
      },
      DELETEs: {},
    };

    it('Account creation with SSO account', async () => {
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

      await vpn.waitForQuery(queries.screenAuthenticationInApp.AUTH_SSO_SET_PASSWORD_HEADLINE.visible());

      // User goes and sets a password on FxA -- server now reports the user has a password
      this.ctx.fxaServer.overrideEndpoints.POSTs['/v1/account/status'].body = {
        exists: true, hasLinkedAccount: false, hasPassword: true
      }

      await vpn.clickOnQuery(queries.screenAuthenticationInApp.AUTH_SSO_SET_PASSWORD_SIGN_IN_BUTTON.visible());
      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_SIGNIN_PASSWORD_INPUT.visible());
    });
  });
});
