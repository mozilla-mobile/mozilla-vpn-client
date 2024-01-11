/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { equal } from 'assert';
import { waitForQuery, getQueryProperty, waitForQueryAndClick, flipFeatureOff, waitForInitialView, waitForCondition, getLastUrl, authenticateInApp } from './helper.js';
import { navBar, global, screenInitialize, screenGetHelp, screenTelemetry, screenSettings, screenMessaging, screenHome } from './queries.js';

describe('Navigation bar', async function() {
  this.timeout(60000);

  async function navigationBarVisible() {
    await waitForQuery(navBar.HOME);
    return await getQueryProperty(navBar.HOME, 'visible');
  }


  it('Is not visible over initial screen', async () => {
    await waitForQuery(global.SCREEN_LOADER.ready());
    equal(await navigationBarVisible(), 'false');
  });


  it('Is not visible over pre-authentication Get help menu', async () => {
    await waitForQueryAndClick(
        screenInitialize.GET_HELP_LINK.visible());
    await waitForQuery(screenGetHelp.BACK_BUTTON.visible());
    await waitForQuery(global.SCREEN_LOADER.ready());
    equal(await navigationBarVisible(), 'false');
  });


  it('Is not visible over desktop onboarding', async () => {
    await waitForQueryAndClick(
        screenInitialize.ALREADY_A_SUBSCRIBER_LINK.visible());
    equal(await navigationBarVisible(), 'false');
  });


  it('Is not visible during browser authentication', async () => {
    await flipFeatureOff('inAppAuthentication');
    await waitForInitialView();
    await waitForQueryAndClick(
        screenInitialize.SIGN_UP_BUTTON.visible());

    if (!this.ctx.wasm) {
      await waitForCondition(async () => {
        const url = await getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });
    }

    await waitForQuery(
        screenInitialize.AUTHENTICATE_VIEW.visible());
    equal(await navigationBarVisible(), 'false');
  });


  it('Is not visible over telemetry screen', async () => {
    await authenticateInApp(true, false);
    await waitForQuery(screenTelemetry.BUTTON.visible());
    equal(await navigationBarVisible(), 'false');
  });


  describe('Authenticated tests', async function() {
    this.ctx.authenticationNeeded = true;

    it('Is visible over the Home screen', async () => {
      await waitForQuery(navBar.HOME.visible());
    });

    it('Has Settings, Messaging, and Home buttons', async () => {
      await waitForQuery(navBar.HOME.visible());
      await waitForQuery(navBar.MESSAGES.visible());
      await waitForQuery(navBar.SETTINGS.visible());
    });

    it('Clicking the Settings button opens settings screen', async () => {
      await waitForQueryAndClick(navBar.SETTINGS.visible());
      await waitForQuery(screenSettings.SCREEN.visible());
    });


    it('Clicking the Messages button opens messaging screen', async () => {
      await waitForQueryAndClick(navBar.MESSAGES.visible());
      await waitForQuery(screenMessaging.SCREEN.visible());
    });


    it('Clicking the Home button opens home screen', async () => {
      await waitForQueryAndClick(navBar.MESSAGES.visible());
      await waitForQuery(screenMessaging.SCREEN.visible());

      await waitForQueryAndClick(navBar.HOME.visible());
      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
    });


    it('Clicking a selected Home button reloads the Home screen', async () => {
      await waitForQueryAndClick(
          screenHome.SERVER_LIST_BUTTON.visible());
      await waitForQuery(
          screenHome.serverListView.BACK_BUTTON.visible());

      await waitForQueryAndClick(navBar.HOME.visible());
      await waitForQueryAndClick(
          screenHome.SERVER_LIST_BUTTON.visible());
    });


    it('Clicking a selected Settings button reloads the Settings screen',
       async () => {
         await waitForQueryAndClick(navBar.SETTINGS.visible());

         await waitForQueryAndClick(
             screenSettings.GET_HELP.visible());

         await waitForQuery(screenGetHelp.BACK_BUTTON.visible());

         await waitForQueryAndClick(navBar.SETTINGS.visible());
         await waitForQuery(screenSettings.GET_HELP.visible());
       });


    it('Is visible over the post-auth Get help menu', async () => {
      await waitForQueryAndClick(navBar.SETTINGS.visible());

      await waitForQueryAndClick(screenSettings.GET_HELP.visible());

      await waitForQuery(screenGetHelp.BACK_BUTTON.visible());

      await waitForQueryAndClick(navBar.SETTINGS.visible());
      equal(await navigationBarVisible(), 'true');
    });
  });

  /* TODO:...
    - selected navbar icon correctly reflects screen opens from systray...
    - not visible during reauth screen...
    what else?
  */
});
