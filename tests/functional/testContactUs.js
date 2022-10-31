/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
const vpn = require('./helper.js');
const assert = require('assert');
const { initialScreen, settingsScreen, navBar, getHelpScreen } = require('./elements.js');


describe('Contact us view', function() {
  it('VPNUserInfo is not visible to unathenticated users', async () => {
    await vpn.waitForElementAndClick(initialScreen.GET_HELP_LINK);
    await vpn.waitForElement(getHelpScreen.LINKS);
    await vpn.waitForElementProperty(getHelpScreen.LINKS, 'visible', 'true');

    await vpn.waitForElementAndClick(getHelpScreen.SUPPORT);

    await vpn.waitForElement(getHelpScreen.contactSupportView.USER_INFO);
    await vpn.waitForElementProperty(getHelpScreen.contactSupportView.USER_INFO, 'visible', 'false');
    assert(
        await vpn.getElementProperty(getHelpScreen.contactSupportView.USER_INFO, 'visible') ===
        'false');
  });

  it('Email inputs are visible to unauthenticated user', async () => {
    await vpn.waitForElementAndClick(initialScreen.GET_HELP_LINK);
    await vpn.waitForElement(getHelpScreen.LINKS);
    await vpn.waitForElementProperty(getHelpScreen.LINKS, 'visible', 'true');

    await vpn.waitForElementAndClick(getHelpScreen.SUPPORT);

    await vpn.waitForElement(getHelpScreen.contactSupportView.UNAUTH_USER_INPUTS);
    await vpn.waitForElementProperty(getHelpScreen.contactSupportView.UNAUTH_USER_INPUTS, 'visible', 'true');
    assert(await vpn.getElementProperty(getHelpScreen.contactSupportView.UNAUTH_USER_INPUTS, 'visible') === 'true');
  });

  describe('Contact us view - authenticated user', function() {
    this.ctx.authenticationNeeded = true;

    async function openContactUsInSettings() {      
      await vpn.waitForElementAndClick(navBar.SETTINGS);

      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');

      await vpn.waitForElement(settingsScreen.GET_HELP);
      await vpn.waitForElementProperty(settingsScreen.GET_HELP, 'visible', 'true');
      await vpn.clickOnElement(settingsScreen.GET_HELP);

      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');
      await vpn.waitForElementProperty(getHelpScreen.LINKS, 'visible', 'true');
      await vpn.waitForElementAndClick(getHelpScreen.SUPPORT);
    }

    it('VPNUserInfo visible to authenticated users', async () => {
      await openContactUsInSettings();

      await vpn.waitForElement(getHelpScreen.contactSupportView.USER_INFO);
      await vpn.waitForElementProperty(getHelpScreen.contactSupportView.USER_INFO, 'visible', 'true');
      assert(await vpn.getElementProperty(getHelpScreen.contactSupportView.USER_INFO, 'visible') === 'true');
    });

    it('VPNUserInfo is disabled', async () => {
      await openContactUsInSettings();

      await vpn.waitForElement(getHelpScreen.contactSupportView.USER_INFO);
      await vpn.waitForElementProperty(
          getHelpScreen.contactSupportView.USER_INFO, 'enabled', 'false');
      assert(
          await vpn.getElementProperty(getHelpScreen.contactSupportView.USER_INFO, 'enabled') ===
          'false');
    });

    it('Email inputs are not visible to authenticated user', async () => {
      it('Email inputs are visible to unauthenticated user', async () => {
        await openContactUsInSettings();

        await vpn.waitForElement(getHelpScreen.contactSupportView.UNAUTH_USER_INPUTS);
        await vpn.waitForElementProperty(getHelpScreen.contactSupportView.UNAUTH_USER_INPUTS, 'visible', 'false');
        assert(await vpn.getElementProperty(getHelpScreen.contactSupportView.UNAUTH_USER_INPUTS, 'visible') === 'false');
      });
    });
  });
});
