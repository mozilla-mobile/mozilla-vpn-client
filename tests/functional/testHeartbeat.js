/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const { initialScreen, telemetryScreen, generalElements } = require('./elements.js');
const vpn = require('./helper.js');

describe('Backend failure', function() {
  this.timeout(300000);

  async function backendFailureAndRestore() {
    await vpn.forceHeartbeatFailure();
    await vpn.waitForElementAndClick('heartbeatTryButton');
  }

  it('Backend failure during the main view', async () => {
    await vpn.waitForMainView();
    await backendFailureAndRestore();

    await vpn.waitForElement(initialScreen.GET_HELP_LINK);
    await vpn.waitForElementProperty(initialScreen.GET_HELP_LINK, 'visible', 'true');
  });

  /* TODO:
    it('Backend failure in the help menu', async () => {
      await vpn.waitForElementProperty(initialScreen.GET_HELP_LINK, 'visible', 'true');
      await vpn.clickOnElement(initialScreen.GET_HELP_LINK);
      await vpn.waitForElementProperty(initialScreen.GET_HELP_LINK, 'visible', 'false');

      await vpn.waitForElement('getHelpBack');
      await vpn.waitForElementProperty('getHelpBack', 'visible', 'true');

      await backendFailureAndRestore();
    });
  */

    // TODO: check fix
  it('Backend failure in the onboarding (aborting in each phase)', async () => {
    let onboardingView = 0;
    let onboarding = true;
    while (onboarding) {
      await vpn.waitForElement(initialScreen.LEARN_MORE_LINK)
      assert(await vpn.getElementProperty(initialScreen.LEARN_MORE_LINK, 'visible') === 'true');
      await vpn.waitForElementAndClick(initialScreen.LEARN_MORE_LINK);

      await vpn.waitForElement(initialScreen.SKIP_ONBOARDING);
      await vpn.waitForElementProperty(initialScreen.SKIP_ONBOARDING, 'visible', 'true');

      await vpn.wait();

      for (let i = 0; i < onboardingView; ++i) {
        assert(await vpn.hasElement(initialScreen.ONBOARDING_NEXT));
        assert(await vpn.getElementProperty(initialScreen.ONBOARDING_NEXT, 'visible') === 'true');
        await vpn.clickOnElement(initialScreen.ONBOARDING_NEXT);

        await vpn.wait();
      }

      assert(await vpn.getElementProperty(initialScreen.ONBOARDING_NEXT, 'visible') === 'true');
      onboarding = await vpn.getElementProperty(initialScreen.ONBOARDING_NEXT, 'text') === 'Next';

      console.log('right before failure');
      await backendFailureAndRestore();
      console.log('line 63');
      await vpn.waitForElement(initialScreen.GET_HELP_LINK);
      await vpn.waitForElementProperty(initialScreen.GET_HELP_LINK, 'visible', 'true');
      await vpn.wait()

      ++onboardingView;
    }
  });

  it('BackendFailure during the authentication', async () => {
    if (this.ctx.wasm) {
      // Ignore this test in wasm
      return;
    }

    await vpn.waitForMainView();
    await vpn.waitForElementAndClick(initialScreen.GET_STARTED);

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });

    await vpn.wait();
    await vpn.waitForElement(initialScreen.AUTHENTICATE_VIEW);
    await vpn.waitForElementProperty(initialScreen.AUTHENTICATE_VIEW, 'visible', 'true');

    await backendFailureAndRestore();
    await vpn.waitForElement(initialScreen.GET_HELP_LINK);
    await vpn.waitForElementProperty(initialScreen.GET_HELP_LINK, 'visible', 'true');
  });

  it('BackendFailure in the Post authentication view', async () => {
    await vpn.authenticateInApp();
    await vpn.waitForElement(telemetryScreen.POST_AUTHENTICATION_BUTTON);
    await backendFailureAndRestore();
    await vpn.waitForElement(telemetryScreen.POST_AUTHENTICATION_BUTTON);
  });

  it('BackendFailure in the Telemetry policy view', async () => {
    await vpn.authenticateInApp(true, false);
    await vpn.waitForElement(telemetryScreen.TELEMETRY_POLICY_BUTTON);
    await backendFailureAndRestore();
    await vpn.waitForElement(telemetryScreen.TELEMETRY_POLICY_BUTTON);
  });

  describe('Post-auth tests', function() {
    this.ctx.authenticationNeeded = true;

    it('BackendFailure in the Controller view', async () => {
      await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
      await vpn.waitForElementProperty(generalElements.CONTROLLER_TITLE, 'visible', 'true');
      assert(
          await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ===
          'VPN is off');

      await backendFailureAndRestore();

      await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
      await vpn.waitForElementProperty(generalElements.CONTROLLER_TITLE, 'visible', 'true');
      assert(
          await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ===
          'VPN is off');
    });

    it('BackendFailure when connecting', async () => {
      await vpn.activate();

      await vpn.waitForCondition(async () => {
        let connectingMsg =
            await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text');
        return connectingMsg === 'Connecting…';
      });

      assert(
          await vpn.getElementProperty('controllerSubTitle', 'text') ===
          'Masking connection and location');

      await backendFailureAndRestore();

      await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
      await vpn.waitForElementProperty(generalElements.CONTROLLER_TITLE, 'visible', 'true');
      assert(await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') === 'VPN is off');
    });

    it('BackendFailure when connected', async () => {
      await vpn.activate();
      await vpn.waitForCondition(async () => {
        return await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ===
            'VPN is on';
      });

      await backendFailureAndRestore();

      await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
      await vpn.waitForElementProperty(generalElements.CONTROLLER_TITLE, 'visible', 'true');
      assert(
          await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ===
          'VPN is off');
    });


    it('disconnecting', async () => {
      await vpn.activate();
      await vpn.waitForCondition(() => {
        return vpn.lastNotification().title === 'VPN Connected';
      });
      await vpn.deactivate();

      await vpn.waitForCondition(async () => {
        const msg = await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text');
        return msg === 'Disconnecting…' || msg === 'VPN is off';
      });

      await backendFailureAndRestore();

      await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
      await vpn.waitForElementProperty(generalElements.CONTROLLER_TITLE, 'visible', 'true');
      assert(
          await vpn.getElementProperty(generalElements.CONTROLLER_TITLE, 'text') ===
          'VPN is off');
    });
  });
});
