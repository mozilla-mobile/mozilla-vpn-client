/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');  // TODO: add asserts to each block
const vpn = require('./helper.js');
const queries = require('./queries.js');
const fxaEndpoints = require('./servers/fxa_endpoints.js')

describe('User authentication', function() {
  this.timeout(300000);

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

      await vpn.waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

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

      await vpn.waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

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

      await vpn.waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

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

      await vpn.waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(
          queries.screenPostAuthentication.BUTTON.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(queries.screenTelemetry.BUTTON.visible());

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

      await vpn.waitForQuery(queries.screenAuthenticationInApp.AUTH_ERROR_POPUP_BUTTON.visible());
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

    it('Account creation with stub account', async () => {
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

      await vpn.waitForQuery(queries.screenAuthenticationInApp.AUTH_ERROR_POPUP_BUTTON.visible());
    });
  });

  describe('auth in app related telemetry tests', () => {
    if(vpn.runningOnWasm()) {
        // No Glean on WASM.
        return;
    }

    // Telemetry design is detailed at:
    // https://miro.com/app/board/uXjVM0BZcnc=/?share_link_id=228137467679

    const goToAuthStartScreen = async () => {
        await vpn.waitForInitialView();
        await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
        await vpn.waitForQuery(
            queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
        await vpn.waitForQuery(
            queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
                .disabled());

        // Stop for just a tad, to be sure the telemetry APIs get called.
        await vpn.wait();
    };

    const goToPasswordScreen = async (signUp = true) => {
        await goToAuthStartScreen();
        await vpn.setQueryProperty(
            queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
            'text', 'test@test.com');
        await vpn.waitForQueryAndClick(
            queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
                .enabled());
        await vpn.waitForQuery(queries.screenAuthenticationInApp
                                   [`AUTH_${signUp ? "SIGNUP" : "SIGNIN"}_BUTTON`].visible());

        // Stop for just a tad, to be sure the telemetry APIs get called.
        await vpn.wait();
    };

    const goToVerificationScreen = async (queriesFamily) => {
        await goToPasswordScreen();
        await vpn.waitForQueryAndWriteInTextField(
            queries.screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT.visible(),
            'P4ass0rd!!'
        );
        await vpn.waitForQueryAndClick(
            queries.screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible());
        await vpn.waitForQuery(
            queries.screenAuthenticationInApp[`${queriesFamily}_BUTTON`].visible()
                .disabled());

        // Stop for just a tad, to be sure the telemetry APIs get called.
        await vpn.wait();
    }

    describe("authentication started screen", async () => {
        const screen = "enter_email";

        beforeEach(async () => {
            await goToAuthStartScreen();
        });

        it("impression event is recorded", async () => {
            const  enterEmailViewEvent = await vpn.gleanTestGetValue("impression", "enterEmailScreen", "main");
            assert.strictEqual(enterEmailViewEvent.length, 1)
            const enterEmailViewEventExtras = enterEmailViewEvent[0].extra;
            assert.strictEqual(screen, enterEmailViewEventExtras.screen);
        });

        it("back button event is recorded", async () => {
            // Click the "<-" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_START_BACK_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "backSelected",
                screen,
            });
        });

        it("get help event is recorded", async () => {
            // Click the "Get help" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_START_GET_HELP_LINK.visible());
            await vpn.testLastInteractionEvent({
                eventName: "getHelpSelected",
                screen
            });
        });

        it("continue button event is recorded", async () => {
            // Fill out the email form to enable "Continue" button
            await vpn.waitForQueryAndWriteInTextField(
                queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
                'test@test.com'
            );
            // Click the now enabled "Continue" button
            await vpn.waitForQueryAndClick(
                queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
                    .enabled());
            await vpn.testLastInteractionEvent({
                eventName: "continueSelected",
                screen
            });
        });
    });

    describe("sign in screen", async () => {
        const screen = "enter_password";

        beforeEach(async () => {
            await goToPasswordScreen(false);
        });

        it("impression event is are recorded", async () => {
            const  enterPasswordViewEvent = await vpn.gleanTestGetValue("impression", "enterPasswordScreen", "main");
            assert.strictEqual(enterPasswordViewEvent.length, 1)
            const enterPasswordViewEventExtras = enterPasswordViewEvent[0].extra;
            assert.strictEqual(screen, enterPasswordViewEventExtras.screen);
        });

        it("back button event is recorded", async () => {
            // Click the "<-" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_SIGNIN_BACK_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "backSelected",
                screen
            });
        });

        it("get help event is recorded", async () => {
            // Click the "Get help" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_SIGNIN_GET_HELP_LINK.visible());
            await vpn.testLastInteractionEvent({
                eventName: "getHelpSelected",
                screen
            });
        });

        it("paste button event is recorded", async () => {
            // Click the "Paste" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_PASTE_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "pastePasswordSelected",
                screen
            });
        });

        it("change email button event is recorded", async () => {
            // Click the "Change email" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_SIGNIN_CHANGE_EMAIL_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "changeEmailSelected",
                screen
            });
        });

        it("forgot password button event is recorded", async () => {
            // Click the "Forgot your password?" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_SIGNIN_FORGOT_PASSWORD_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "forgotYourPasswordSelected",
                screen
            });
        });

        it("cancel event is recorded", async () => {
            // Click the "Cancel" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_SIGNIN_CANCEL_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "cancelSelected",
                screen
            });
        });

        // Leaving this for documentation more than anything.
        // The reason why it's skipped is because we can't query the "Terms of Service"
        // or the "Privacy Policy" links, because they are not actual QML components.
        // They are HTML tags inside of a QML text component???
        it.skip("legal disclaimer events are recorded", async () => {
            // Click the "Terms of Service" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_TERMS_OF_SERVICE.visible());
            await vpn.testLastInteractionEvent({
                eventName: "termsOfServiceSelected",
                screen
            });

            // Click the "Privacy Notice" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_PRIVACY_NOTICE.visible());
            await vpn.testLastInteractionEvent({
                eventName: "privacyNoticeSelected",
                screen
            });
        });

        it("signin button event and outcome event are recorded", async () => {
            // Fill out the email form to enable "Sign in" button
            await vpn.waitForQueryAndWriteInTextField(
                queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible(),
                'P4ass0rd!!'
            );
            // Click the now enabled "Sign in" button
            await vpn.waitForQueryAndClick(
                queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
                    .enabled());
            await vpn.testLastInteractionEvent({
                eventName: "signInSelected",
                screen
            });

            const startedOutcomeEvent = await vpn.gleanTestGetValue("outcome", "loginStarted", "main");
            assert.strictEqual(startedOutcomeEvent.length, 1);

            await vpn.waitForMozillaProperty(
                'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

            const endedOutcomeEvent = await vpn.gleanTestGetValue("outcome", "loginEnded", "main");
            assert.strictEqual(endedOutcomeEvent.length, 1);

            // Make sure 2fa outcome event were NOT recorded
            const unwantedEvents = [
                ...(await vpn.gleanTestGetValue("outcome", "twoFaVerificationFailed", "main")),
                ...(await vpn.gleanTestGetValue("outcome", "twoFaVerificationSucceeded", "main"))
            ];
            assert.strictEqual(unwantedEvents.length, 0);
        });
    });

    describe("unblock code screen", async () => {
        const screen = "enter_unblock_code";

        before(() => {
            this.ctx.fxaOverrideEndpoints = {
                GETs: {},
                POSTs: {
                    '/v1/account/status': {
                        status: 400,
                        bodyValidator: fxaEndpoints.validators.fxaStatus,
                        body: {errno: 107, validation: {keys: ['unblockCode']}}
                    },
                    '/v1/account/login/send_unblock_code': {
                        status: 200,
                        body: {}
                    },
                    '/v1/account/login': {}
                },
                DELETEs: {},
            };
        });

        beforeEach(async () => {
            await goToAuthStartScreen();
            await vpn.setQueryProperty(
                queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
                'text', 'test@test.com');
            await vpn.waitForQueryAndClick(
                queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
                    .enabled());
            await vpn.waitForQuery(
                queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_BUTTON.visible());
        });

        it("impression event is recorded", async () => {
            const  unblockCodeViewEvent = await vpn.gleanTestGetValue("impression", "enterUnblockCodeScreen", "main");
            assert.strictEqual(unblockCodeViewEvent.length, 1)
            const unblockCodeViewEventExtras = unblockCodeViewEvent[0].extra;
            assert.strictEqual(screen, unblockCodeViewEventExtras.screen);
        });

        it("close button event is recorded", async () => {
            // Click the "Close" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_BACK_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "closeSelected",
                screen
            });
        });

        it("get help event is recorded", async () => {
            // Click the "Get help" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_GET_HELP_LINK.visible());
            await vpn.testLastInteractionEvent({
                eventName: "getHelpSelected",
                screen
            });
        });

        it("cancel event is recorded", async () => {
            // Click the "Cancel" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_CANCEL_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "cancelSelected",
                screen
            });
        });

        it("paste button event is recorded", async () => {
            // Click the "Paste" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_PASTE_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "pastePasswordSelected",
                screen
            });
        });

        it("resend code button event is recorded", async () => {
            // Click the "Resend Code" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_RESEND_CODE_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "resendCodeSelected",
                screen
            });
        });

        // Leaving this for documentation more than anything.
        // The reason why it's skipped is because we can't query the "Terms of Service"
        // or the "Privacy Policy" links, because they are not actual QML components.
        // They are HTML tags inside of a QML text component???
        it.skip("legal disclaimer events are recorded", async () => {
            // Click the "Terms of Service" button
            // Click the "Terms of Service" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_TERMS_OF_SERVICE.visible());
            await vpn.testLastInteractionEvent({
                eventName: "termsOfServiceSelected",
                screen
            });

            // Click the "Privacy Notice" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_PRIVACY_NOTICE.visible());
            await vpn.testLastInteractionEvent({
                eventName: "privacyNoticeSelected",
                screen
            });
        });

        it("verify button events are recorded", async () => {
            // Let's start with a failing verification
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 400;
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {errno: 107, validation: {keys: ['unblockCode']}}

            // Fill out the code form to enable "Verify" button
            await vpn.waitForQueryAndWriteInTextField(
                queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_TEXT_INPUT.visible(),
                '87654321'
            );
            // Click the now enabled "Verify" button
            await vpn.waitForQueryAndClick(
                queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_BUTTON.visible()
                    .enabled());
            await vpn.testLastInteractionEvent({
                eventName: "verifySelected",
                elementId: "verify",
                screen
            });

            // First a failing outcome
            let failedOutcomeEvent;
            await vpn.waitForCondition(async () => {
                failedOutcomeEvent = await vpn.gleanTestGetValue("outcome", "twoFaVerificationFailed", "main");
                return failedOutcomeEvent.length == 1;
            });
            const failedOutcomeExtras = failedOutcomeEvent[0].extra;
            assert.strictEqual("unblock_code", failedOutcomeExtras.type);

            // Prepare mocks for a successfull verification
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;

            // Clear the verification code from the form
            await vpn.waitForQueryAndWriteInTextField(
                queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_TEXT_INPUT.visible(),
                ''
            );
            // Fill out the code form to enable "Verify" button
            await vpn.waitForQueryAndWriteInTextField(
                queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_TEXT_INPUT.visible(),
                '12345678'
            );
            // Click the now enabled "Verify" button
            await vpn.waitForQueryAndClick(
                queries.screenAuthenticationInApp.AUTH_UNBLOCKCODE_BUTTON.visible()
                    .enabled());
            await vpn.testLastInteractionEvent({
                eventName: "verifySelected",
                screen
            });

            // Now the successfull outcome
            const successOutcomeEvent = await vpn.gleanTestGetValue("outcome", "twoFaVerificationSucceeded", "main");
            assert.strictEqual(successOutcomeEvent.length, 1);
            const successOutcomeExtras = successOutcomeEvent[0].extra;
            assert.strictEqual("unblock_code", successOutcomeExtras.type);
        });
    });

    describe("create password screen", async () => {
        const screen = "create_password";

        before(() => {
            // Override endpoints to always redirect to create account screen.
            this.ctx.fxaOverrideEndpoints = {
                GETs: {},
                POSTs: {
                    '/v1/account/status': {
                        status: 200,
                        bodyValidator: fxaEndpoints.validators.fxaStatus,
                        body: { exists: false }
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
        })

        beforeEach(async () => {
            await goToPasswordScreen();
        });

        it("impression event is recorded", async () => {
            const  createPasswordViewEvent = await vpn.gleanTestGetValue("impression", "createPasswordScreen", "main");
            assert.strictEqual(createPasswordViewEvent.length, 1)
            const createPasswordViewEventExtras = createPasswordViewEvent[0].extra;
            assert.strictEqual(screen, createPasswordViewEventExtras.screen);
        });

        it("back button event is recorded", async () => {
            // Click the "<-" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_SIGNUP_BACK_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "backSelected",
                screen
            });
        });

        it("get help event is recorded", async () => {
            // Click the "Get help" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_SIGNUP_GET_HELP_LINK.visible());
            await vpn.testLastInteractionEvent({
                eventName: "getHelpSelected",
                screen
            });
        });

        it("cancel event is recorded", async () => {
            // Click the "Cancel" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_SIGNUP_CANCEL_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "cancelSelected",
                screen
            });
        });

        // Leaving this for documentation more than anything.
        // The reason why it's skipped is because we can't query the "Terms of Service"
        // or the "Privacy Policy" links, because they are not actual QML components.
        // They are HTML tags inside of a QML text component???
        it.skip("legal disclaimer events are recorded", async () => {
            // Click the "Terms of Service" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_TERMS_OF_SERVICE.visible());
            await vpn.testLastInteractionEvent({
                eventName: "termsOfServiceSelected",
                screen
            });

            // Click the "Privacy Notice" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_PRIVACY_NOTICE.visible());
            await vpn.testLastInteractionEvent({
                eventName: "privacyNoticeSelected",
                screen
            });
        });

        it("create account button event and outcome event are recorded", async () => {
            // Fill out the password form to enable "Create account" button
            await vpn.waitForQueryAndWriteInTextField(
                queries.screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT.visible(),
                'P4ass0rd!!'
            );
            // Click the now enabled "Create account" button
            await vpn.waitForQueryAndClick(
                queries.screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
                    .enabled());
            // Two events are exected here,
            // because of the pressing of the "Continue" button in the start screen
            await vpn.testLastInteractionEvent({
                eventName: "createAccountSelected",
                screen
            });

            const startedEvent = await vpn.gleanTestGetValue("outcome", "registrationStarted", "main");
            assert.strictEqual(startedEvent.length, 1);

            await vpn.waitForMozillaProperty(
                'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

            const completedOutcomeEvent = await vpn.gleanTestGetValue("outcome", "registrationCompleted", "main");
            assert.strictEqual(completedOutcomeEvent.length, 1);

             // Make sure no 2fa outcome event was NOT recorded
             const unwantedEvents = [
                ...(await vpn.gleanTestGetValue("outcome", "twoFaVerificationFailed", "main")),
                ...(await vpn.gleanTestGetValue("outcome", "twoFaVerificationSucceeded", "main"))
            ];
            assert.strictEqual(unwantedEvents.length, 0);
        });
    });

    describe("email verification screen", async () => {
        const screen = "enter_verification_code";

        before(() => {
            // Override endpoints to always redirect to always lead to the verification screen.
            this.ctx.fxaOverrideEndpoints = {
                GETs: {},
                POSTs: {
                    '/v1/account/status': {
                        status: 200,
                        bodyValidator: fxaEndpoints.validators.fxaStatus,
                        body: { exists: false }
                    },
                    '/v1/account/create': {
                        status: 200,
                        bodyValidator: fxaEndpoints.validators.fxaCreate,
                        body: {
                        sessionToken: 'sessionToken',
                        verified: false,
                        verificationMethod: 'email-otp'
                        }
                    },
                    '/v1/session/resend_code': {
                        status: 200,
                        body: {}
                    },
                    // Will set it when necessary.
                    '/v1/session/verify_code': {}
                },
                DELETEs: {},
            };
        });

        beforeEach(async () => {
            await goToVerificationScreen("AUTH_EMAILVER");
        });

        it('impression event is recorded', async () => {
          const verificationViewEvent = await vpn.gleanTestGetValue(
              "impression", "enterVerificationCodeScreen", "main");
          assert.strictEqual(verificationViewEvent.length, 1)
          const verificationViewEventExtras = verificationViewEvent[0].extra;
          assert.strictEqual(screen, verificationViewEventExtras.screen);
        });

        it("get help event is recorded", async () => {
            // Click the "Get help" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_EMAILVER_GET_HELP_LINK.visible());
            await vpn.testLastInteractionEvent({
                eventName: "getHelpSelected",
                screen
            });
        });

        it("cancel event is recorded", async () => {
            // Click the "Cancel" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_EMAILVER_CANCEL_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "cancelSelected",
                screen
            });
        });

        it("paste button event is recorded", async () => {
            // Click the "Paste" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_EMAILVER_PASTE_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "pastePasswordSelected",
                screen
            });
        });

        it("resend code button event is recorded", async () => {
            // Click the "Resend Code" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_EMAILVER_RESEND_CODE_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "resendCodeSelected",
                screen
            });
        });

        // Leaving this for documentation more than anything.
        // The reason why it's skipped is because we can't query the "Terms of Service"
        // or the "Privacy Policy" links, because they are not actual QML components.
        // They are HTML tags inside of a QML text component???
        it.skip("legal disclaimer events are recorded", async () => {
            // Click the "Terms of Service" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_TERMS_OF_SERVICE.visible());
            await vpn.testLastInteractionEvent({
                eventName: "termsOfServiceSelected",
                screen
            });

            // Click the "Privacy Notice" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_PRIVACY_NOTICE.visible());
            await vpn.testLastInteractionEvent({
                eventName: "privacyNoticeSelected",
                screen
            });
        });

        it("verify button events are recorded", async () => {
            // Let's start with a failing verification
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify_code'].status = 400;
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify_code'].body = {
                errno: 107,
                validation: { keys: ['code'] }
            }
            // Fill out the code form to enable "Verify" button
            await vpn.waitForQueryAndWriteInTextField(
                queries.screenAuthenticationInApp.AUTH_EMAILVER_TEXT_INPUT.visible(),
                '654321'
            );
            // Click the now enabled "Verify" button
            await vpn.waitForQueryAndClick(
                queries.screenAuthenticationInApp.AUTH_EMAILVER_BUTTON.visible()
                    .enabled());
            await vpn.testLastInteractionEvent({
                eventName: "verifySelected",
                screen
            });

            // First a failing outcome
            let failedOutcomeEvent;
            await vpn.waitForCondition(async () => {
                failedOutcomeEvent = await vpn.gleanTestGetValue("outcome", "twoFaVerificationFailed", "main");
                return failedOutcomeEvent.length == 1;
            });
            const failedOutcomeExtras = failedOutcomeEvent[0].extra;
            assert.strictEqual("email", failedOutcomeExtras.type);

            // Prepare mocks for a successfull verification
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify_code'].status = 200;
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify_code'].body = {};

            // Clear the verification code from the form
            await vpn.waitForQueryAndWriteInTextField(
                queries.screenAuthenticationInApp.AUTH_EMAILVER_TEXT_INPUT.visible(),
                ''
            );
            // Fill out the code form to enable "Verify" button
            await vpn.waitForQueryAndWriteInTextField(
                queries.screenAuthenticationInApp.AUTH_EMAILVER_TEXT_INPUT.visible(),
                '123456'
            );
            // Click the now enabled "Verify" button
            await vpn.waitForQueryAndClick(
                queries.screenAuthenticationInApp.AUTH_EMAILVER_BUTTON.visible()
                    .enabled());
            await vpn.testLastInteractionEvent({
                eventName: "verifySelected",
                screen
            });

            // Now the successfull outcome
            const successOutcomeEvent = await vpn.gleanTestGetValue("outcome", "twoFaVerificationSucceeded", "main");
            assert.strictEqual(successOutcomeEvent.length, 1);
            const successOutcomeExtras = successOutcomeEvent[0].extra;
            assert.strictEqual("email", successOutcomeExtras.type);
        });
    });

    describe("TOTP verification screen", async () => {
        const screen = "enter_security_code";

        before(() => {
            // Override endpoints to always redirect to always lead to the verification screen.
            this.ctx.fxaOverrideEndpoints = {
                GETs: {},
                POSTs: {
                    '/v1/account/status': {
                        status: 200,
                        bodyValidator: fxaEndpoints.validators.fxaStatus,
                        body: { exists: false }
                    },
                    '/v1/account/create': {
                        status: 200,
                        bodyValidator: fxaEndpoints.validators.fxaCreate,
                        body: {
                        sessionToken: 'sessionToken',
                        verified: false,
                        verificationMethod: 'totp-2fa'
                        }
                    },
                    // Will set it when necessary.
                    '/v1/session/verify/totp': {}
                },
                DELETEs: {},
            };
        });

        beforeEach(async () => {
            await goToVerificationScreen("AUTH_TOTP");
        });

        it("impression event is recorded", async () => {
            const  verificationViewEvent = await vpn.gleanTestGetValue("impression", "enterSecurityCodeScreen", "main");
            assert.strictEqual(verificationViewEvent.length, 1)
            const verificationViewEventExtras = verificationViewEvent[0].extra;
            assert.strictEqual(screen, verificationViewEventExtras.screen);
        });

        it("get help event is recorded", async () => {
            // Click the "Get help" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_TOTP_GET_HELP_LINK.visible());
            await vpn.testLastInteractionEvent({
                eventName: "getHelpSelected",
                screen
            });
        });

        it("cancel event is recorded", async () => {
            // Click the "Cancel" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_TOTP_CANCEL_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "cancelSelected",
                screen
            });
        });

        it("paste button event is recorded", async () => {
            // Click the "Paste" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_TOTP_PASTE_BUTTON.visible());
            await vpn.testLastInteractionEvent({
                eventName: "pastePasswordSelected",
                screen
            });
        });

        // Leaving this for documentation more than anything.
        // The reason why it's skipped is because we can't query the "Terms of Service"
        // or the "Privacy Policy" links, because they are not actual QML components.
        // They are HTML tags inside of a QML text component???
        it.skip("legal disclaimer events are recorded", async () => {
            // Click the "Terms of Service" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_TERMS_OF_SERVICE.visible());
            await vpn.testLastInteractionEvent({
                eventName: "termsOfServiceSelected",
                screen
            });

            // Click the "Privacy Notice" button
            await vpn.waitForQueryAndClick(queries.screenAuthenticationInApp.AUTH_PRIVACY_NOTICE.visible());
            await vpn.testLastInteractionEvent({
                eventName: "privacyNoticeSelected",
                screen
            });
        });

        it("TOTP verify button events are recorded", async () => {
            // Let's start with a failing verification
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify/totp'].status = 200;
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify/totp'].body = {
                success: false
            };

            // Fill out the code form to enable "Verify" button
            await vpn.waitForQueryAndWriteInTextField(
                queries.screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible(),
                '654321'
            );
            // Click the now enabled "Verify" button
            await vpn.waitForQueryAndClick(
                queries.screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
                    .enabled());
            await vpn.testLastInteractionEvent({
                eventName: "verifySelected",
                screen
            });

            // First a failing outcome
            let failedOutcomeEvent;
            await vpn.waitForCondition(async () => {
                failedOutcomeEvent = await vpn.gleanTestGetValue("outcome", "twoFaVerificationFailed", "main");
                return failedOutcomeEvent.length == 1;
            });
            const failedOutcomeExtras = failedOutcomeEvent[0].extra;
            assert.strictEqual("totp", failedOutcomeExtras.type);

            // Prepare mocks for a successfull verification
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify/totp'].status = 200;
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify/totp'].body = {
                success: true
            };

            // Clear the verification code from the form
            await vpn.waitForQueryAndWriteInTextField(
                queries.screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible(),
                ''
            );
            // Fill out the code form to enable "Verify" button
            await vpn.waitForQueryAndWriteInTextField(
                queries.screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible(),
                '123456'
            );
            // Click the now enabled "Verify" button
            await vpn.waitForQueryAndClick(
                queries.screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
                    .enabled());
            await vpn.testLastInteractionEvent({
                eventName: "verifySelected",
                screen
            });

            // Now the successfull outcome
            const successOutcomeEvent = await vpn.gleanTestGetValue("outcome", "twoFaVerificationSucceeded", "main");
            assert.strictEqual(successOutcomeEvent.length, 1);
            const successOutcomeExtras = successOutcomeEvent[0].extra;
            assert.strictEqual("totp", successOutcomeExtras.type);
        });
    });
  });
});
