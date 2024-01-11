/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import assert from 'assert';  // TODO: add asserts to each block
import { isFeatureFlippedOn, flipFeatureOn, waitForInitialView, clickOnQuery, waitForQuery, waitForQueryAndClick, setQueryProperty, copyToClipboard } from './helper.js';
import { screenInitialize, global, screenAuthenticationInApp, screenGetHelp } from './queries.js';
import { validators } from './servers/fxa_endpoints.js';

describe('User authentication', function() {
  this.timeout(300000);

  describe('Account creation Nav', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 200,
          bodyValidator: validators.fxaStatus,
          body: null,
          callback: (req) => this.ctx.fxaStatusCallback(req)
        },

        '/v1/account/login': {
          status: 200,
          bodyValidator: validators.fxaLogin,
          body: null,
          callback: (req) => this.ctx.fxaLoginCallback(req)
        },
      },
      DELETEs: {},
    };

    it('Back and forward', async () => {
      if (!(await isFeatureFlippedOn('inAppAuthentication'))) {
        await flipFeatureOn('inAppAuthentication');
        await flipFeatureOn('inAppAccountCreate');
      }

      // Step 1: main -> start -> main
      await waitForInitialView();
      await clickOnQuery(screenInitialize.SIGN_UP_BUTTON.visible());
      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_START_BACK_BUTTON.visible());
      await waitForInitialView();

      // Step 2: main -> start -> help -> start
      await clickOnQuery(screenInitialize.SIGN_UP_BUTTON.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_START_GET_HELP_LINK.visible());
      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQueryAndClick(screenGetHelp.BACK_BUTTON);
      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());

      // Step 3: start -> sign-up -> help -> sign-up
      this.ctx.fxaStatusCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].body = {
          exists: false
        }
      };
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());
      await waitForQuery(global.SCREEN_LOADER.ready());

      await waitForQueryAndClick(screenAuthenticationInApp
                                         .AUTH_SIGNUP_GET_HELP_LINK.visible());
      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQueryAndClick(screenGetHelp.BACK_BUTTON);
      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_SIGNUP_BACK_BUTTON.visible());

      // Step 4: sign-up -> start
      await clickOnQuery(
          screenAuthenticationInApp.AUTH_SIGNUP_BACK_BUTTON.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());

      // Step 5: start -> sign-in -> help -> sign-in
      this.ctx.fxaStatusCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/status'].body = {
          exists: true, hasPassword: true
        }
      };
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await waitForQueryAndClick(screenAuthenticationInApp
                                         .AUTH_SIGNIN_GET_HELP_LINK.visible());
      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQueryAndClick(screenGetHelp.BACK_BUTTON);
      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_SIGNIN_BACK_BUTTON.visible());

      // Step 6: sign-in -> start
      await clickOnQuery(
          screenAuthenticationInApp.AUTH_SIGNIN_BACK_BUTTON.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());

      // Step 7: start -> sign-in -> email verification -> help -> email
      // verification
      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          sessionToken: '',
          verified: false,
          verificationMethod: 'email-otp'
        }
      };
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await waitForQuery(screenAuthenticationInApp
                                 .AUTH_SIGNIN_PASSWORD_INPUT.visible());

      await copyToClipboard('pa$$vv0rd');
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_PASTE_BUTTON
              .visible());

      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
              .enabled());

      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_EMAILVER_GET_HELP_LINK
              .visible());
      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQueryAndClick(screenGetHelp.BACK_BUTTON);
      await waitForQuery(global.SCREEN_LOADER.ready());

      // Step 8: email verification -> start
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_EMAILVER_CANCEL_BUTTON
              .visible());
      await clickOnQuery(screenInitialize.SIGN_UP_BUTTON.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());

      // Step 9: start -> sign-in -> totp -> help -> totp
      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          sessionToken: '',
          verified: false,
          verificationMethod: 'totp-2fa'
        }
      };
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await waitForQuery(screenAuthenticationInApp
                                 .AUTH_SIGNIN_PASSWORD_INPUT.visible());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT
              .visible(),
          'text', 'P4ass0rd!!');

      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
              .enabled());

      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_TOTP_GET_HELP_LINK.visible());
      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQueryAndClick(screenGetHelp.BACK_BUTTON);
      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_TOTP_CANCEL_BUTTON.visible());

      // Step 10: totp -> main
      await clickOnQuery(
          screenAuthenticationInApp.AUTH_TOTP_CANCEL_BUTTON.visible());
      await waitForInitialView();

      // Step 11: main -> start -> sign-in -> unblock code -> help -> unblock
      // code
      this.ctx.fxaLoginCallback = (req) => {
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {
          errno: 107,
          validation: {keys: ['unblockCode']}
        };
        this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 400;
      };
      await clickOnQuery(screenInitialize.SIGN_UP_BUTTON.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await waitForQuery(screenAuthenticationInApp
                                 .AUTH_SIGNIN_PASSWORD_INPUT.visible());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT
              .visible(),
          'text', 'P4ass0rd!!');

      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
              .enabled());

      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_UNBLOCKCODE_GET_HELP_LINK
              .visible());
      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQueryAndClick(screenGetHelp.BACK_BUTTON);
      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQuery(screenAuthenticationInApp
                                 .AUTH_UNBLOCKCODE_BACK_BUTTON.visible());

      // Step 12: unblock code -> start
      await clickOnQuery(screenAuthenticationInApp
                                 .AUTH_UNBLOCKCODE_BACK_BUTTON.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
    });
  });
});
