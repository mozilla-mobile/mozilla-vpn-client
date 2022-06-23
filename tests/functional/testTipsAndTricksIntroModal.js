/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 const assert = require('assert');
 const vpn = require('./helper.js');
 

describe('Tips and tricks intro modal', function() {
    this.timeout(60000);

    beforeEach(async () => {
        await vpn.setSetting('tips-and-tricks-intro-shown', 'false')
        await vpn.authenticateInApp(true, true)
      });

    it('Access and navigate tips and tricks via intro modal', async () => {
        await vpn.wait()
        await vpn.waitForElementProperty('tipsAndTricksIntroPopupLoader', 'active', 'true');
        await vpn.waitForElement('tipsAndTricksIntroPopupDiscoverNowButton');
        await vpn.clickOnElement('tipsAndTricksIntroPopupDiscoverNowButton');

        await vpn.wait()
        await vpn.waitForElement('settingsTipsAndTricksPage');
        await vpn.waitForElement('tipsAndTricksCloseButton');
        await vpn.clickOnElement('tipsAndTricksCloseButton');

        await vpn.wait()
        await vpn.waitForElement('ViewMainStackView');
    });

    it('Closing modal with close button', async () => {
        await vpn.wait()
        await vpn.waitForElementProperty('tipsAndTricksIntroPopupLoader', 'active', 'true');
        await vpn.waitForElement('tipsAndTricksIntroPopupCloseButton');
        await vpn.clickOnElement('tipsAndTricksIntroPopupCloseButton');

        await vpn.wait()
        await vpn.waitForElementProperty('tipsAndTricksIntroPopupLoader', 'active', 'false');
    });

    it('Closing modal with go back button', async () => {
        await vpn.wait()
        await vpn.waitForElementProperty('tipsAndTricksIntroPopupLoader', 'active', 'true');
        await vpn.waitForElement('tipsAndTricksIntroPopupGoBackButton');
        await vpn.clickOnElement('tipsAndTricksIntroPopupGoBackButton');
        
        await vpn.wait()
        await vpn.waitForElementProperty('tipsAndTricksIntroPopupLoader', 'active', 'false');
    });
      

})