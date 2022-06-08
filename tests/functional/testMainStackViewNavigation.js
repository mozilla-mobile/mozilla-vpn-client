/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');

describe('mainStackView navigation', function() {
  async function openContactUs() {
    await vpn.wait();
    await vpn.openContactUs();
    await vpn.wait();
    await vpn.waitForElement('contactUs');
    await vpn.waitForElementProperty('contactUs', 'visible', 'true');
    assert(await vpn.getElementProperty('contactUs', 'visible') === 'true');
  }

  async function closeContactUs() {
    await vpn.wait();
    await vpn.waitForElement('supportTicketScreen');
    await vpn.waitForElementProperty('supportTicketScreen', 'visible', 'true');
    await vpn.clickOnElement('supportTicketScreen');
    await vpn.wait();
  }

  async function getMainStackViewDepth() {
    await vpn.waitForElement('MainStackView');
    return await vpn.getElementProperty('MainStackView', 'depth');
  }

  it('onContactUsNeeded(): Stack history is preserved pre-authentication',
     async () => {
       await vpn.waitForMainView();
       await vpn.wait();
       await vpn.clickOnElement('learnMoreLink');
       await vpn.waitForElement('skipOnboarding');
       await vpn.waitForElementProperty('skipOnboarding', 'visible', 'true');
       await openContactUs();
       await closeContactUs();
       await vpn.waitForElement('skipOnboarding');
       await vpn.waitForElementProperty('skipOnboarding', 'visible', 'true');
       assert(
           await vpn.getElementProperty('skipOnboarding', 'visible') ===
           'true');
     });

  describe('Post-auth tests', function() {
    this.ctx.authenticationNeeded = true;

    it('onContactUsNeeded(): Stack history is preserved post-authentication',
       async () => {
         await vpn.waitForElement('deviceListButton');
         await vpn.waitForElementProperty(
             'deviceListButton', 'visible', 'true');
         await vpn.clickOnElement('deviceListButton');
         await vpn.wait();
         await openContactUs();
         await closeContactUs();
         await vpn.wait();
         await vpn.waitForElement('deviceListBackButton');
         await vpn.waitForElementProperty(
             'deviceListBackButton', 'visible', 'true');
         assert(
             await vpn.getElementProperty('deviceListBackButton', 'visible') ===
             'true');
       });

    it('onSettingsNeeded(): Settings view is pushed to mainStackView',
       async () => {
         const mainStackDepth = await getMainStackViewDepth();
         await vpn.wait();
         await vpn.openSettings();
         await vpn.wait();
         await vpn.waitForElement('settingsBackButton');
         await vpn.waitForElementProperty(
             'settingsBackButton', 'visible', 'true');
         assert(
             await getMainStackViewDepth() ===
             (parseInt(mainStackDepth) + 1).toString());
       });


    it('onSettingsNeeded(): Stack history is preserved after closing Settings',
       async () => {
         await vpn.waitForElement('deviceListButton');
         await vpn.waitForElementProperty(
             'deviceListButton', 'visible', 'true');
         await vpn.clickOnElement('deviceListButton');

         await vpn.waitForElement('deviceListBackButton');
         await vpn.waitForElementProperty(
             'deviceListBackButton', 'visible', 'true');
         await vpn.openSettings();
         await vpn.waitForElement('settingsBackButton');
         await vpn.waitForElementProperty(
             'settingsBackButton', 'visible', 'true');
         await vpn.clickOnElement('settingsBackButton');

         await vpn.waitForElement('deviceListBackButton');
         await vpn.waitForElementProperty(
             'deviceListBackButton', 'visible', 'true');
         assert(
             await vpn.getElementProperty('deviceListBackButton', 'visible') ===
             'true');
       });
  });
});
