/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import assert, { equal, strictEqual } from 'assert';
import { screenInitialize, screenGetHelp, screenAuthenticationInApp } from './queries.js';
import { waitForQueryAndClick, waitForQuery, setQueryProperty, wait, getQueryProperty, runningOnWasm, gleanTestGetValue } from './helper.js';


describe('Initialize', function() {
  it('Navigating to and from the help menu is possible', async () => {
    await waitForQueryAndClick(
        screenInitialize.GET_HELP_LINK.visible());
    await waitForQueryAndClick(screenGetHelp.BACK_BUTTON);

    await waitForQuery(screenInitialize.SWIPE_VIEW.visible());
  });

  it('SwipeView is visible', async () => {
    await waitForQuery(screenInitialize.SWIPE_VIEW.visible());
  });

  it('Sign up button is visible', async () => {
    await waitForQuery(screenInitialize.SIGN_UP_BUTTON.visible());
  });

  it('Already a subscriber button is visible', async () => {
    await waitForQuery(
        screenInitialize.ALREADY_A_SUBSCRIBER_LINK.visible());
  });

  it('Panel title is set correctly based on StackView currentIndex',
     async () => {
       await waitForQuery(screenInitialize.SWIPE_VIEW.visible());
       await setQueryProperty(
           screenInitialize.SWIPE_VIEW, 'currentIndex', 0);
       await wait();
       await waitForQuery(screenInitialize.PANEL_TITLE.visible());
       equal(
           await getQueryProperty(
               screenInitialize.PANEL_TITLE, 'text'), 'Mozilla VPN');
     });

  it('Panel description is set correctly based on StackView currentIndex',
     async () => {
       await waitForQuery(screenInitialize.SWIPE_VIEW.visible());
       await setQueryProperty(
           screenInitialize.SWIPE_VIEW, 'currentIndex', 0);
       await wait();
       const descriptionText = await getQueryProperty(
           screenInitialize.PANEL_DESCRIPTION, 'text');
       assert(descriptionText.includes('Firefox'));
     });

  it('Panel title and description are updated when SwipeView currentIndex changes',
     async () => {
       await waitForQuery(screenInitialize.SWIPE_VIEW.visible());
       await setQueryProperty(
           screenInitialize.SWIPE_VIEW, 'currentIndex', 2);
       await wait();
       equal(
           await getQueryProperty(
               screenInitialize.PANEL_TITLE.visible(), 'text'),
           'One tap to safety');
       const descriptionText = await getQueryProperty(
           screenInitialize.PANEL_DESCRIPTION, 'text');
       assert(descriptionText.includes('Protecting yourself is simple'));
     });

  it('Sign up button opens auth flow', async () => {
    await waitForQueryAndClick(
        screenInitialize.SIGN_UP_BUTTON.visible());
    await waitForQuery(
        screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
  });

  it('Already a subscriber? opens auth flow', async () => {
    await waitForQueryAndClick(
        screenInitialize.ALREADY_A_SUBSCRIBER_LINK.visible());
    await waitForQuery(
        screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
  });

  describe('initialize related telemetry tests', () => {
    if(runningOnWasm()) {
      // No Glean on WASM.
      return;
    }

    const telemetryScreenId = "signup";

    // Telemetry design is detailed at:
    // https://miro.com/app/board/uXjVM0BZcnc=/?userEmail=sandrigo@mozilla.com&openComment=3458764559943493792&mid=8418131&utm_source=notification&utm_medium=email&utm_campaign=mentions&utm_content=reply-to-mention&share_link_id=496680579489

    it("impression event is recorded", async () => {
      await waitForQuery(screenInitialize.SIGN_UP_BUTTON.visible());
      const signupScreenEvents = await gleanTestGetValue("impression", "signupScreen", "main");
      strictEqual(signupScreenEvents.length, 1);
      const signupScreenExtras = signupScreenEvents[0].extra;
      strictEqual(telemetryScreenId, signupScreenExtras.screen);
    });

    it("get help event is recorded", async () => {
      // Click on the "Get help" link
      await waitForQueryAndClick(
        screenInitialize.GET_HELP_LINK.visible());
      const getHelpEvents = await gleanTestGetValue("interaction", "getHelpSelected", "main");
      strictEqual(getHelpEvents.length, 1);
      const getHelpExtras = getHelpEvents[0].extra;
      strictEqual(telemetryScreenId, getHelpExtras.screen);
    });

    it("sign up click event is recorded", async () => {
      // Click the "Sign up" button
      await waitForQueryAndClick(
        screenInitialize.SIGN_UP_BUTTON.visible());
      const signupButtonEvents = await gleanTestGetValue("interaction", "signupSelected", "main");
      strictEqual(signupButtonEvents.length, 1);
      const signupButtonExtras = signupButtonEvents[0].extra;
      strictEqual(telemetryScreenId, signupButtonExtras.screen);
    });

    it("already a subscriber click event is recorded", async () => {
      // Click the "Already a subscriber?" button
      await waitForQueryAndClick(
        screenInitialize.ALREADY_A_SUBSCRIBER_LINK.visible());
      const alreadyASubscriberButtonEvents = await gleanTestGetValue("interaction", "alreadyASubscriberSelected", "main");
      strictEqual(alreadyASubscriberButtonEvents.length, 1);
      const alreadyASubscriberButtonExtras = alreadyASubscriberButtonEvents[0].extra;
      strictEqual(telemetryScreenId, alreadyASubscriberButtonExtras.screen);
    });
  });
});
