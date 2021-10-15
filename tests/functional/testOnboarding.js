/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const util = require('util');
const vpn = require('./helper.js');

describe('Initial view and onboarding', function() {
  this.timeout(300000);

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
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'false');

    await vpn.waitForElement('getHelpBack');
    await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');
  });

  it('Open help links', async () => {
    await vpn.clickOnElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'false');

    await vpn.waitForElement('getHelpLinks');
    await vpn.waitForElementProperty('getHelpLinks', 'visible', 'true');

    await vpn.waitForElement('getHelpLinks/getHelpBackList-0');
    await vpn.waitForElementProperty(
        'getHelpLinks/getHelpBackList-0', 'visible', 'true');

    await vpn.waitForElement('getHelpLinks/getHelpBackList-1');
    await vpn.waitForElementProperty(
        'getHelpLinks/getHelpBackList-1', 'visible', 'true');

    await vpn.waitForElement('getHelpLinks/getHelpBackList-2');
    await vpn.waitForElementProperty(
        'getHelpLinks/getHelpBackList-2', 'visible', 'true');

    await vpn.clickOnElement('getHelpLinks/getHelpBackList-2');
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.startsWith('file://') && url.includes('mozillavpn') &&
          url.endsWith('.txt');
    });

    await vpn.clickOnElement('getHelpLinks/getHelpBackList-0');
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/support');
    });

    await vpn.clickOnElement('getHelpLinks/getHelpBackList-1');
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/contact');
    });
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
