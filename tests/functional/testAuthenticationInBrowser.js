/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const { initialView, homeView, navBar, settingsView } = require('./elements.js');
const vpn = require('./helper.js');

describe('User authentication in browser', function() {
  this.timeout(60000);

  beforeEach(async () => {
    await vpn.flipFeatureOff('inAppAuthentication');
  })

  it('returns to main view on canceling authentication', async () => {
    await vpn.waitForMainView();

    await vpn.waitForElementAndClick(initialView.GET_STARTED);

    if (!this.ctx.wasm) {
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });
    }

    await vpn.waitForElement(initialView.AUTHENTICATE_VIEW);
    await vpn.waitForElementProperty(initialView.AUTHENTICATE_VIEW, 'visible', 'true');
    await vpn.waitForElementAndClick(homeView.CANCEL_FOOTER_LINK);

    await vpn.waitForElement(initialView.GET_STARTED);
    await vpn.waitForElementProperty(initialView.GET_STARTED, 'visible', 'true');
  });

  it('Starts authentication at end of onboarding view', async () => {
    await vpn.waitForMainView();

    assert(await vpn.getElementProperty(initialView.LEARN_MORE_LINK, 'visible') === 'true');
    await vpn.waitForElementAndClick(initialView.LEARN_MORE_LINK);

    await vpn.waitForElement(initialView.SKIP_ONBOARDING);
    await vpn.waitForElementProperty(initialView.SKIP_ONBOARDING, 'visible', 'true');

    // This is needed just for humans. The UI is already in the other state
    // before completing the animation.
    await vpn.wait();

    while (true) {
      assert(await vpn.hasElement(initialView.ONBOARDING_NEXT));
      assert(await vpn.getElementProperty(initialView.ONBOARDING_NEXT, 'visible') === 'true');

      assert(await vpn.getElementProperty(initialView.ONBOARDING_NEXT, 'visible') === 'true');
      if (await vpn.getElementProperty(initialView.ONBOARDING_NEXT, 'text') === 'Next') {
        await vpn.clickOnElement(initialView.ONBOARDING_NEXT);
        // This is needed just for humans. The UI is already in the other state
        // before completing the animation.
        await vpn.wait();
        continue;
      }

      break;
    }
    await vpn.clickOnElement(initialView.ONBOARDING_NEXT);

    if (!this.ctx.wasm) {
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });
    }

    await vpn.waitForElement(initialView.AUTHENTICATE_VIEW);
    await vpn.waitForElementProperty(initialView.AUTHENTICATE_VIEW, 'visible', 'true');
  });

  it('Completes authentication', async () => {
    await vpn.authenticateInBrowser(true, true, this.ctx.wasm);
  });

  it('Completes authentication after logout', async () => {
    await vpn.authenticateInBrowser(true, true, this.ctx.wasm);    
    await vpn.waitForElementAndClick(navBar.SETTINGS);

    await vpn.waitForElement(settingsView.SIGN_OUT);
    await vpn.scrollToElement(settingsView.SCREEN, settingsView.SIGN_OUT);
    await vpn.waitForElementAndClick(settingsView.SIGN_OUT);
    await vpn.waitForMainView();

    await vpn.authenticateInBrowser(false, false, this.ctx.wasm);
  });
});
