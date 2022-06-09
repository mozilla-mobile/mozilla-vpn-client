/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
const assert = require('assert');
const vpn = require('./helper.js');

describe('Tutorials', function() {
  this.timeout(60000);
  this.ctx.authenticationNeeded = true;

  async function openHighlightedTutorial() {
    await vpn.waitForElement('settingsButton');
    await vpn.clickOnElement('settingsButton');
    await vpn.wait();

    await vpn.waitForElement('settingsTipsAndTricks');
    await vpn.clickOnElement('settingsTipsAndTricks');
    await vpn.wait();

    await vpn.waitForElement('highlightedTutorial');
    await vpn.clickOnElement('highlightedTutorial');
    await vpn.wait();
  }

  async function clickTooltipCloseButton() {
    await vpn.waitForElement('tutorialLeave');
    await vpn.clickOnElement('tutorialLeave');
  }

  describe('Tutorial tooltip', function() {
    beforeEach(async () => {
      await openHighlightedTutorial();
    });

    it('Has close button', async () => {
      await vpn.waitForElement('tutorialLeave');
      assert(
          (await vpn.getElementProperty('tutorialLeave', 'visible')) ===
          'true');
    });

    it('Clicking close button opens the "Leave tutorial?" modal', async () => {
      await clickTooltipCloseButton();
      await vpn.wait();

      await vpn.wait();
      await vpn.waitForElementProperty(
          'tutorialPopupPrimaryButton', 'visible', 'true');

      assert(
          (await vpn.getElementProperty(
              'tutorialPopupPrimaryButton', 'text')) === 'Resume tutorial');
    });
  });

  describe('"Leave tutorial?" popup', function() {
    beforeEach(async () => {
      await openHighlightedTutorial();
      await vpn.wait();
      await clickTooltipCloseButton();
      await vpn.wait();
    });

    it('Clicking primary button closes modal and resumes tutorial',
       async () => {
         await vpn.waitForElementProperty(
             'tutorialPopupPrimaryButton', 'visible', 'true');
         await vpn.clickOnElement('tutorialPopupPrimaryButton');
         await vpn.wait();
         assert(
             (await vpn.getElementProperty(
                 'tutorialPopupPrimaryButton', 'visible')) === 'false');

         assert(
             (await vpn.getElementProperty('tutorialUiRoot', 'visible')) ===
             'true');
       });

    it('Clicking secondary button closes modal and stops tutorial',
       async () => {
         await vpn.waitForElementProperty(
             'tutorialPopupSecondaryButton', 'visible', 'true');
         await vpn.clickOnElement('tutorialPopupSecondaryButton');
         await vpn.wait();

         assert(
             (await vpn.getElementProperty('tutorialUiRoot', 'visible')) ===
             'false');
       });
  });

  describe(
      'Opening views from system tray menu triggers "Leave tutorial?" popup',
      function() {
        beforeEach(async () => {
          await openHighlightedTutorial();
        });

        it('Opening settings from system tray during tutorial triggers "Leave tutorial?" popup',
           async () => {
             await vpn.wait();
             await vpn.openSettings();
             await vpn.wait();
             assert(
                 (await vpn.getElementProperty(
                     'tutorialPopupPrimaryButton', 'visible')) === 'true');
           });

        it('Clicking secondary button opens clicked system tray option',
           async () => {
             await vpn.wait();
             await vpn.openSettings();
             await vpn.wait();

             await vpn.waitForElementProperty(
                 'tutorialPopupSecondaryButton', 'visible', 'true');
             await vpn.clickOnElement('tutorialPopupSecondaryButton');
             await vpn.wait();

             await vpn.waitForElementProperty(
                 'tutorialUiRoot', 'visible', 'false');
             assert(
                 (await vpn.getElementProperty('tutorialUiRoot', 'visible')) ===
                 'false');

             await vpn.waitForElement('settings');
             assert(
                 (await vpn.getElementProperty('settings', 'visible')) ===
                 'true');
           });
      });
});
