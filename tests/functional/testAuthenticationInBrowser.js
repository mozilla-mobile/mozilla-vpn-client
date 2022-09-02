/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');

describe('User authentication in browser', function() {
  this.timeout(60000);

  beforeEach(async () => {
    await vpn.flipFeatureOff('inAppAuthentication');
  })

  it('returns to main view on canceling authentication', async () => {
    await vpn.waitForMainView();

    await vpn.clickOnElement('getStarted');

    if (!this.ctx.wasm) {
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });
    }

    await vpn.waitForElement('authenticatingView');
    await vpn.waitForElementProperty('authenticatingView', 'visible', 'true');

    await vpn.waitForElement('cancelFooterLink');
    await vpn.waitForElementProperty('cancelFooterLink', 'visible', 'true');

    await vpn.clickOnElement('cancelFooterLink');

    await vpn.waitForElement('getStarted');
    await vpn.waitForElementProperty('getStarted', 'visible', 'true');
  });

  it('Starts authentication at end of onboarding view', async () => {
    await vpn.waitForMainView();

    assert(await vpn.getElementProperty('learnMoreLink', 'visible') === 'true');
    await vpn.clickOnElement('learnMoreLink');

    await vpn.waitForElement('skipOnboarding');
    await vpn.waitForElementProperty('skipOnboarding', 'visible', 'true');

    // This is needed just for humans. The UI is already in the other state
    // before completing the animation.
    await vpn.wait();

    while (true) {
      assert(await vpn.hasElement('onboardingNext'));
      assert(
          await vpn.getElementProperty('onboardingNext', 'visible') === 'true');

      assert(
          await vpn.getElementProperty('onboardingNext', 'visible') === 'true');
      if (await vpn.getElementProperty('onboardingNext', 'text') === 'Next') {
        await vpn.clickOnElement('onboardingNext');
        // This is needed just for humans. The UI is already in the other state
        // before completing the animation.
        await vpn.wait();
        continue;
      }

      break;
    }
    await vpn.clickOnElement('onboardingNext');

    if (!this.ctx.wasm) {
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });
    }

    await vpn.waitForElement('authenticatingView');
    await vpn.waitForElementProperty('authenticatingView', 'visible', 'true');
  });

  it('Completes authentication', async () => {
    await vpn.authenticateInBrowser(true, true, this.ctx.wasm);
  });

  it('Completes authentication after logout', async () => {
    await vpn.authenticateInBrowser(true, true, this.ctx.wasm);

    await vpn.waitForElement('navigationLayout/navButton-settings');
    await vpn.clickOnElement('navigationLayout/navButton-settings');

    await vpn.waitForElement('settingsLogout');
    await vpn.scrollToElement('settingsView-flickable', 'settingsLogout');
    await vpn.clickOnElement('settingsLogout');
    await vpn.waitForMainView();

    await vpn.authenticateInBrowser(false, false, this.ctx.wasm);
  });
});
