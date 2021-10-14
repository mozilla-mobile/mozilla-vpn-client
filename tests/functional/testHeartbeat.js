/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const util = require('util');
const vpn = require('./helper.js');

describe('Backend failure', function() {
  async function backendFailureAndRestore() {
    await vpn.forceHeartbeatFailure();

    await vpn.waitForElement('heartbeatTryButton');
    await vpn.waitForElementProperty('heartbeatTryButton', 'visible', 'true');

    await vpn.wait();

    await vpn.clickOnElement('heartbeatTryButton');
    await vpn.wait();
  }

  this.timeout(300000);

  before(async () => {
    await vpn.connect();
  });

  beforeEach(() => {});

  afterEach(vpn.dumpFailure);

  after(async () => {
    vpn.disconnect();
  });

  it('reset the app', async () => await vpn.reset());

  it('Backend failure during the main view', async () => {
    assert(await vpn.getLastUrl() === '');

    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');

    await backendFailureAndRestore();

    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
    await vpn.wait();
  });

  it('Backend failure in the help menu', async () => {
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
    await vpn.clickOnElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'false');

    await vpn.waitForElement('getHelpBack');
    await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');

    await backendFailureAndRestore();

    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
  });

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
    await vpn.waitForElement('getHelpLink');
    await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');

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

    await vpn.wait();
  });

  it('authenticate', async () => await vpn.authenticate(false));

  it('BackendFailure in the Post authentication view', async () => {
    await vpn.waitForElement('postAuthenticationButton');

    await backendFailureAndRestore();

    await vpn.waitForElement('postAuthenticationButton');
    await vpn.clickOnElement('postAuthenticationButton');
  });

  it('BackendFailure in the Telemetry policy view', async () => {
    await vpn.waitForElement('telemetryPolicyButton');

    await backendFailureAndRestore();

    await vpn.waitForElement('telemetryPolicyButton');
    await vpn.clickOnElement('telemetryPolicyButton');
    await vpn.wait();
  });

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

  it('connecting', async () => {
    await vpn.activate();

    await vpn.waitForCondition(async () => {
      let connectingMsg =
          await vpn.getElementProperty('controllerTitle', 'text');
      return connectingMsg === 'Connecting…';
    });
  });

  it('BackendFailure when connected', async () => {
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

  it('connecting', async () => {
    //await vpn.activate();

    await vpn.waitForCondition(async () => {
      let connectingMsg =
          await vpn.getElementProperty('controllerTitle', 'text');
      return connectingMsg === 'Connecting…';
    });
  });

  it('connected', async () => {
    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('controllerTitle', 'text') ===
          'VPN is on';
    });

    vpn.wait();
  });

  it('disconnecting', async () => {
    await vpn.deactivate();

    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('controllerTitle', 'text') ===
          'Disconnecting…';
    });

    await backendFailureAndRestore();

    await vpn.waitForElement('controllerTitle');
    await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
    assert(
        await vpn.getElementProperty('controllerTitle', 'text') ===
        'VPN is off');
  });

  it('Logout', async () => {
    await vpn.logout();
    await vpn.wait();
  });

  it('quit the app', async () => await vpn.quit());
});
