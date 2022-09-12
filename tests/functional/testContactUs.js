/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
const vpn = require('./helper.js');
const assert = require('assert');


describe('Contact us view', function() {
  it('VPNUserInfo is not visible to unathenticated users', async () => {
    await vpn.clickOnElement('getHelpLink');
    await vpn.waitForElement('getHelpLinks');
    await vpn.waitForElementProperty('getHelpLinks', 'visible', 'true');

    await vpn.clickOnElement('inAppSupport');

    await vpn.waitForElement('contactUs-userInfo');
    await vpn.waitForElementProperty('contactUs-userInfo', 'visible', 'false');
    assert(
        await vpn.getElementProperty('contactUs-userInfo', 'visible') ===
        'false');
  });

  it('Email inputs are visible to unauthenticated user', async () => {
    await vpn.clickOnElement('getHelpLink');
    await vpn.waitForElement('getHelpLinks');
    await vpn.waitForElementProperty('getHelpLinks', 'visible', 'true');

    await vpn.clickOnElement('inAppSupport');

    await vpn.waitForElement('contactUs-unauthedUserInputs');
    await vpn.waitForElementProperty(
        'contactUs-unauthedUserInputs', 'visible', 'true');
    assert(
        await vpn.getElementProperty(
            'contactUs-unauthedUserInputs', 'visible') === 'true');
  });

  describe('Contact us view - authenticated user', function() {
    this.ctx.authenticationNeeded = true;

    async function openContactUsInSettings() {
      await vpn.waitForElement('navigationLayout/navButton-settings');
      await vpn.clickOnElement('navigationLayout/navButton-settings');
      await vpn.wait();

      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');

      await vpn.waitForElement('settingsGetHelp');
      await vpn.waitForElementProperty('settingsGetHelp', 'visible', 'true');
      await vpn.clickOnElement('settingsGetHelp');

      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');
      await vpn.waitForElementProperty('getHelpLinks', 'visible', 'true');
      await vpn.clickOnElement('inAppSupport');
    }

    it('VPNUserInfo visible to authenticated users', async () => {
      await openContactUsInSettings();

      await vpn.waitForElement('contactUs-userInfo');
      await vpn.waitForElementProperty('contactUs-userInfo', 'visible', 'true');
      assert(
          await vpn.getElementProperty('contactUs-userInfo', 'visible') ===
          'true');
    });

    it('VPNUserInfo is disabled', async () => {
      await openContactUsInSettings();

      await vpn.waitForElement('contactUs-userInfo');
      await vpn.waitForElementProperty(
          'contactUs-userInfo', 'enabled', 'false');
      assert(
          await vpn.getElementProperty('contactUs-userInfo', 'enabled') ===
          'false');
    });

    it('Email inputs are not visible to authenticated user', async () => {
      it('Email inputs are visible to unauthenticated user', async () => {
        await openContactUsInSettings();

        await vpn.waitForElement('contactUs-unauthedUserInputs');
        await vpn.waitForElementProperty(
            'contactUs-unauthedUserInputs', 'visible', 'false');
        assert(
            await vpn.getElementProperty(
                'contactUs-unauthedUserInputs', 'visible') === 'false');
      });
    });
  });
});
