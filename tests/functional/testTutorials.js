/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const { navBar, settingsScreen, homeScreen } = require('./elements.js');
const vpn = require('./helper.js');

describe('Tutorials', function () {
  this.timeout(60000);
  this.ctx.authenticationNeeded = true;

  async function openHighlightedTutorial() {
    await vpn.waitForElementAndClick(navBar.SETTINGS);
    await vpn.waitForElementAndClick(settingsScreen.TIPS_AND_TRICKS);  
    await vpn.waitForElementAndClick(homeScreen.TUTORIAL_LIST_HIGHLIGHT);
  }

  async function clickTooltipCloseButton() {
    await vpn.waitForElementAndClick(homeScreen.TUTORIAL_LEAVE);
  }

  async function loadAddons() {
    await vpn.resetAddons('04_tutorials_basic');
  }

  describe('Tutorial tooltip', function() {
    beforeEach(async () => {
      await loadAddons();
      await openHighlightedTutorial();
    });

    it('Has close button', async () => {
      await vpn.waitForCondition(async () => {
        return await vpn.getElementProperty(homeScreen.TUTORIAL_LEAVE, 'visible') === 'true'
      });

      assert((await vpn.getElementProperty(homeScreen.TUTORIAL_LEAVE, 'visible')) === 'true');
    });

    it('Clicking close button opens the "Leave tutorial?" modal', async () => {
      await clickTooltipCloseButton();      
      await vpn.waitForElementProperty(homeScreen.TUTORIAL_POPUP_PRIMARY_BUTTON, 'visible', 'true');

      assert(
        (await vpn.getElementProperty(homeScreen.TUTORIAL_POPUP_PRIMARY_BUTTON, 'text')) === 'Resume tutorial');
    });
  });

  describe('"Leave tutorial?" popup', function() {
    beforeEach(async () => {
      await openHighlightedTutorial();
      await vpn.wait();
      await clickTooltipCloseButton();
    });

    it('Clicking primary button closes modal and resumes tutorial',
      async () => {
        await vpn.waitForElementProperty(homeScreen.TUTORIAL_POPUP_PRIMARY_BUTTON, 'visible', 'true');
        await vpn.waitForElementAndClick(homeScreen.TUTORIAL_POPUP_PRIMARY_BUTTON);

        assert((await vpn.getElementProperty(homeScreen.TUTORIAL_POPUP_PRIMARY_BUTTON, 'visible')) === 'false');
        assert((await vpn.getElementProperty(homeScreen.TUTORIAL_UI, 'visible')) === 'true');
      });

    it('Clicking secondary button closes modal and stops tutorial',
      async () => {
        await vpn.waitForElementProperty(homeScreen.TUTORIAL_POPUP_SECONDARY_BUTTON, 'visible', 'true');
        await vpn.waitForElementAndClick(homeScreen.TUTORIAL_POPUP_SECONDARY_BUTTON);

        assert((await vpn.getElementProperty(homeScreen.TUTORIAL_UI, 'visible')) === 'false');
      });
  });
});
