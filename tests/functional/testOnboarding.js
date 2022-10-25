/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const { initialView, settingsView } = require('./elements.js');
const vpn = require('./helper.js');

describe('Initial view and onboarding', function() {
  this.timeout(240000);

  beforeEach(async () => {
    assert(await vpn.getLastUrl() === '');
  })

  it('Check for links on mainView', async () => {
    await vpn.waitForElement(initialView.GET_HELP_LINK);
    await vpn.waitForElementProperty(initialView.GET_HELP_LINK, 'visible', 'true');
    assert(await vpn.getElementProperty(initialView.GET_STARTED, 'visible') === 'true');
    assert(await vpn.getElementProperty(initialView.LEARN_MORE_LINK, 'visible') === 'true');
  });

  it('Open the help menu', async () => {
    await vpn.wait()
    await vpn.clickOnElement(initialView.GET_HELP_LINK);
    await vpn.waitForElement(settingsView.getHelpView.BACK);
    await vpn.waitForElementProperty(settingsView.getHelpView.BACK, 'visible', 'true');
  });

  it('Open help links', async () => {
    await vpn.wait()
    await vpn.clickOnElement(initialView.GET_HELP_LINK);
    await vpn.waitForElement(settingsView.getHelpView.LINKS);
    await vpn.waitForElementProperty(settingsView.getHelpView.LINKS, 'visible', 'true');

    await vpn.waitForElement(settingsView.getHelpView.HELP_CENTER);
    await vpn.waitForElementProperty(settingsView.getHelpView.HELP_CENTER, 'visible', 'true');

    await vpn.waitForElement(settingsView.getHelpView.SUPPORT);
    await vpn.waitForElementProperty(settingsView.getHelpView.SUPPORT, 'visible', 'true');

    await vpn.waitForElement(settingsView.getHelpView.LOGS);
    await vpn.waitForElementProperty(settingsView.getHelpView.LOGS, 'visible', 'true');

    await vpn.clickOnElement(settingsView.getHelpView.LOGS);
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

    await vpn.clickOnElement(settingsView.getHelpView.SUPPORT);
    await vpn.waitForElement(settingsView.getHelpView.contactSupportView.UNAUTH_USER_INPUTS);
    await vpn.waitForElementProperty(settingsView.getHelpView.contactSupportView.UNAUTH_USER_INPUTS, 'visible', 'true');
  });

  it('Complete the onboarding (aborting in each phase)', async () => {
    let onboardingView = 0;

    while (true) {
      await vpn.wait()
      assert(await vpn.getElementProperty(initialView.LEARN_MORE_LINK, 'visible') === 'true');
      await vpn.clickOnElement(initialView.LEARN_MORE_LINK);

      await vpn.waitForElement(initialView.SKIP_ONBOARDING);
      await vpn.waitForElementProperty(initialView.SKIP_ONBOARDING, 'visible', 'true');

      // This is needed just for humans. The UI is already in the other state
      // before completing the animation.
      await vpn.wait();

      for (let i = 0; i < onboardingView; ++i) {
        assert(await vpn.hasElement(initialView.ONBOARDING_NEXT));
        assert(await vpn.getElementProperty(initialView.ONBOARDING_NEXT, 'visible') === 'true');
        await vpn.clickOnElement(initialView.ONBOARDING_NEXT);

        // This is needed just for humans. The UI is already in the other state
        // before completing the animation.
        await vpn.wait();
      }

      assert(await vpn.getElementProperty(initialView.ONBOARDING_NEXT, 'visible') === 'true');
      if (await vpn.getElementProperty(initialView.ONBOARDING_NEXT, 'text') !== 'Next') {
        break;
      }

      await vpn.clickOnElement(initialView.SKIP_ONBOARDING);

      await vpn.waitForElement(initialView.GET_HELP_LINK);
      await vpn.waitForElementProperty(initialView.GET_HELP_LINK, 'visible', 'true');

      // This is needed just for humans. The UI is already in the other state
      // before completing the animation.
      await vpn.wait();

      ++onboardingView;
    }

    assert(onboardingView, 4);
  });
});
