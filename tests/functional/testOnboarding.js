/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const util = require('util');
const vpn = require('./helper.js');

describe('Initial view and onboarding', function() {
  this.timeout(100000);

  before(async () => {
    await vpn.connect();
  });

  beforeEach(() => {});

  afterEach(() => {});

  after(async () => {
    vpn.disconnect();
  });

  it('reset the app', async () => await vpn.reset());

  it('wait for the main view', async () => {
    assert(await vpn.getLastUrl() == '');

    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
    assert(await vpn.getElementProperty('getStarted', 'visible') == 'true');
    assert(await vpn.getElementProperty('learnMoreLink', 'visible') == 'true');
  });

  it('Open the help menu', async () => {
    await vpn.clickOnElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'false');

    await vpn.waitForElement('getHelpBack');
    await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');
  });

  it('Open some links', async () => {
    await vpn.waitForElement('getHelpBackList');
    await vpn.waitForElement('getHelpBackList/getHelpBackList-0');
    await vpn.waitForElement('getHelpBackList/getHelpBackList-1');
    await vpn.waitForElement('getHelpBackList/getHelpBackList-2');

    // We don't test the view log.

    await vpn.clickOnElement('getHelpBackList/getHelpBackList-1');
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/support');
    });

    await vpn.clickOnElement('getHelpBackList/getHelpBackList-2');
    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.endsWith('/r/vpn/contact');
    });
  });

  it('Go back to the main view', async() => {
    await vpn.clickOnElement('getHelpBack');

    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');

    // This is needed just for humans. The UI is already in the other state
    // before completing the animation.
    await vpn.wait();
  });

  it('Complete the onboarding (aborting in each phase)', async () => {
    let onboardingView = 0;

    while (true) {
      assert(
          await vpn.getElementProperty('learnMoreLink', 'visible') == 'true');
      await vpn.clickOnElement('learnMoreLink');

      await vpn.waitForElement('skipOnboarding');
      await vpn.waitForElementProperty('skipOnboarding', 'visible', 'true');

      // This is needed just for humans. The UI is already in the other state
      // before completing the animation.
      await vpn.wait();

      for (let i = 0; i < onboardingView; ++i) {
        assert(await vpn.hasElement('onboardingNext'));
        assert(
            await vpn.getElementProperty('onboardingNext', 'visible') ==
            'true');
        await vpn.clickOnElement('onboardingNext');

        // This is needed just for humans. The UI is already in the other state
        // before completing the animation.
        await vpn.wait();
      }

      assert(
          await vpn.getElementProperty('onboardingNext', 'visible') == 'true');
      if (await vpn.getElementProperty('onboardingNext', 'text') != 'Next') {
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

  it('quit the app', async () => await vpn.quit());
});
