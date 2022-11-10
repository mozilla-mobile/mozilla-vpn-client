/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const { homeScreen } = require('./elements.js');
const vpn = require('./helper.js');

describe('Tips and tricks intro modal', function () {
  this.timeout(60000);

  beforeEach(async () => {
    await vpn.setSetting('tips-and-tricks-intro-shown', 'false');
    await vpn.authenticateInApp(true, true);
  });

  it('Access and navigate tips and tricks via intro modal', async () => {
    await vpn.wait();
    await vpn.waitForElementProperty(homeScreen.TIPS_AND_TRICKS_POPUP_LOADER, 'active', 'true');
    await vpn.waitForElement(homeScreen.TIPS_AND_TRICKS_POPUP_DISCOVER);
    await vpn.clickOnElement(homeScreen.TIPS_AND_TRICKS_POPUP_DISCOVER);

    await vpn.wait();
    await vpn.waitForElement(homeScreen.TIPS_AND_TRICKS_VIEW);
    await vpn.waitForElement(homeScreen.HOME_SCREEN_BACK);
    await vpn.clickOnElement(homeScreen.HOME_SCREEN_BACK);    

    await vpn.wait();
    await vpn.waitForElement(homeScreen.HOME_SCREEN);
  });

  it('Closing modal with close button', async () => {
    await vpn.wait();
    await vpn.waitForElementProperty(homeScreen.TIPS_AND_TRICKS_POPUP_LOADER, 'active', 'true');
    await vpn.waitForElement(homeScreen.TIPS_AND_TRICKS_POPUP_CLOSE);
    await vpn.clickOnElement(homeScreen.TIPS_AND_TRICKS_POPUP_CLOSE);

    await vpn.wait();
    await vpn.waitForElementProperty(homeScreen.TIPS_AND_TRICKS_POPUP_LOADER, 'active', 'false');
  });

  it('Closing modal with go back button', async () => {
    await vpn.wait();
    await vpn.waitForElementProperty(homeScreen.TIPS_AND_TRICKS_POPUP_LOADER, 'active', 'true');
    await vpn.waitForElement(homeScreen.TIPS_AND_TRICKS_POPUP_BACK);
    await vpn.clickOnElement(homeScreen.TIPS_AND_TRICKS_POPUP_BACK);

    await vpn.wait();
    await vpn.waitForElementProperty(homeScreen.TIPS_AND_TRICKS_POPUP_LOADER, 'active', 'false');
  });
})
