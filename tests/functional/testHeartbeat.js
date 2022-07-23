/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');

describe('Backend failure', function() {
  this.timeout(300000);

  async function backendFailureAndRestore() {
    await vpn.forceHeartbeatFailure();

    await vpn.waitForElement('heartbeatTryButton');
    await vpn.waitForElementProperty('heartbeatTryButton', 'visible', 'true');

    await vpn.wait();

    await vpn.clickOnElement('heartbeatTryButton');
    await vpn.wait();
  }

  it('Backend failure during the main view', async () => {
    await vpn.waitForMainView();
    await backendFailureAndRestore();

    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
    await vpn.wait();
  });

  /* TODO
    it('Backend failure in the help menu', async () => {
      await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
      await vpn.clickOnElement('getHelpLink');
      await vpn.waitForElementProperty('getHelpLink', 'visible', 'false');

      await vpn.waitForElement('getHelpBack');
      await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');

      await backendFailureAndRestore();
    });
  */

  it('Backend failure in the onboarding (aborting in each phase)', async () => {
    let onboardingView = 0;
    let onboarding = true;
    while (onboarding) {
      assert(
          await vpn.getElementProperty('learnMoreLink', 'visible') === 'true');
      await vpn.clickOnElement('learnMoreLink');

      await vpn.waitForElement('skipOnboarding');
      await vpn.waitForElementProperty('skipOnboarding', 'visible', 'true');

      await vpn.wait();

      for (let i = 0; i < onboardingView; ++i) {
        assert(await vpn.hasElement('onboardingNext'));
        assert(
            await vpn.getElementProperty('onboardingNext', 'visible') ===
            'true');
        await vpn.clickOnElement('onboardingNext');

        await vpn.wait();
      }

      assert(
          await vpn.getElementProperty('onboardingNext', 'visible') === 'true');

      onboarding =
          await vpn.getElementProperty('onboardingNext', 'text') === 'Next';

      await backendFailureAndRestore();
      await vpn.waitForElement('getHelpLink');
      await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');

      await vpn.wait();

      ++onboardingView;
    }
  });

  it('BackendFailure during the authentication', async () => {
    if (this.ctx.wasm) {
      // Ignore this test in wasm
      return;
    }

    await vpn.waitForMainView();
    await vpn.clickOnElement('getStarted');

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });

    await vpn.wait();

    await vpn.waitForElement('authenticatingView');
    await vpn.waitForElementProperty('authenticatingView', 'visible', 'true');

    await backendFailureAndRestore();
    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
  });

  it('BackendFailure in the Post authentication view', async () => {
    await vpn.authenticateInApp();
    await vpn.waitForElement('postAuthenticationButton');
    await backendFailureAndRestore();
    await vpn.waitForElement('postAuthenticationButton');
  });

  it('BackendFailure in the Telemetry policy view', async () => {
    await vpn.authenticateInApp(true, false);
    await vpn.waitForElement('telemetryPolicyButton');
    await backendFailureAndRestore();
    await vpn.waitForElement('telemetryPolicyButton');
  });

  describe('Post-auth tests', function() {
    this.ctx.authenticationNeeded = true;

    it('BackendFailure in the Controller view', async () => {
      await vpn.waitForElement('controllerTitle');
      await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
      assert(
          await vpn.getElementProperty('controllerTitle', 'text') ===
          'VPN is off');

      await backendFailureAndRestore();

      await vpn.waitForElement('controllerTitle');
      await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
      assert(
          await vpn.getElementProperty('controllerTitle', 'text') ===
          'VPN is off');
    });

    it('BackendFailure when connecting', async () => {
      await vpn.activate();

      await vpn.waitForCondition(async () => {
        let connectingMsg =
            await vpn.getElementProperty('controllerTitle', 'text');
        return connectingMsg === 'Connecting…';
      });

      assert(
          await vpn.getElementProperty('controllerSubTitle', 'text') ===
          'Masking connection and location');

      await backendFailureAndRestore();

      await vpn.waitForElement('controllerTitle');
      await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
      assert(
          await vpn.getElementProperty('controllerTitle', 'text') ===
          'VPN is off');
    });

    it('BackendFailure when connected', async () => {
      await vpn.activate();
      await vpn.waitForCondition(async () => {
        return await vpn.getElementProperty('controllerTitle', 'text') ===
            'VPN is on';
      });

      await backendFailureAndRestore();

      await vpn.waitForElement('controllerTitle');
      await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
      assert(
          await vpn.getElementProperty('controllerTitle', 'text') ===
          'VPN is off');
    });


    it('disconnecting', async () => {
      await vpn.activate();
      await vpn.waitForCondition(() => {
        return vpn.lastNotification().title === 'VPN Connected';
      });
      await vpn.deactivate();

      await vpn.waitForCondition(async () => {
        const msg = await vpn.getElementProperty('controllerTitle', 'text');
        return msg === 'Disconnecting…' || msg === 'VPN is off';
      });

      await backendFailureAndRestore();

      await vpn.waitForElement('controllerTitle');
      await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
      assert(
          await vpn.getElementProperty('controllerTitle', 'text') ===
          'VPN is off');
    });
  });
});
