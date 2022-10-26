/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const { initialScreen, mobileOnBoardingScreen, getHelpScreen } = require('./elements.js');
const vpn = require('./helper.js');


describe('Mobile Onboarding', function() {
  beforeEach(async () => {
    await vpn.waitForElement(initialScreen.SCREEN);
    await vpn.flipFeatureOn('mobileOnboarding');
  });

  it('Navigating to and from the help menu is possible', async () => {
    await vpn.waitForElementAndClick(initialScreen.GET_HELP_LINK);
    await vpn.waitForElementAndClick(getHelpScreen.BACK);
       
    await vpn.waitForElement(mobileOnBoardingScreen.SWIPE_VIEW);
    assert(await vpn.getElementProperty(mobileOnBoardingScreen.SWIPE_VIEW, 'visible') === 'true')
  });

  it('SwipeView is visible', async () => {    
    await vpn.waitForElement(mobileOnBoardingScreen.SWIPE_VIEW);
    assert(await vpn.getElementProperty(mobileOnBoardingScreen.SWIPE_VIEW, 'visible') === 'true')
  });

  it('Sign up button is visible', async () => {
    await vpn.waitForElement(mobileOnBoardingScreen.SIGNUP_BUTTON);
    assert(await vpn.getElementProperty(mobileOnBoardingScreen.SIGNUP_BUTTON, 'visible') === 'true')
  });

  it('Already a subscriber button is visible', async () => {    
    await vpn.waitForElement(mobileOnBoardingScreen.ALREADY_SUBBED_LINK);
    assert(await vpn.getElementProperty(mobileOnBoardingScreen.ALREADY_SUBBED_LINK, 'visible') === 'true')
  });

  it('Panel title is set correctly based on StackView currentIndex',
     async () => {
       await vpn.waitForElement(mobileOnBoardingScreen.SWIPE_VIEW);
       await vpn.setElementProperty(mobileOnBoardingScreen.SWIPE_VIEW, 'currentIndex', 'i', 0);
       await vpn.wait();
       await vpn.waitForElement(mobileOnBoardingScreen.PANEL_TITLE);
       assert(await vpn.getElementProperty(mobileOnBoardingScreen.PANEL_TITLE, 'text') === 'Mozilla VPN');
     });

  it('Panel description is set correctly based on StackView currentIndex',
     async () => {
       await vpn.waitForElement(mobileOnBoardingScreen.SWIPE_VIEW);
       await vpn.setElementProperty(mobileOnBoardingScreen.SWIPE_VIEW, 'currentIndex', 'i', 0);
       await vpn.wait();
       const descriptionText = await vpn.getElementProperty(mobileOnBoardingScreen.PANEL_DESCRIPTION, 'text');
       assert(descriptionText.includes('Firefox'));
     });

  it('Panel title and description are updated when SwipeView currentIndex changes',
     async () => {       
       await vpn.waitForElement(mobileOnBoardingScreen.SWIPE_VIEW);
       await vpn.setElementProperty(mobileOnBoardingScreen.SWIPE_VIEW, 'currentIndex', 'i', '2');
       await vpn.wait();
       assert(await vpn.getElementProperty(mobileOnBoardingScreen.PANEL_TITLE, 'text') === 'Protect your privacy');
       const descriptionText = await vpn.getElementProperty(mobileOnBoardingScreen.PANEL_DESCRIPTION, 'text');
       assert(descriptionText.includes('Route your activity and location'));
     });

  it('Sign up button opens auth flow', async () => {
    await vpn.wait();
    await vpn.waitForElement(mobileOnBoardingScreen.SIGNUP_BUTTON);
    await vpn.clickOnElement(mobileOnBoardingScreen.SIGNUP_BUTTON);
    await vpn.wait();

    if (!this.ctx.wasm) {
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });
    }

    await vpn.waitForElement(initialScreen.AUTHENTICATE_VIEW);
    await vpn.waitForElementProperty(initialScreen.AUTHENTICATE_VIEW, 'visible', 'true');
  });

  it('Already a subscriber? opens auth flow', async () => {
    await vpn.waitForElementAndClick(mobileOnBoardingScreen.ALREADY_SUBBED_LINK);

    if (!this.ctx.wasm) {
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });
    }

    await vpn.waitForElement(initialScreen.AUTHENTICATE_VIEW);
    await vpn.waitForElementProperty(initialScreen.AUTHENTICATE_VIEW, 'visible', 'true');
  });
});
