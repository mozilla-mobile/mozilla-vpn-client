/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');

describe('Initial view and onboarding', function() {
  this.timeout(240000);

  beforeEach(async () => {
    assert(await vpn.getLastUrl() === '');
  })

  it('Check for links on mainView', async () => {
    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
    assert(await vpn.getElementProperty('getStarted', 'visible') === 'true');
    assert(await vpn.getElementProperty('learnMoreLink', 'visible') === 'true');
  });

  it('Open the help menu', async () => {
    await vpn.clickOnElement('getHelpLink');
    await vpn.waitForElement('getHelpBack');
    await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');
  });

  it('Open help links', async () => {
    await vpn.clickOnElement('getHelpLink');
    await vpn.waitForElement('getHelpLinks');
    await vpn.waitForElementProperty('getHelpLinks', 'visible', 'true');

    await vpn.waitForElement('helpCenter');
    await vpn.waitForElementProperty('helpCenter', 'visible', 'true');

    await vpn.waitForElement('inAppSupport');
    await vpn.waitForElementProperty('inAppSupport', 'visible', 'true');

    await vpn.waitForElement('viewLogs');
    await vpn.waitForElementProperty('viewLogs', 'visible', 'true');

    await vpn.clickOnElement('viewLogs');
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.startsWith('file://') && url.includes('mozillavpn') &&
          url.endsWith('.txt');
    });

    await vpn.clickOnElement('helpCenter');
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/support');
    });

    await vpn.clickOnElement('inAppSupport');
    await vpn.waitForElement('contactUs-unauthedUserInputs');
    await vpn.waitForElementProperty(
        'contactUs-unauthedUserInputs', 'visible', 'true');
  });

  it('Complete the onboarding (aborting in each phase)', async () => {
    let onboardingView = 0;

    while (true) {
      assert(
          await vpn.getElementProperty('learnMoreLink', 'visible') === 'true');
      await vpn.clickOnElement('learnMoreLink');

      await vpn.waitForElement('skipOnboarding');
      await vpn.waitForElementProperty('skipOnboarding', 'visible', 'true');

      // This is needed just for humans. The UI is already in the other state
      // before completing the animation.
      await vpn.wait();

      for (let i = 0; i < onboardingView; ++i) {
        assert(await vpn.hasElement('onboardingNext'));
        assert(
            await vpn.getElementProperty('onboardingNext', 'visible') ===
            'true');
        await vpn.clickOnElement('onboardingNext');

        // This is needed just for humans. The UI is already in the other state
        // before completing the animation.
        await vpn.wait();
      }

      assert(
          await vpn.getElementProperty('onboardingNext', 'visible') === 'true');
      if (await vpn.getElementProperty('onboardingNext', 'text') !== 'Next') {
        break;
      }

      await vpn.clickOnElement('skipOnboarding');

      await vpn.waitForElement('getHelpLink');
      await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');

      // This is needed just for humans. The UI is already in the other state
      // before completing the animation.
      await vpn.wait();

      ++onboardingView;
    }

    assert(onboardingView, 4);
  });
});
