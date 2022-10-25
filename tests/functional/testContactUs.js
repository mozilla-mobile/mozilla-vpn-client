/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
const vpn = require('./helper.js');
const assert = require('assert');
const { initialView, settingsView, navBar } = require('./elements.js');


describe('Contact us view', function() {
  it('VPNUserInfo is not visible to unathenticated users', async () => {
    await vpn.waitForElementAndClick(initialView.GET_HELP_LINK);
    await vpn.waitForElement(settingsView.getHelpView.LINKS);
    await vpn.waitForElementProperty(settingsView.getHelpView.LINKS, 'visible', 'true');

    await vpn.waitForElementAndClick(settingsView.getHelpView.SUPPORT);

    await vpn.waitForElement(settingsView.getHelpView.contactSupportView.USER_INFO);
    await vpn.waitForElementProperty(settingsView.getHelpView.contactSupportView.USER_INFO, 'visible', 'false');
    assert(
        await vpn.getElementProperty(settingsView.getHelpView.contactSupportView.USER_INFO, 'visible') ===
        'false');
  });

  it('Email inputs are visible to unauthenticated user', async () => {
    await vpn.waitForElementAndClick(initialView.GET_HELP_LINK);
    await vpn.waitForElement(settingsView.getHelpView.LINKS);
    await vpn.waitForElementProperty(settingsView.getHelpView.LINKS, 'visible', 'true');

    await vpn.waitForElementAndClick(settingsView.getHelpView.SUPPORT);

    await vpn.waitForElement(settingsView.getHelpView.contactSupportView.UNAUTH_USER_INPUTS);
    await vpn.waitForElementProperty(settingsView.getHelpView.contactSupportView.UNAUTH_USER_INPUTS, 'visible', 'true');
    assert(await vpn.getElementProperty(settingsView.getHelpView.contactSupportView.UNAUTH_USER_INPUTS, 'visible') === 'true');
  });

  describe('Contact us view - authenticated user', function() {
    this.ctx.authenticationNeeded = true;

    async function openContactUsInSettings() {      
      await vpn.waitForElementAndClick(navBar.SETTINGS);

      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');

      await vpn.waitForElement(settingsView.GET_HELP);
      await vpn.waitForElementProperty(settingsView.GET_HELP, 'visible', 'true');
      await vpn.clickOnElement(settingsView.GET_HELP);

      await vpn.waitForElementProperty('screenLoader', 'busy', 'false');
      await vpn.waitForElementProperty(settingsView.getHelpView.LINKS, 'visible', 'true');
      await vpn.waitForElementAndClick(settingsView.getHelpView.SUPPORT);
    }

    it('VPNUserInfo visible to authenticated users', async () => {
      await openContactUsInSettings();

      await vpn.waitForElement(settingsView.getHelpView.contactSupportView.USER_INFO);
      await vpn.waitForElementProperty(settingsView.getHelpView.contactSupportView.USER_INFO, 'visible', 'true');
      assert(await vpn.getElementProperty(settingsView.getHelpView.contactSupportView.USER_INFO, 'visible') === 'true');
    });

    it('VPNUserInfo is disabled', async () => {
      await openContactUsInSettings();

      await vpn.waitForElement(settingsView.getHelpView.contactSupportView.USER_INFO);
      await vpn.waitForElementProperty(
          settingsView.getHelpView.contactSupportView.USER_INFO, 'enabled', 'false');
      assert(
          await vpn.getElementProperty(settingsView.getHelpView.contactSupportView.USER_INFO, 'enabled') ===
          'false');
    });

    it('Email inputs are not visible to authenticated user', async () => {
      it('Email inputs are visible to unauthenticated user', async () => {
        await openContactUsInSettings();

        await vpn.waitForElement(settingsView.getHelpView.contactSupportView.UNAUTH_USER_INPUTS);
        await vpn.waitForElementProperty(settingsView.getHelpView.contactSupportView.UNAUTH_USER_INPUTS, 'visible', 'false');
        assert(await vpn.getElementProperty(settingsView.getHelpView.contactSupportView.UNAUTH_USER_INPUTS, 'visible') === 'false');
      });
    });
  });
});
