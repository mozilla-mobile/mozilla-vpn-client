/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const { actions } = require('./actions.js');
const elements = require('./elements.js');
const vpn = require('./helper.js');
 
describe('Add-ons functionality', function () {
   this.timeout(60000);
   this.ctx.authenticationNeeded = true;
 
   beforeEach(async () => {
        await actions.settings.goToSettings()
        await actions.settings.goToGetHelpView();
        await actions.settings.activateDeveloperOptions()
        await actions.settings.useStagingServers()
        await actions.settings.enableCustomAddOns()
   });   
 
   it('Verify empty manifest add-on', async () => {
    await actions.general.sendText(elements.CUSTOM_ADDON_TEXTFIELD, 'https://bakulf.github.io/vpn-addons-test//empty_manifest/');
    await vpn.wait(3000)
    await vpn.waitForElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.clickOnElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.wait(3000)
   });
   
   it('Verify invalid manifest add-on', async () => {
    await actions.general.sendText(elements.CUSTOM_ADDON_TEXTFIELD, 'https://bakulf.github.io/vpn-addons-test//broken_manifest/');
    await vpn.wait(3000)
    await vpn.waitForElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.clickOnElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.wait(3000)
   });

   it('Verify not found manifest add-on', async () => {
    await actions.general.sendText(elements.CUSTOM_ADDON_TEXTFIELD, 'https://bakulf.github.io/vpn-addons-test//404_manifest/');
    await vpn.wait(3000)
    await vpn.waitForElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.clickOnElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.wait(3000)
   });

   it('Verify one-addOn-only manifest add-on', async () => {
    await actions.general.sendText(elements.CUSTOM_ADDON_TEXTFIELD, 'https://bakulf.github.io/vpn-addons-test//single_manifest/');
    await vpn.wait(3000)
    await vpn.waitForElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.clickOnElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.wait(3000)
   });

   it('Verify manifest with missing add-on files(1 add-on OK, 1 add-on missing) add-on', async () => {
    await actions.general.sendText(elements.CUSTOM_ADDON_TEXTFIELD, 'https://bakulf.github.io/vpn-addons-test//missing_addon/');
    await vpn.wait(3000)
    await vpn.waitForElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.clickOnElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.wait(3000)
   });

   it('Verify invalid hash add-on', async () => {
    await actions.general.sendText(elements.CUSTOM_ADDON_TEXTFIELD, 'https://bakulf.github.io/vpn-addons-test//broken_addon/');
    await vpn.wait(3000)
    await vpn.waitForElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.clickOnElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.wait(3000)
   });

   it('Verify one add-on message add-on', async () => {    
    await actions.general.sendText(elements.CUSTOM_ADDON_TEXTFIELD, 'https://bakulf.github.io/vpn-addons-test//messages_1/');
    await vpn.waitForElementAndClick(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.hasElement(elements.MESSAGE_INBOX_VIEW);
    await vpn.wait(4000);
    await vpn.waitForElementAndClick(elements.NAVIGATION_BAR_MESSAGES);
    assert(await vpn.toMatchSnapshot(this.tests[0].title));
   });

   it('Verify two add-on message', async () => {    
    await actions.general.sendText(elements.CUSTOM_ADDON_TEXTFIELD, 'https://bakulf.github.io/vpn-addons-test//messages_2/');
    await vpn.waitForElementAndClick(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.hasElement(elements.MESSAGE_INBOX_VIEW);
    await vpn.wait(4000);
    await vpn.waitForElementAndClick(elements.NAVIGATION_BAR_MESSAGES);
    assert(await vpn.toMatchSnapshot(this.tests[0].title));
   });

   it('Verify three add-on message', async () => {
    await actions.general.sendText(elements.CUSTOM_ADDON_TEXTFIELD, 'https://bakulf.github.io/vpn-addons-test//messages_3/');
    await vpn.waitForElementAndClick(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.hasElement(elements.MESSAGE_INBOX_VIEW);
    await vpn.wait(4000);
    await vpn.waitForElementAndClick(elements.NAVIGATION_BAR_MESSAGES);
    assert(await vpn.toMatchSnapshot(this.tests[0].title));
   });

   it.only('Verify four add-on message', async () => {
    await actions.general.sendText(elements.CUSTOM_ADDON_TEXTFIELD, 'https://bakulf.github.io/vpn-addons-test//messages_4/');
    await vpn.waitForElementAndClick(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.hasElement(elements.MESSAGE_INBOX_VIEW);
    await vpn.wait(4000);
    await vpn.waitForElementAndClick(elements.NAVIGATION_BAR_MESSAGES);
    console.log('line 102');
    assert(await vpn.toMatchSnapshot(this.tests[0].title));
   });

   it('Verify messages with locale conditions', async () => {
    await actions.general.sendText(elements.CUSTOM_ADDON_TEXTFIELD, 'https://bakulf.github.io/vpn-addons-test//messages_locale/');
    await vpn.wait(3000)
    await vpn.waitForElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.clickOnElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.wait(3000)
   });

   it('Verify messages with dates', async () => {
    await actions.general.sendText(elements.CUSTOM_ADDON_TEXTFIELD, 'https://bakulf.github.io/vpn-addons-test//messages_dates/');
    await vpn.waitForElementAndClick(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.hasElement(elements.MESSAGE_INBOX_VIEW);
    await vpn.wait(4000);
    await vpn.waitForElementAndClick(elements.NAVIGATION_BAR_MESSAGES);
    assert(await vpn.toMatchSnapshot(this.tests[0].title));
   });

   it('Verify advanced tips and tricks', async () => {
    await actions.general.sendText(elements.CUSTOM_ADDON_TEXTFIELD, 'https://bakulf.github.io/vpn-addons-test//advanced_tips_and_tricks/');
    await vpn.wait(3000)
    await vpn.waitForElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.clickOnElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.wait(3000)
   });

   it('Verify privacy bundle message', async () => {
    await actions.general.sendText(elements.CUSTOM_ADDON_TEXTFIELD, 'https://bakulf.github.io/vpn-addons-test//messages_bundle/');
    await vpn.wait(3000)
    await vpn.waitForElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.clickOnElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.wait(3000)
   });

   it('Verify long title add-on', async () => {
    await actions.general.sendText(elements.CUSTOM_ADDON_TEXTFIELD, 'https://bakulf.github.io/vpn-addons-test//messages_long/');
    await vpn.wait(3000)
    await vpn.waitForElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.clickOnElement(elements.NAVIGATION_BAR_MESSAGES);
    await vpn.wait(3000)
   });
});
 