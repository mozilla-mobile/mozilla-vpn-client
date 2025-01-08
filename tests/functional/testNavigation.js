/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');
const queries = require('./queries.js');

describe('Navigation bar', async function() {
  this.timeout(60000);

  async function navigationBarVisible() {
    await vpn.waitForQuery(queries.navBar.HOME);
    return await vpn.getQueryProperty(queries.navBar.HOME, 'visible');
  }


  it('Is not visible over initial screen', async () => {
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
    assert.equal(await navigationBarVisible(), 'false');
  });


  it('Is not visible over pre-authentication Help menu', async () => {
    await vpn.waitForQueryAndClick(
        queries.screenInitialize.GET_HELP_LINK.visible());
    await vpn.waitForQuery(queries.screenGetHelp.BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
    assert.equal(await navigationBarVisible(), 'false');
  });


  it('Is not visible over desktop onboarding', async () => {
    await vpn.waitForQueryAndClick(
        queries.screenInitialize.ALREADY_A_SUBSCRIBER_LINK.visible());
    assert.equal(await navigationBarVisible(), 'false');
  });


  it('Is not visible during browser authentication', async () => {
    await vpn.flipFeatureOff('inAppAuthentication');
    await vpn.waitForInitialView();
    await vpn.waitForQueryAndClick(
        queries.screenInitialize.SIGN_UP_BUTTON.visible());

    if (!this.ctx.wasm) {
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });
    }

    await vpn.waitForQuery(
        queries.screenInitialize.AUTHENTICATE_VIEW.visible());
    assert.equal(await navigationBarVisible(), 'false');
  });

  describe('Authenticated tests', async function() {
    this.ctx.authenticationNeeded = true;

    it('Is visible over the Home screen', async () => {
      await vpn.waitForQuery(queries.navBar.HOME.visible());
    });

    it('Has Settings, Messaging, and Home buttons', async () => {
      await vpn.waitForQuery(queries.navBar.HOME.visible());
      await vpn.waitForQuery(queries.navBar.MESSAGES.visible());
      await vpn.waitForQuery(queries.navBar.SETTINGS.visible());
    });

    it('Clicking the Settings button opens settings screen', async () => {
      await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
      await vpn.waitForQuery(queries.screenSettings.SCREEN.visible());
    });


    it('Clicking the Messages button opens messaging screen', async () => {
      await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
      await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());
    });


    it('Clicking the Home button opens home screen', async () => {
      await vpn.waitForQueryAndClick(queries.navBar.MESSAGES.visible());
      await vpn.waitForQuery(queries.screenMessaging.SCREEN.visible());

      await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    });


    it('Clicking a selected Home button reloads the Home screen', async () => {
      await vpn.waitForQueryAndClick(
          queries.screenHome.SERVER_LIST_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenHome.serverListView.BACK_BUTTON.visible());

      await vpn.waitForQueryAndClick(queries.navBar.HOME.visible());
      await vpn.waitForQueryAndClick(
          queries.screenHome.SERVER_LIST_BUTTON.visible());
    });


    it('Clicking a selected Settings button reloads the Settings screen',
       async () => {
         await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());

         await vpn.waitForQueryAndClick(
             queries.screenSettings.GET_HELP.visible());

         await vpn.waitForQuery(queries.screenGetHelp.BACK_BUTTON.visible());

         await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
         await vpn.waitForQuery(queries.screenSettings.GET_HELP.visible());
       });


    it('Is visible over the post-auth Help menu', async () => {
      await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());

      await vpn.waitForQueryAndClick(queries.screenSettings.GET_HELP.visible());

      await vpn.waitForQuery(queries.screenGetHelp.BACK_BUTTON.visible());

      await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
      assert.equal(await navigationBarVisible(), 'true');
    });

    it("Time to main screen is recorded", async () => {
      // Check that the time to the main screen was greater than 1ms but less than a full second.
      let timing = await vpn.gleanTestGetValue("performance", "timeToMainScreen", "");
      assert(timing.sum > 1000000);
      assert(timing.sum < 1000000000);
    })
  });

  /* TODO:...
    - selected navbar icon correctly reflects screen opens from systray...
    - not visible during reauth screen...
    what else?
  */
});
