/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');
const queries = require('./queries.js');

describe('Tutorials', function () {
  this.timeout(60000);
  this.ctx.authenticationNeeded = true;

  async function openHighlightedTutorial() {
    await vpn.clickOnQuery(queries.navBar.SETTINGS);
    await vpn.waitForQueryAndClick(queries.screenSettings.TIPS_AND_TRICKS);
    await vpn.waitForQueryAndClick(
        queries.screenSettings.TUTORIAL_LIST_HIGHLIGHT);
  }

  async function clickTooltipCloseButton() {
    await vpn.waitForQueryAndClick(queries.screenHome.TUTORIAL_LEAVE.visible());
  }

  describe('Tutorial tooltip', function() {
    beforeEach(async () => {
      await vpn.resetAddons('04_tutorials_basic');
      await openHighlightedTutorial();
    });

    it('Has close button', async () => {
      await vpn.waitForQuery(queries.screenHome.TUTORIAL_LEAVE.visible());
    });

    it('Clicking close button opens the "Leave tutorial?" modal', async () => {
      await clickTooltipCloseButton();

      await vpn.waitForQuery(
          queries.screenHome.TUTORIAL_POPUP_PRIMARY_BUTTON.visible());

      assert.equal(
          (await vpn.getQueryProperty(
              queries.screenHome.TUTORIAL_POPUP_PRIMARY_BUTTON, 'text')),
          'Resume tutorial');
    });
  });

  describe('"Leave tutorial?" popup', function() {
    beforeEach(async () => {
      await vpn.resetAddons('04_tutorials_basic');
      await openHighlightedTutorial();
      await clickTooltipCloseButton();
    });

    it('Clicking primary button closes modal and resumes tutorial',
       async () => {
         await vpn.waitForQueryAndClick(
             queries.screenHome.TUTORIAL_POPUP_PRIMARY_BUTTON.visible());
         await vpn.waitForQuery(queries.screenHome.TUTORIAL_UI.visible());
       });

    it('Clicking secondary button closes modal and stops tutorial',
       async () => {
         await vpn.waitForQueryAndClick(
             queries.screenHome.TUTORIAL_POPUP_SECONDARY_BUTTON.visible());
         await vpn.waitForQuery(queries.screenHome.TUTORIAL_UI.hidden());
       });
  });
});
