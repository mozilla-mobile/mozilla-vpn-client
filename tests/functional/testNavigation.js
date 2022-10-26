/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const { navBar, initialScreen, getHelpScreen, settingsScreen, telemetryScreen, homeScreen, inAppMessagingScreen } = require('./elements.js');
const vpn = require('./helper.js');

describe('Navigation bar', async function() {
  this.timeout(60000);

  async function navigationBarVisible() {
    await vpn.waitForElement(navBar.HOME);
    return await vpn.getElementProperty(navBar.HOME, 'visible');
  }


  it('Is not visible over initial screen', async () => {
    await vpn.wait();
    assert(await navigationBarVisible() === 'false');
  });


  it('Is not visible over pre-authentication Get help menu', async () => {
    await vpn.waitForElementAndClick(initialScreen.GET_HELP_LINK);
    await vpn.waitForElement(getHelpScreen.BACK);
    await vpn.waitForElementProperty(getHelpScreen.BACK, 'visible', 'true');
    await vpn.wait();
    assert(await navigationBarVisible() === 'false');
  });


  it('Is not visible over desktop onboarding', async () => {    
    await vpn.waitForElementAndClick(initialScreen.LEARN_MORE_LINK);
    assert(await navigationBarVisible() === 'false');
  });


  it('Is not visible during browser authentication', async () => {    
    await vpn.waitForMainView();
    await vpn.clickOnElement(initialScreen.GET_STARTED);

    if (!this.ctx.wasm) {
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });
    }
    
    await vpn.waitForElement(initialScreen.AUTHENTICATE_VIEW);
    await vpn.waitForElementProperty(initialScreen.AUTHENTICATE_VIEW, 'visible', 'true');
    assert(await navigationBarVisible() === 'false');
  });


  it('Is not visible over telemetry screen', async () => {
    await vpn.authenticateInApp(true, false);
    await vpn.waitForElement(telemetryScreen.TELEMETRY_POLICY_BUTTON);
    assert(await navigationBarVisible() === 'false');
  });


  describe('Authenticated tests', async function() {
    this.ctx.authenticationNeeded = true;

    it('Is visible over the Home screen', async () => {
      await vpn.waitForElement(navBar.HOME);
      await vpn.waitForElementProperty(navBar.HOME, 'visible', 'true');
      assert(await navigationBarVisible() === 'true');
    });

    it('Has Settings, Messaging, and Home buttons', async () => {
      await vpn.waitForElement(navBar.HOME);
      await vpn.waitForElementProperty(navBar.HOME, 'visible', 'true');

      await vpn.waitForElement(navBar.MESSAGES);
      await vpn.waitForElementProperty(navBar.MESSAGES, 'visible', 'true');

      await vpn.waitForElement(navBar.SETTINGS);
      await vpn.waitForElementProperty(navBar.SETTINGS, 'visible', 'true');
    });


    it('Clicking the Settings button opens settings screen', async () => {
      await vpn.waitForElement(navBar.SETTINGS);
      await vpn.clickOnElement(navBar.SETTINGS);
      await vpn.waitForElement(settingsScreen.SETTINGS);
      assert(   await vpn.getElementProperty(settingsScreen.SETTINGS, 'visible') === 'true');
    });


    it('Clicking the Messages button opens messaging screen', async () => {
      await vpn.waitForElement(navBar.MESSAGES);
      await vpn.clickOnElement(navBar.MESSAGES);
      await vpn.waitForElement(inAppMessagingScreen.SCREEN);
      assert(await vpn.getElementProperty(inAppMessagingScreen.SCREEN, 'visible') === 'true');
    });


    it('Clicking the Home button opens home screen', async () => {
      await vpn.waitForElement(navBar.MESSAGES);
      await vpn.clickOnElement(navBar.MESSAGES);
      await vpn.waitForElement(inAppMessagingScreen.SCREEN);

      await vpn.clickOnElement(navBar.HOME);
      await vpn.waitForElement(homeScreen.SERVER_LIST_BUTTON);

      assert(await vpn.getElementProperty(homeScreen.SERVER_LIST_BUTTON, 'visible') === 'true');
    });


    it('Clicking a selected Home button reloads the Home screen', async () => {
      await vpn.waitForElement(homeScreen.SERVER_LIST_BUTTON);
      await vpn.clickOnElement(homeScreen.SERVER_LIST_BUTTON);
      await vpn.waitForElement(homeScreen.selectSingleHopServerView.BACK_BUTTON);
      await vpn.waitForElementProperty(homeScreen.selectSingleHopServerView.BACK_BUTTON, 'visible', 'true');

      await vpn.waitForElementAndClick(navBar.HOME);
      assert(await vpn.getElementProperty(homeScreen.SERVER_LIST_BUTTON, 'visible') === 'true');
    });


    it('Clicking a selected Settings button reloads the Settings screen',
       async () => {
         await vpn
             .waitForElement(navBar.SETTINGS)
                 await vpn.clickOnElement(
                     navBar.SETTINGS);

         await vpn.waitForElement(settingsScreen.GET_HELP);
         await vpn.waitForElementProperty(settingsScreen.GET_HELP, 'visible', 'true');
         await vpn.clickOnElement(settingsScreen.GET_HELP);

         await vpn.waitForElement(getHelpScreen.BACK);
         await vpn.waitForElementProperty(getHelpScreen.BACK, 'visible', 'true');

         await vpn.waitForElementAndClick(navBar.SETTINGS);
         assert(await vpn.getElementProperty(settingsScreen.GET_HELP, 'visible') === 'true');
       });


    it('Is visible over the post-auth Get help menu', async () => {
      await vpn.waitForElement(navBar.SETTINGS)
          await vpn.clickOnElement(navBar.SETTINGS);

      await vpn.waitForElement(settingsScreen.GET_HELP);
      await vpn.waitForElementProperty(settingsScreen.GET_HELP, 'visible', 'true');
      await vpn.clickOnElement(settingsScreen.GET_HELP);

      await vpn.waitForElement(getHelpScreen.BACK);
      await vpn.waitForElementProperty(getHelpScreen.BACK, 'visible', 'true');

      await vpn.waitForElementAndClick(navBar.SETTINGS);
      assert(await navigationBarVisible() === 'true');
    });
  });

  /* TODO:...
    - selected navbar icon correctly reflects screen opens from systray...
    - not visible during reauth screen...
    what else?
  */
});
