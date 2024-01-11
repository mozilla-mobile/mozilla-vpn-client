/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { strictEqual } from 'assert';  // TODO: add asserts to each block
import { authenticateInApp, isFeatureFlippedOn, flipFeatureOn, waitForInitialView, clickOnQuery, waitForQuery, setQueryProperty, waitForQueryAndClick, copyToClipboard, waitForMozillaProperty, runningOnWasm, wait, waitForQueryAndWriteInTextField, gleanTestGetValue, testLastInteractionEvent, waitForCondition } from './helper.js';
import { screenInitialize, screenAuthenticationInApp, global, screenPostAuthentication, screenTelemetry, screenHome } from './queries.js';
import { validators } from './servers/fxa_endpoints.js';

describe('User authentication', function() {
  this.timeout(300000);

  it('Completes authentication in app', async () => {
    await authenticateInApp();
  });

  describe('Account creation', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 200,
          bodyValidator: validators.fxaStatus,
          body: {exists: false}
        },

        '/v1/account/create': {
          status: 200,
          bodyValidator: validators.fxaCreate,
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
      if (!(await isFeatureFlippedOn('inAppAuthentication'))) {
        await flipFeatureOn('inAppAuthentication');
        await flipFeatureOn('inAppAccountCreate');
      }

      await waitForInitialView();

      await clickOnQuery(screenInitialize.SIGN_UP_BUTTON.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .disabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await waitForQuery(screenAuthenticationInApp
                                 .AUTH_SIGNUP_PASSWORD_INPUT.visible());

      // Condition 1
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT
              .visible(),
          'text', '12345');
      await waitForQuery(
          screenAuthenticationInApp
              .AUTH_SIGNUP_PASSWORD_LENGTH_CONDITION.visible()
              .prop('_passwordConditionIsSatisfied', false));
      await waitForQuery(
          screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_EMAIL_CONDITION
              .visible()
              .prop('_passwordConditionIsSatisfied', false));
      await waitForQuery(
          screenAuthenticationInApp
              .AUTH_SIGNUP_PASSWORD_COMMON_CONDITION.visible()
              .prop('_passwordConditionIsSatisfied', false));

      // Condition 2
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT
              .visible(),
          'text', 'test@test.com');
      await waitForQuery(
          screenAuthenticationInApp
              .AUTH_SIGNUP_PASSWORD_LENGTH_CONDITION.visible()
              .prop('_passwordConditionIsSatisfied', true));
      await waitForQuery(
          screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_EMAIL_CONDITION
              .visible()
              .prop('_passwordConditionIsSatisfied', false));
      await waitForQuery(
          screenAuthenticationInApp
              .AUTH_SIGNUP_PASSWORD_COMMON_CONDITION.visible()
              .prop('_passwordConditionIsSatisfied', true));

      // Condition 3
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT
              .visible(),
          'text', '12345678');
      await waitForQuery(
          screenAuthenticationInApp
              .AUTH_SIGNUP_PASSWORD_LENGTH_CONDITION.visible()
              .prop('_passwordConditionIsSatisfied', true));
      await waitForQuery(
          screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_EMAIL_CONDITION
              .visible()
              .prop('_passwordConditionIsSatisfied', true));
      await waitForQuery(
          screenAuthenticationInApp
              .AUTH_SIGNUP_PASSWORD_COMMON_CONDITION.visible()
              .prop('_passwordConditionIsSatisfied', false));

      await waitForQuery(
          screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
              .disabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT
              .visible(),
          'text', 'P4ass0rd!!');
      await waitForQuery(
          screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
              .enabled());

      await setQueryProperty(
          screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT
              .visible(),
          'text', '');
      await waitForQuery(
          screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
              .disabled());
      await copyToClipboard('P4ass0rd!!');
      await clickOnQuery(screenAuthenticationInApp
                                 .AUTH_SIGNUP_PASSWORD_PASTE_BUTTON.visible());

      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
              .enabled());

      await waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQueryAndClick(
          screenPostAuthentication.BUTTON.visible());

      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQueryAndClick(screenTelemetry.BUTTON.visible());

      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
    });
  });

  describe('Account creation with email verification', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 200,
          bodyValidator: validators.fxaStatus,
          body: {exists: false}
        },

        '/v1/account/create': {
          status: 200,
          bodyValidator: validators.fxaCreate,
          body: {
            sessionToken: 'sessionToken',
            'verified': false,
            verificationMethod: 'email-otp'
          }
        },

        '/v1/session/verify_code': {
          status: 200,
          requiredHeaders: ['Authorization'],
          bodyValidator: validators.fxaVerifyCode,
          body: {}
        },
      },
      DELETEs: {},
    };

    it('Account creation with email verification', async () => {
      if (!(await isFeatureFlippedOn('inAppAuthentication'))) {
        await flipFeatureOn('inAppAuthentication');
        await flipFeatureOn('inAppAccountCreate');
      }

      await waitForInitialView();

      await clickOnQuery(screenInitialize.SIGN_UP_BUTTON.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .disabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await waitForQuery(screenAuthenticationInApp
                                 .AUTH_SIGNUP_PASSWORD_INPUT.visible());

      await waitForQuery(
          screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
              .disabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT
              .visible(),
          'text', 'P4ass0rd!!');
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
              .enabled());

      await waitForQuery(
          screenAuthenticationInApp.AUTH_EMAILVER_TEXT_INPUT.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_EMAILVER_BUTTON.visible()
              .disabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_EMAILVER_TEXT_INPUT.visible(),
          'text', '12345');
      await waitForQuery(
          screenAuthenticationInApp.AUTH_EMAILVER_BUTTON.visible()
              .disabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_EMAILVER_TEXT_INPUT.visible(),
          'text', '123456');
      await waitForQuery(
          screenAuthenticationInApp.AUTH_EMAILVER_BUTTON.visible()
              .enabled());

      await setQueryProperty(
          screenAuthenticationInApp.AUTH_EMAILVER_TEXT_INPUT.visible(),
          'text', '');
      await copyToClipboard('654321');
      await clickOnQuery(screenAuthenticationInApp
                                 .AUTH_EMAILVER_PASTE_BUTTON.visible());

      await clickOnQuery(
          screenAuthenticationInApp.AUTH_EMAILVER_BUTTON.visible()
              .enabled());

      await waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQueryAndClick(
          screenPostAuthentication.BUTTON.visible());

      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQueryAndClick(screenTelemetry.BUTTON.visible());

      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
    });
  });

  describe('Account creation with TOTP', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 200,
          bodyValidator: validators.fxaStatus,
          body: {exists: false}
        },

        '/v1/account/create': {
          status: 200,
          bodyValidator: validators.fxaCreate,
          body: {
            sessionToken: 'sessionToken',
            'verified': false,
            verificationMethod: 'totp-2fa'
          }
        },

        '/v1/session/verify/totp': {
          status: 200,
          requiredHeaders: ['Authorization'],
          bodyValidator: validators.fxaVerifyTotp,
          body: {success: true}
        },
      },
      DELETEs: {},
    };

    it('Account creation with TOTP', async () => {
      if (!(await isFeatureFlippedOn('inAppAuthentication'))) {
        await flipFeatureOn('inAppAuthentication');
        await flipFeatureOn('inAppAccountCreate');
      }

      await waitForInitialView();

      await clickOnQuery(screenInitialize.SIGN_UP_BUTTON.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .disabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await waitForQuery(screenAuthenticationInApp
                                 .AUTH_SIGNUP_PASSWORD_INPUT.visible());

      await waitForQuery(
          screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
              .disabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT
              .visible(),
          'text', 'P4ass0rd!!');
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
              .enabled());

      await waitForQuery(
          screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
              .disabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible(),
          'text', '12345');
      await waitForQuery(
          screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
              .disabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible(),
          'text', '123456');
      await waitForQuery(
          screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
              .enabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible(),
          'text', '');
      await waitForQuery(
          screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
              .disabled());
      await copyToClipboard('123456');
      await clickOnQuery(
          screenAuthenticationInApp.AUTH_TOTP_PASTE_BUTTON.visible());
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
              .enabled());

      await waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQueryAndClick(
          screenPostAuthentication.BUTTON.visible());

      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQueryAndClick(screenTelemetry.BUTTON.visible());

      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
    });
  });

  describe('Authentication with unblock code', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 400,
          bodyValidator: validators.fxaStatus,
          body: {errno: 107, validation: {keys: ['unblockCode']}}
        },
      },
      DELETEs: {},
    };

    it('Authentication with unblock code', async () => {
      if (!(await isFeatureFlippedOn('inAppAuthentication'))) {
        await flipFeatureOn('inAppAuthentication');
        await flipFeatureOn('inAppAccountCreate');
      }

      await waitForInitialView();

      await clickOnQuery(screenInitialize.SIGN_UP_BUTTON.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .disabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await waitForQuery(screenAuthenticationInApp
                                 .AUTH_UNBLOCKCODE_TEXT_INPUT.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_UNBLOCKCODE_BUTTON.visible()
              .disabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_UNBLOCKCODE_TEXT_INPUT
              .visible(),
          'text', '1234567');
      await waitForQuery(
          screenAuthenticationInApp.AUTH_UNBLOCKCODE_BUTTON.visible()
              .disabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_UNBLOCKCODE_TEXT_INPUT
              .visible(),
          'text', '12345678');
      await waitForQuery(
          screenAuthenticationInApp.AUTH_UNBLOCKCODE_BUTTON.visible()
              .enabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_UNBLOCKCODE_TEXT_INPUT
              .visible(),
          'text', '');
      await copyToClipboard("12345678");
      await clickOnQuery(screenAuthenticationInApp
                                 .AUTH_UNBLOCKCODE_PASTE_BUTTON.visible());
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_UNBLOCKCODE_BUTTON.visible()
              .enabled());

      await waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQueryAndClick(
          screenPostAuthentication.BUTTON.visible());

      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQueryAndClick(screenTelemetry.BUTTON.visible());

      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
    });
  });

  describe('Account creation with stub account', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 200,
          bodyValidator: validators.fxaStatus,
          body: {exists: true, hasLinkedAccount: false, hasPassword: false}
        },
      },
      DELETEs: {},
    };

    it('Account creation with stub account', async () => {
      if (!(await isFeatureFlippedOn('inAppAuthentication'))) {
        await flipFeatureOn('inAppAuthentication');
        await flipFeatureOn('inAppAccountCreate');
      }

      await waitForInitialView();

      await clickOnQuery(screenInitialize.SIGN_UP_BUTTON.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .disabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await waitForQuery(screenAuthenticationInApp.AUTH_ERROR_POPUP_BUTTON.visible());
    });
  });

  describe('Account creation with SSO account', function() {
    this.ctx.fxaOverrideEndpoints = {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 200,
          bodyValidator: validators.fxaStatus,
          body: {exists: true, hasLinkedAccount: true, hasPassword: false}
        },
      },
      DELETEs: {},
    };

    it('Account creation with stub account', async () => {
      if (!(await isFeatureFlippedOn('inAppAuthentication'))) {
        await flipFeatureOn('inAppAuthentication');
        await flipFeatureOn('inAppAccountCreate');
      }

      await waitForInitialView();

      await clickOnQuery(screenInitialize.SIGN_UP_BUTTON.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await waitForQuery(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .disabled());
      await setQueryProperty(
          screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await waitForQueryAndClick(
          screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await waitForQuery(screenAuthenticationInApp.AUTH_ERROR_POPUP_BUTTON.visible());
    });
  });

  describe('auth in app related telemetry tests', () => {
    if(runningOnWasm()) {
        // No Glean on WASM.
        return;
    }

    // Telemetry design is detailed at:
    // https://miro.com/app/board/uXjVM0BZcnc=/?share_link_id=228137467679

    const goToAuthStartScreen = async () => {
        await waitForInitialView();
        await clickOnQuery(screenInitialize.SIGN_UP_BUTTON.visible());
        await waitForQuery(
            screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
        await waitForQuery(
            screenAuthenticationInApp.AUTH_START_BUTTON.visible()
                .disabled());

        // Stop for just a tad, to be sure the telemetry APIs get called.
        await wait();
    };

    const goToPasswordScreen = async (signUp = true) => {
        await goToAuthStartScreen();
        await setQueryProperty(
            screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
            'text', 'test@test.com');
        await waitForQueryAndClick(
            screenAuthenticationInApp.AUTH_START_BUTTON.visible()
                .enabled());
        await waitForQuery(screenAuthenticationInApp
                                   [`AUTH_${signUp ? "SIGNUP" : "SIGNIN"}_BUTTON`].visible());

        // Stop for just a tad, to be sure the telemetry APIs get called.
        await wait();
    };

    const goToVerificationScreen = async (queriesFamily) => {
        await goToPasswordScreen();
        await waitForQueryAndWriteInTextField(
            screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT.visible(),
            'P4ass0rd!!'
        );
        await waitForQueryAndClick(
            screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible());
        await waitForQuery(
            screenAuthenticationInApp[`${queriesFamily}_BUTTON`].visible()
                .disabled());

        // Stop for just a tad, to be sure the telemetry APIs get called.
        await wait();
    }

    describe("authentication started screen", async () => {
        const screen = "enter_email";

        beforeEach(async () => {
            await goToAuthStartScreen();
        });

        it("impression event is recorded", async () => {
            const enterEmailViewEvent = await gleanTestGetValue("impression", "enterEmailScreen", "main");
            strictEqual(enterEmailViewEvent.length, 1)
            const enterEmailViewEventExtras = enterEmailViewEvent[0].extra;
            strictEqual(screen, enterEmailViewEventExtras.screen);
        });

        it("back button event is recorded", async () => {
            // Click the "<-" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_START_BACK_BUTTON.visible());
            await testLastInteractionEvent({
                eventName: "backSelected",
                screen,
            });
        });

        it("get help event is recorded", async () => {
            // Click the "Get help" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_START_GET_HELP_LINK.visible());
            await testLastInteractionEvent({
                eventName: "getHelpSelected",
                screen
            });
        });

        it("continue button event is recorded", async () => {
            // Fill out the email form to enable "Continue" button
            await waitForQueryAndWriteInTextField(
                screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
                'test@test.com'
            );
            // Click the now enabled "Continue" button
            await waitForQueryAndClick(
                screenAuthenticationInApp.AUTH_START_BUTTON.visible()
                    .enabled());
            await testLastInteractionEvent({
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

        it("impression event is recorded", async () => {
            const  enterPasswordViewEvent = await gleanTestGetValue("impression", "enterPasswordScreen", "main");
            strictEqual(enterPasswordViewEvent.length, 1)
            const enterPasswordViewEventExtras = enterPasswordViewEvent[0].extra;
            strictEqual(screen, enterPasswordViewEventExtras.screen);
        });

        it("back button event is recorded", async () => {
            // Click the "<-" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_SIGNIN_BACK_BUTTON.visible());
            await testLastInteractionEvent({
                eventName: "backSelected",
                screen
            });
        });

        it("get help event is recorded", async () => {
            // Click the "Get help" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_SIGNIN_GET_HELP_LINK.visible());
            await testLastInteractionEvent({
                eventName: "getHelpSelected",
                screen
            });
        });

        it("paste button event is recorded", async () => {
            // Click the "Paste" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_PASTE_BUTTON.visible());
            await testLastInteractionEvent({
                eventName: "pastePasswordSelected",
                screen
            });
        });

        it("change email button event is recorded", async () => {
            // Click the "Change email" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_SIGNIN_CHANGE_EMAIL_BUTTON.visible());
            await testLastInteractionEvent({
                eventName: "changeEmailSelected",
                screen
            });
        });

        it("forgot password button event is recorded", async () => {
            // Click the "Forgot your password?" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_SIGNIN_FORGOT_PASSWORD_BUTTON.visible());
            await testLastInteractionEvent({
                eventName: "forgotYourPasswordSelected",
                screen
            });
        });

        it("cancel event is recorded", async () => {
            // Click the "Cancel" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_SIGNIN_CANCEL_BUTTON.visible());
            await testLastInteractionEvent({
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
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_TERMS_OF_SERVICE.visible());
            await testLastInteractionEvent({
                eventName: "termsOfServiceSelected",
                screen
            });

            // Click the "Privacy Notice" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_PRIVACY_NOTICE.visible());
            await testLastInteractionEvent({
                eventName: "privacyNoticeSelected",
                screen
            });
        });

        it("signin button event and outcome event are recorded", async () => {
            // Fill out the email form to enable "Sign in" button
            await waitForQueryAndWriteInTextField(
                screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible(),
                'P4ass0rd!!'
            );
            // Click the now enabled "Sign in" button
            await waitForQueryAndClick(
                screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
                    .enabled());
            await testLastInteractionEvent({
                eventName: "signInSelected",
                screen
            });

            const startedOutcomeEvent = await gleanTestGetValue("outcome", "loginStarted", "main");
            strictEqual(startedOutcomeEvent.length, 1);

            await waitForMozillaProperty(
                'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

            const endedOutcomeEvent = await gleanTestGetValue("outcome", "loginEnded", "main");
            strictEqual(endedOutcomeEvent.length, 1);

            // Make sure 2fa outcome event were NOT recorded
            const unwantedEvents = [
                ...(await gleanTestGetValue("outcome", "twoFaVerificationFailed", "main")),
                ...(await gleanTestGetValue("outcome", "twoFaVerificationSucceeded", "main"))
            ];
            strictEqual(unwantedEvents.length, 0);
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
                        bodyValidator: validators.fxaStatus,
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
            await setQueryProperty(
                screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
                'text', 'test@test.com');
            await waitForQueryAndClick(
                screenAuthenticationInApp.AUTH_START_BUTTON.visible()
                    .enabled());
            await waitForQuery(
                screenAuthenticationInApp.AUTH_UNBLOCKCODE_BUTTON.visible());
        });

        it("impression event is recorded", async () => {
            const  unblockCodeViewEvent = await gleanTestGetValue("impression", "enterUnblockCodeScreen", "main");
            strictEqual(unblockCodeViewEvent.length, 1)
            const unblockCodeViewEventExtras = unblockCodeViewEvent[0].extra;
            strictEqual(screen, unblockCodeViewEventExtras.screen);
        });

        it("close button event is recorded", async () => {
            // Click the "Close" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_UNBLOCKCODE_BACK_BUTTON.visible());
            await testLastInteractionEvent({
                eventName: "closeSelected",
                screen
            });
        });

        it("get help event is recorded", async () => {
            // Click the "Get help" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_UNBLOCKCODE_GET_HELP_LINK.visible());
            await testLastInteractionEvent({
                eventName: "getHelpSelected",
                screen
            });
        });

        it("cancel event is recorded", async () => {
            // Click the "Cancel" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_UNBLOCKCODE_CANCEL_BUTTON.visible());
            await testLastInteractionEvent({
                eventName: "cancelSelected",
                screen
            });
        });

        it("paste button event is recorded", async () => {
            // Click the "Paste" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_UNBLOCKCODE_PASTE_BUTTON.visible());
            await testLastInteractionEvent({
                eventName: "pastePasswordSelected",
                screen
            });
        });

        it("resend code button event is recorded", async () => {
            // Click the "Resend Code" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_UNBLOCKCODE_RESEND_CODE_BUTTON.visible());
            await testLastInteractionEvent({
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
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_TERMS_OF_SERVICE.visible());
            await testLastInteractionEvent({
                eventName: "termsOfServiceSelected",
                screen
            });

            // Click the "Privacy Notice" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_PRIVACY_NOTICE.visible());
            await testLastInteractionEvent({
                eventName: "privacyNoticeSelected",
                screen
            });
        });

        it("verify button events are recorded", async () => {
            // Let's start with a failing verification
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 400;
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].body = {errno: 107, validation: {keys: ['unblockCode']}}

            // Fill out the code form to enable "Verify" button
            await waitForQueryAndWriteInTextField(
                screenAuthenticationInApp.AUTH_UNBLOCKCODE_TEXT_INPUT.visible(),
                '87654321'
            );
            // Click the now enabled "Verify" button
            await waitForQueryAndClick(
                screenAuthenticationInApp.AUTH_UNBLOCKCODE_BUTTON.visible()
                    .enabled());
            await testLastInteractionEvent({
                eventName: "verifySelected",
                elementId: "verify",
                screen
            });

            // First a failing outcome
            let failedOutcomeEvent;
            await waitForCondition(async () => {
                failedOutcomeEvent = await gleanTestGetValue("outcome", "twoFaVerificationFailed", "main");
                return failedOutcomeEvent.length == 1;
            });
            const failedOutcomeExtras = failedOutcomeEvent[0].extra;
            strictEqual("unblock_code", failedOutcomeExtras.type);

            // Prepare mocks for a successfull verification
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/account/login'].status = 200;

            // Clear the verification code from the form
            await waitForQueryAndWriteInTextField(
                screenAuthenticationInApp.AUTH_UNBLOCKCODE_TEXT_INPUT.visible(),
                ''
            );
            // Fill out the code form to enable "Verify" button
            await waitForQueryAndWriteInTextField(
                screenAuthenticationInApp.AUTH_UNBLOCKCODE_TEXT_INPUT.visible(),
                '12345678'
            );
            // Click the now enabled "Verify" button
            await waitForQueryAndClick(
                screenAuthenticationInApp.AUTH_UNBLOCKCODE_BUTTON.visible()
                    .enabled());
            await testLastInteractionEvent({
                eventName: "verifySelected",
                screen
            });
            // Give the authentication APIs a moment to finish.
            await wait();

            // Now the successfull outcome
            const successOutcomeEvent = await gleanTestGetValue("outcome", "twoFaVerificationSucceeded", "main");
            strictEqual(successOutcomeEvent.length, 1);
            const successOutcomeExtras = successOutcomeEvent[0].extra;
            strictEqual("unblock_code", successOutcomeExtras.type);
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
                        bodyValidator: validators.fxaStatus,
                        body: { exists: false }
                    },
                    '/v1/account/create': {
                        status: 200,
                        bodyValidator: validators.fxaCreate,
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
            const  createPasswordViewEvent = await gleanTestGetValue("impression", "createPasswordScreen", "main");
            strictEqual(createPasswordViewEvent.length, 1)
            const createPasswordViewEventExtras = createPasswordViewEvent[0].extra;
            strictEqual(screen, createPasswordViewEventExtras.screen);
        });

        it("back button event is recorded", async () => {
            // Click the "<-" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_SIGNUP_BACK_BUTTON.visible());
            await testLastInteractionEvent({
                eventName: "backSelected",
                screen
            });
        });

        it("get help event is recorded", async () => {
            // Click the "Get help" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_SIGNUP_GET_HELP_LINK.visible());
            await testLastInteractionEvent({
                eventName: "getHelpSelected",
                screen
            });
        });

        it("cancel event is recorded", async () => {
            // Click the "Cancel" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_SIGNUP_CANCEL_BUTTON.visible());
            await testLastInteractionEvent({
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
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_TERMS_OF_SERVICE.visible());
            await testLastInteractionEvent({
                eventName: "termsOfServiceSelected",
                screen
            });

            // Click the "Privacy Notice" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_PRIVACY_NOTICE.visible());
            await testLastInteractionEvent({
                eventName: "privacyNoticeSelected",
                screen
            });
        });

        it("create account button event and outcome event are recorded", async () => {
            // Fill out the password form to enable "Create account" button
            await waitForQueryAndWriteInTextField(
                screenAuthenticationInApp.AUTH_SIGNUP_PASSWORD_INPUT.visible(),
                'P4ass0rd!!'
            );
            // Click the now enabled "Create account" button
            await waitForQueryAndClick(
                screenAuthenticationInApp.AUTH_SIGNUP_BUTTON.visible()
                    .enabled());
            // Two events are exected here,
            // because of the pressing of the "Continue" button in the start screen
            await testLastInteractionEvent({
                eventName: "createAccountSelected",
                screen
            });

            const startedEvent = await gleanTestGetValue("outcome", "registrationStarted", "main");
            strictEqual(startedEvent.length, 1);

            await waitForMozillaProperty(
                'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

            const completedOutcomeEvent = await gleanTestGetValue("outcome", "registrationCompleted", "main");
            strictEqual(completedOutcomeEvent.length, 1);

             // Make sure no 2fa outcome event was NOT recorded
             const unwantedEvents = [
                ...(await gleanTestGetValue("outcome", "twoFaVerificationFailed", "main")),
                ...(await gleanTestGetValue("outcome", "twoFaVerificationSucceeded", "main"))
            ];
            strictEqual(unwantedEvents.length, 0);
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
                        bodyValidator: validators.fxaStatus,
                        body: { exists: false }
                    },
                    '/v1/account/create': {
                        status: 200,
                        bodyValidator: validators.fxaCreate,
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
          const verificationViewEvent = await gleanTestGetValue(
              "impression", "enterVerificationCodeScreen", "main");
          strictEqual(verificationViewEvent.length, 1)
          const verificationViewEventExtras = verificationViewEvent[0].extra;
          strictEqual(screen, verificationViewEventExtras.screen);
        });

        it("get help event is recorded", async () => {
            // Click the "Get help" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_EMAILVER_GET_HELP_LINK.visible());
            await testLastInteractionEvent({
                eventName: "getHelpSelected",
                screen
            });
        });

        it("cancel event is recorded", async () => {
            // Click the "Cancel" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_EMAILVER_CANCEL_BUTTON.visible());
            await testLastInteractionEvent({
                eventName: "cancelSelected",
                screen
            });
        });

        it("paste button event is recorded", async () => {
            // Click the "Paste" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_EMAILVER_PASTE_BUTTON.visible());
            await testLastInteractionEvent({
                eventName: "pastePasswordSelected",
                screen
            });
        });

        it("resend code button event is recorded", async () => {
            // Click the "Resend Code" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_EMAILVER_RESEND_CODE_BUTTON.visible());
            await testLastInteractionEvent({
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
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_TERMS_OF_SERVICE.visible());
            await testLastInteractionEvent({
                eventName: "termsOfServiceSelected",
                screen
            });

            // Click the "Privacy Notice" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_PRIVACY_NOTICE.visible());
            await testLastInteractionEvent({
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
            await waitForQueryAndWriteInTextField(
                screenAuthenticationInApp.AUTH_EMAILVER_TEXT_INPUT.visible(),
                '654321'
            );
            // Click the now enabled "Verify" button
            await waitForQueryAndClick(
                screenAuthenticationInApp.AUTH_EMAILVER_BUTTON.visible()
                    .enabled());
            await testLastInteractionEvent({
                eventName: "verifySelected",
                screen
            });

            // First a failing outcome
            let failedOutcomeEvent;
            await waitForCondition(async () => {
                failedOutcomeEvent = await gleanTestGetValue("outcome", "twoFaVerificationFailed", "main");
                return failedOutcomeEvent.length == 1;
            });
            const failedOutcomeExtras = failedOutcomeEvent[0].extra;
            strictEqual("email", failedOutcomeExtras.type);

            // Prepare mocks for a successfull verification
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify_code'].status = 200;
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify_code'].body = {};

            // Clear the verification code from the form
            await waitForQueryAndWriteInTextField(
                screenAuthenticationInApp.AUTH_EMAILVER_TEXT_INPUT.visible(),
                ''
            );
            // Fill out the code form to enable "Verify" button
            await waitForQueryAndWriteInTextField(
                screenAuthenticationInApp.AUTH_EMAILVER_TEXT_INPUT.visible(),
                '123456'
            );
            // Click the now enabled "Verify" button
            await waitForQueryAndClick(
                screenAuthenticationInApp.AUTH_EMAILVER_BUTTON.visible()
                    .enabled());
            await testLastInteractionEvent({
                eventName: "verifySelected",
                screen
            });
            // Give the authentication APIs a moment to finish.
            await wait();

            // Now the successfull outcome
            const successOutcomeEvent = await gleanTestGetValue("outcome", "twoFaVerificationSucceeded", "main");
            strictEqual(successOutcomeEvent.length, 1);
            const successOutcomeExtras = successOutcomeEvent[0].extra;
            strictEqual("email", successOutcomeExtras.type);
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
                        bodyValidator: validators.fxaStatus,
                        body: { exists: false }
                    },
                    '/v1/account/create': {
                        status: 200,
                        bodyValidator: validators.fxaCreate,
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
            const  verificationViewEvent = await gleanTestGetValue("impression", "enterSecurityCodeScreen", "main");
            strictEqual(verificationViewEvent.length, 1)
            const verificationViewEventExtras = verificationViewEvent[0].extra;
            strictEqual(screen, verificationViewEventExtras.screen);
        });

        it("get help event is recorded", async () => {
            // Click the "Get help" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_TOTP_GET_HELP_LINK.visible());
            await testLastInteractionEvent({
                eventName: "getHelpSelected",
                screen
            });
        });

        it("cancel event is recorded", async () => {
            // Click the "Cancel" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_TOTP_CANCEL_BUTTON.visible());
            await testLastInteractionEvent({
                eventName: "cancelSelected",
                screen
            });
        });

        it("paste button event is recorded", async () => {
            // Click the "Paste" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_TOTP_PASTE_BUTTON.visible());
            await testLastInteractionEvent({
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
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_TERMS_OF_SERVICE.visible());
            await testLastInteractionEvent({
                eventName: "termsOfServiceSelected",
                screen
            });

            // Click the "Privacy Notice" button
            await waitForQueryAndClick(screenAuthenticationInApp.AUTH_PRIVACY_NOTICE.visible());
            await testLastInteractionEvent({
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
            await waitForQueryAndWriteInTextField(
                screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible(),
                '654321'
            );
            // Click the now enabled "Verify" button
            await waitForQueryAndClick(
                screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
                    .enabled());
            await testLastInteractionEvent({
                eventName: "verifySelected",
                screen
            });

            // First a failing outcome
            let failedOutcomeEvent;
            await waitForCondition(async () => {
                failedOutcomeEvent = await gleanTestGetValue("outcome", "twoFaVerificationFailed", "main");
                return failedOutcomeEvent.length == 1;
            });
            const failedOutcomeExtras = failedOutcomeEvent[0].extra;
            strictEqual("totp", failedOutcomeExtras.type);

            // Prepare mocks for a successfull verification
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify/totp'].status = 200;
            this.ctx.fxaOverrideEndpoints.POSTs['/v1/session/verify/totp'].body = {
                success: true
            };

            // Clear the verification code from the form
            await waitForQueryAndWriteInTextField(
                screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible(),
                ''
            );
            // Fill out the code form to enable "Verify" button
            await waitForQueryAndWriteInTextField(
                screenAuthenticationInApp.AUTH_TOTP_TEXT_INPUT.visible(),
                '123456'
            );
            // Click the now enabled "Verify" button
            await waitForQueryAndClick(
                screenAuthenticationInApp.AUTH_TOTP_BUTTON.visible()
                    .enabled());
            await testLastInteractionEvent({
                eventName: "verifySelected",
                screen
            });
            // Give the authentication APIs a moment to finish.
            await wait();

            // Now the successfull outcome
            const successOutcomeEvent = await gleanTestGetValue("outcome", "twoFaVerificationSucceeded", "main");
            strictEqual(successOutcomeEvent.length, 1);
            const successOutcomeExtras = successOutcomeEvent[0].extra;
            strictEqual("totp", successOutcomeExtras.type);
        });
    });
  });
});
