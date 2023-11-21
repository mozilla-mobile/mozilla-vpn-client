/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');  // TODO: add asserts to each block
const vpn = require('./helper.js');
const queries = require('./queries.js');
const fxaEndpoints = require('./servers/fxa_endpoints.js')

describe('User authentication', function() {
  this.timeout(300000);

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
          exists: true, hasPassword: true
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
          queries.screenAuthenticationInApp.AUTH_TOTP_CANCEL_BUTTON.visible());
      await vpn.waitForInitialView();

      // Step 7: main -> sign up -> code -> error

      this.ctx.fxaStatusCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].body = {
          exists: true, hasPassword: true
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
