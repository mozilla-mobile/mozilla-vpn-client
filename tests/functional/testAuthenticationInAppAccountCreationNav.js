/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');  // TODO: add asserts to each block
const vpn = require('./helper.js');
const queries = require('./queries.js');
const fxaEndpoints = require('./servers/fxa_endpoints.js')

describe('User authentication', function() {
  this.timeout(300000);

  describe('Account creation Nav', function() {
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
      await vpn.waitForQuery(queries.screenGetHelp.STACKVIEW.ready());
      await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON.visible());
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
      await vpn.waitForQuery(queries.screenGetHelp.STACKVIEW.ready());
      await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON.visible());
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
          exists: true, hasPassword: true
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
      await vpn.waitForQuery(queries.screenGetHelp.STACKVIEW.ready());
      await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON.visible());
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
      await vpn.waitForQuery(queries.screenGetHelp.STACKVIEW.ready());
      await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON.visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

      // Step 8: email verification -> start
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_EMAILVER_CANCEL_BUTTON
              .visible());
      await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
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
      await vpn.waitForQuery(queries.screenGetHelp.STACKVIEW.ready());
      await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON.visible());
      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_TOTP_CANCEL_BUTTON.visible());

      // Step 10: totp -> main
      await vpn.clickOnQuery(
          queries.screenAuthenticationInApp.AUTH_TOTP_CANCEL_BUTTON.visible());
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
      await vpn.waitForQuery(queries.screenGetHelp.STACKVIEW.ready());
      await vpn.waitForQueryAndClick(queries.screenGetHelp.BACK_BUTTON.visible());
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
});
