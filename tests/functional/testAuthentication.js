/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const util = require('util');
const vpn = require('./helper.js');

const exec = util.promisify(require('child_process').exec);

describe('User authentication', function() {
  this.timeout(60000);

  before(async () => {
    await vpn.connect();
  });

  beforeEach(() => {});

  afterEach(vpn.dumpFailure);

  after(async () => {
    vpn.disconnect();
  });


  it('reset the app', async () => await vpn.reset());

  it('wait for the main view', async () => {
    assert.strictEqual(await vpn.getLastUrl(), '');

    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
    assert(await vpn.getElementProperty('getStarted', 'visible') === 'true');
    assert(await vpn.getElementProperty('learnMoreLink', 'visible') === 'true');
  });

  it('Start and abort the authentication (initial view)', async () => {
    await vpn.clickOnElement('getStarted');

    await vpn.waitForElement('telemetryPolicyButton');
    await vpn.waitForElementProperty(
        'telemetryPolicyButton', 'visible', 'true');
    await vpn.clickOnElement('telemetryPolicyButton');

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });

    // This is to make humans happy.
    await vpn.wait();

    await vpn.waitForElement('authenticatingView');
    await vpn.waitForElementProperty('authenticatingView', 'visible', 'true');

    await vpn.waitForElement('cancelFooterLink');
    await vpn.waitForElementProperty('cancelFooterLink', 'visible', 'true');

    await vpn.clickOnElement('cancelFooterLink');

    // This is to make humans happy.
    await vpn.wait();

    await vpn.waitForElement('getStarted');
    await vpn.waitForElementProperty('getStarted', 'visible', 'true');
  });

  it('Start and abort the authentication (onboarding view)', async () => {
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

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });

    // This is to make humans happy.
    await vpn.wait();

    await vpn.waitForElement('authenticatingView');
    await vpn.waitForElementProperty('authenticatingView', 'visible', 'true');

    await vpn.waitForElement('cancelFooterLink');
    await vpn.waitForElementProperty('cancelFooterLink', 'visible', 'true');

    await vpn.clickOnElement('cancelFooterLink');

    // This is to make humans happy.
    await vpn.wait();

    await vpn.waitForElement('getStarted');
    await vpn.waitForElementProperty('getStarted', 'visible', 'true');
  });

  it('Start and complete the authentication', async () => {
    await vpn.authenticate(false, false);
  });

  it('Post authentication view', async () => {
    await vpn.waitForElement('postAuthenticationButton');
    await vpn.clickOnElement('postAuthenticationButton');

    // This is to make humans happy.
    await vpn.wait();
  });

  it('Logout', async () => {
    await vpn.logout();
    await vpn.wait();
  });

  it('quit the app', async () => await vpn.quit());
});
