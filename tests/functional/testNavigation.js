/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const {waitForElementProperty} = require('./helper.js');
const vpn = require('./helper.js');

describe('Navigation bar', async function() {
  this.timeout(60000);

  async function navigationBarVisible() {
    await vpn.waitForElement('navigationLayout/navButton-home');
    return await vpn.getElementProperty(
        'navigationLayout/navButton-home', 'visible');
  }


  it('Is not visible over initial screen', async () => {
    await vpn.wait();
    assert(await navigationBarVisible() === 'false');
  });


  it('Is not visible over pre-authentication Get help menu', async () => {
    await vpn.clickOnElement('getHelpLink');
    await vpn.waitForElement('getHelpBack');
    await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');
    await vpn.wait();
    assert(await navigationBarVisible() === 'false');
  });


  it('Is not visible over desktop onboarding', async () => {
    await vpn.waitForElement('learnMoreLink')
        await vpn.clickOnElement('learnMoreLink');
    assert(await navigationBarVisible() === 'false');
  });


  it('Is not visible during browser authentication', async () => {
    await vpn.waitForMainView();
    await vpn.clickOnElement('getStarted');

    if (!this.ctx.wasm) {
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });
    }

    await vpn.waitForElement('authenticatingView');
    await vpn.waitForElementProperty('authenticatingView', 'visible', 'true');
    assert(await navigationBarVisible() === 'false');
  });


  it('Is not visible over telemetry screen', async () => {
    await vpn.authenticateInApp(true, false);
    await vpn.waitForElement('telemetryPolicyButton');
    assert(await navigationBarVisible() === 'false');
  });


  describe('Authenticated tests', async function() {
    this.ctx.authenticationNeeded = true;

    it('Is visible over the Home screen', async () => {
      await vpn.waitForElement('navigationLayout/navButton-home');
      await vpn.waitForElementProperty(
          'navigationLayout/navButton-home', 'visible', 'true');
      assert(await navigationBarVisible() === 'true');
    });

    it('Has Settings, Messaging, and Home buttons', async () => {
      await vpn.waitForElement('navigationLayout/navButton-home');
      await vpn.waitForElementProperty(
          'navigationLayout/navButton-home', 'visible', 'true');

      await vpn.waitForElement('navigationLayout/navButton-messages');
      await vpn.waitForElementProperty(
          'navigationLayout/navButton-messages', 'visible', 'true');

      await vpn.waitForElement('navigationLayout/navButton-settings');
      await vpn.waitForElementProperty(
          'navigationLayout/navButton-settings', 'visible', 'true');
    });


    it('Clicking the Settings button opens settings screen', async () => {
      await vpn.waitForElement('navigationLayout/navButton-settings');
      await vpn.clickOnElement('navigationLayout/navButton-settings');
      await vpn.waitForElement('settingsView');
      assert(
          await vpn.getElementProperty('settingsView', 'visible') === 'true');
    });


    it('Clicking the Messages button opens messaging screen', async () => {
      await vpn.waitForElement('navigationLayout/navButton-messages');
      await vpn.clickOnElement('navigationLayout/navButton-messages');
      await vpn.waitForElement('messageInboxView');
      assert(
          await vpn.getElementProperty('messageInboxView', 'visible') ===
          'true');
    });


    it('Clicking the Home button opens home screen', async () => {
      await vpn.waitForElement('navigationLayout/navButton-messages');
      await vpn.clickOnElement('navigationLayout/navButton-messages');
      await vpn.waitForElement('messageInboxView');

      await vpn.clickOnElement('navigationLayout/navButton-home');
      await vpn.waitForElement('serverListButton');

      assert(
          await vpn.getElementProperty('serverListButton', 'visible') ===
          'true');
    });


    it('Clicking a selected Home button reloads the Home screen', async () => {
      await vpn.waitForElement('serverListButton');
      await vpn.clickOnElement('serverListButton');
      await vpn.waitForElement('serverListBackButton');
      await vpn.waitForElementProperty(
          'serverListBackButton', 'visible', 'true');

      await vpn.clickOnElement('navigationLayout/navButton-home');
      assert(
          await vpn.getElementProperty('serverListButton', 'visible') ===
          'true');
    });


    it('Clicking a selected Settings button reloads the Settings screen',
       async () => {
         await vpn
             .waitForElement('navigationLayout/navButton-settings')
                 await vpn.clickOnElement(
                     'navigationLayout/navButton-settings');

         await vpn.waitForElement('settingsGetHelp');
         await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');
         await vpn.clickOnElement('settingsGetHelp');

         await vpn.waitForElement('getHelpBack');
         await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');

         await vpn.clickOnElement('navigationLayout/navButton-settings');

         assert(
             await vpn.getElementProperty('settingsGetHelp', 'visible') ===
             'true');
       });


    it('Is visible over the post-auth Get help menu', async () => {
      await vpn.waitForElement('navigationLayout/navButton-settings')
          await vpn.clickOnElement('navigationLayout/navButton-settings');

      await vpn.waitForElement('settingsGetHelp');
      await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');
      await vpn.clickOnElement('settingsGetHelp');

      await vpn.waitForElement('getHelpBack');
      await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');

      await vpn.clickOnElement('navigationLayout/navButton-settings');

      assert(await navigationBarVisible() === 'true');
    });
  });

  /* TODOS...
    - selected navbar icon correctly reflects screen opens from systray...
    - not visible during reauth screen...
    what else?
  */
});
