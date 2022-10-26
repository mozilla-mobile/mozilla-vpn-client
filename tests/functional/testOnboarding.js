/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const { initialScreen, getHelpScreen } = require('./elements.js');
const vpn = require('./helper.js');

describe('Initial view and onboarding', function() {
  this.timeout(240000);

  beforeEach(async () => {
    assert(await vpn.getLastUrl() === '');
  })

  it('Check for links on mainView', async () => {
    await vpn.waitForElement(initialScreen.GET_HELP_LINK);
    await vpn.waitForElementProperty(initialScreen.GET_HELP_LINK, 'visible', 'true');
    assert(await vpn.getElementProperty(initialScreen.GET_STARTED, 'visible') === 'true');
    assert(await vpn.getElementProperty(initialScreen.LEARN_MORE_LINK, 'visible') === 'true');
  });

  it('Open the help menu', async () => {
    await vpn.wait()
    await vpn.clickOnElement(initialScreen.GET_HELP_LINK);
    await vpn.waitForElement(getHelpScreen.BACK);
    await vpn.waitForElementProperty(getHelpScreen.BACK, 'visible', 'true');
  });

  it('Open help links', async () => {
    await vpn.wait()
    await vpn.clickOnElement(initialScreen.GET_HELP_LINK);
    await vpn.waitForElement(getHelpScreen.LINKS);
    await vpn.waitForElementProperty(getHelpScreen.LINKS, 'visible', 'true');

    await vpn.waitForElement(getHelpScreen.HELP_CENTER);
    await vpn.waitForElementProperty(getHelpScreen.HELP_CENTER, 'visible', 'true');

    await vpn.waitForElement(getHelpScreen.SUPPORT);
    await vpn.waitForElementProperty(getHelpScreen.SUPPORT, 'visible', 'true');

    await vpn.waitForElement(getHelpScreen.LOGS);
    await vpn.waitForElementProperty(getHelpScreen.LOGS, 'visible', 'true');

    await vpn.clickOnElement(getHelpScreen.LOGS);
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

    await vpn.clickOnElement(getHelpScreen.SUPPORT);
    await vpn.waitForElement(getHelpScreen.contactSupportView.UNAUTH_USER_INPUTS);
    await vpn.waitForElementProperty(getHelpScreen.contactSupportView.UNAUTH_USER_INPUTS, 'visible', 'true');
  });

  it('Complete the onboarding (aborting in each phase)', async () => {
    let onboardingView = 0;

    while (true) {
      await vpn.wait()
      assert(await vpn.getElementProperty(initialScreen.LEARN_MORE_LINK, 'visible') === 'true');
      await vpn.clickOnElement(initialScreen.LEARN_MORE_LINK);

      await vpn.waitForElement(initialScreen.SKIP_ONBOARDING);
      await vpn.waitForElementProperty(initialScreen.SKIP_ONBOARDING, 'visible', 'true');

      // This is needed just for humans. The UI is already in the other state
      // before completing the animation.
      await vpn.wait();

      for (let i = 0; i < onboardingView; ++i) {
        assert(await vpn.hasElement(initialScreen.ONBOARDING_NEXT));
        assert(await vpn.getElementProperty(initialScreen.ONBOARDING_NEXT, 'visible') === 'true');
        await vpn.clickOnElement(initialScreen.ONBOARDING_NEXT);

        // This is needed just for humans. The UI is already in the other state
        // before completing the animation.
        await vpn.wait();
      }

      assert(await vpn.getElementProperty(initialScreen.ONBOARDING_NEXT, 'visible') === 'true');
      if (await vpn.getElementProperty(initialScreen.ONBOARDING_NEXT, 'text') !== 'Next') {
        break;
      }

      await vpn.clickOnElement(initialScreen.SKIP_ONBOARDING);

      await vpn.waitForElement(initialScreen.GET_HELP_LINK);
      await vpn.waitForElementProperty(initialScreen.GET_HELP_LINK, 'visible', 'true');

      // This is needed just for humans. The UI is already in the other state
      // before completing the animation.
      await vpn.wait();

      ++onboardingView;
    }

    assert(onboardingView, 4);
  });
});
