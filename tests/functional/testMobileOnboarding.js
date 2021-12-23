/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');


describe('Mobile Onboarding', function() {
  beforeEach(async () => {
    await vpn.waitForElement('initialStackView');
    await vpn.setElementProperty(
        'initialStackView', 'state', 's', 'testMobileOnboarding');
  });

  it('Navigating to and from the help menu is possible', async () => {
    await vpn.wait();
    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
    await vpn.clickOnElement('getHelpLink');

    await vpn.wait();

    await vpn.waitForElement('getHelpBack');
    await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');
    await vpn.clickOnElement('getHelpBack');
    await vpn.wait();
    await vpn.waitForElement('swipeView');
    assert(await vpn.getElementProperty('swipeView', 'visible') === 'true')
  });

  it('SwipeView is visible', async () => {
    await vpn.wait();
    await vpn.waitForElement('swipeView');
    assert(await vpn.getElementProperty('swipeView', 'visible') === 'true')
  });

  it('Sign up button is visible', async () => {
    await vpn.wait();
    await vpn.waitForElement('signUpButton');
    assert(await vpn.getElementProperty('signUpButton', 'visible') === 'true')
  });

  it('Already a subscriber button is visible', async () => {
    await vpn.wait();
    await vpn.waitForElement('alreadyASubscriberLink');
    assert(
        await vpn.getElementProperty('alreadyASubscriberLink', 'visible') ===
        'true')
  });

  it('Panel title is set correctly based on StackView currentIndex',
     async () => {
       await vpn.wait();
       await vpn.waitForElement('swipeView');
       await vpn.setElementProperty('swipeView', 'currentIndex', 'i', 0);
       await vpn.wait();
       await vpn.waitForElement('panelTitle');
       assert(
           await vpn.getElementProperty('panelTitle', 'text') ===
           'Mozilla VPN');
     });

  it('Panel description is set correctly based on StackView currentIndex',
     async () => {
       await vpn.wait();
       await vpn.waitForElement('swipeView');
       await vpn.setElementProperty('swipeView', 'currentIndex', 'i', 0);
       await vpn.wait();
       const descriptionText =
           await vpn.getElementProperty('panelDescription', 'text');
       assert(descriptionText.includes('Firefox'));
     });

  it('Panel title and description are updated when SwipeView currentIndex changes',
     async () => {
       await vpn.wait();
       await vpn.waitForElement('swipeView');
       await vpn.setElementProperty('swipeView', 'currentIndex', 'i', '2');
       await vpn.wait();
       assert(
           await vpn.getElementProperty('panelTitle', 'text') ===
           'No bandwidth restrictions');
       const descriptionText =
           await vpn.getElementProperty('panelDescription', 'text');
       assert(descriptionText.includes('ISP throttling'));
     });

  it('Sign up button opens auth flow', async () => {
    await vpn.wait();
    await vpn.waitForElement('signUpButton');
    await vpn.clickOnElement('signUpButton');
    await vpn.wait();

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });
    await vpn.waitForElement('authenticatingView');
    await vpn.waitForElementProperty('authenticatingView', 'visible', 'true');
  });

  it('Already a subscriber? opens auth flow', async () => {
    await vpn.wait();
    await vpn.waitForElement('alreadyASubscriberLink');
    await vpn.clickOnElement('alreadyASubscriberLink');
    await vpn.wait();

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });
    await vpn.waitForElement('authenticatingView');
    await vpn.waitForElementProperty('authenticatingView', 'visible', 'true');
  });
});
