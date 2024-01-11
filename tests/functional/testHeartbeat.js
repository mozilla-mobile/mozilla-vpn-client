/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { equal } from 'assert';
import { screenInitialize, screenGetHelp, screenBackendFailure, screenPostAuthentication, screenTelemetry, screenHome } from './queries.js';
import { waitForInitialView, forceHeartbeatFailure, waitForQueryAndClick, waitForQuery, flipFeatureOff, clickOnQuery, waitForCondition, getLastUrl, authenticateInApp, getQueryProperty, activate, lastNotification, deactivate } from './helper.js';

describe('Backend failure', function() {
  this.timeout(300000);

  it('Backend failure during the main view', async () => {
    await waitForInitialView();
    await forceHeartbeatFailure();

    await waitForQueryAndClick(screenInitialize.GET_HELP_LINK);
    await waitForQuery(screenGetHelp.LINKS.visible());
  });

  it('Backend failure in the help menu', async () => {
      await waitForQueryAndClick(
        screenInitialize.GET_HELP_LINK.visible());

      await
    waitForQuery(screenGetHelp.BACK_BUTTON.visible());

    await forceHeartbeatFailure();

    await
    waitForQuery(screenGetHelp.BACK_BUTTON.visible());
  });

  it('BackendFailure during browser authentication', async () => {
    if (this.ctx.wasm) {
      // Ignore this test in wasm
      return;
    }

    await flipFeatureOff('inAppAuthentication');

    await waitForInitialView();
    await clickOnQuery(screenInitialize.SIGN_UP_BUTTON.visible());

    await waitForCondition(async () => {
      const url = await getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });

    await waitForQuery(
        screenInitialize.AUTHENTICATE_VIEW.visible());

    // Ensure that in the event of a Guardian error during authentication,
    // the "something went wrong" screen is displayed to the user.
    await forceHeartbeatFailure();
    await waitForQueryAndClick(
      screenBackendFailure.HEARTBEAT_TRY_BUTTON.visible());

    await waitForQuery(screenInitialize.GET_HELP_LINK.visible());
  });

  it('BackendFailure in the Post authentication view', async () => {
    await authenticateInApp();
    await waitForQuery(screenPostAuthentication.BUTTON.visible());
    await forceHeartbeatFailure();
    await waitForQuery(screenPostAuthentication.BUTTON.visible());
  });

  it('BackendFailure in the Telemetry policy view', async () => {
    await authenticateInApp(true, false);
    await waitForQuery(screenTelemetry.BUTTON.visible());
    await forceHeartbeatFailure();
    await waitForQuery(screenTelemetry.BUTTON.visible());
  });

  describe('Post-auth tests', function() {
    this.ctx.authenticationNeeded = true;

    it('BackendFailure in the Controller view', async () => {
      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
      equal(
          await getQueryProperty(
              screenHome.CONTROLLER_TITLE.visible(), 'text'),
          'VPN is off');

      await forceHeartbeatFailure();

      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
      equal(
          await getQueryProperty(
              screenHome.CONTROLLER_TITLE.visible(), 'text'),
          'VPN is off');
    });

    it('BackendFailure when connecting', async () => {
      await activate();

      await waitForCondition(async () => {
        let connectingMsg = await getQueryProperty(
            screenHome.CONTROLLER_TITLE.visible(), 'text');
        return connectingMsg === 'Connecting…';
      });

      equal(
          await getQueryProperty(
              screenHome.CONTROLLER_SUBTITLE, 'text'),
          'Masking connection and location');

      await forceHeartbeatFailure();

      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());

      // VPN should not be disconnected after a heartbeat failure. 
      // This ensures that the VPN remains on to avoid potentially leaking traffic
      // and leaving the user unprotected.
      await waitForCondition(async () => {
        return await getQueryProperty(
                   screenHome.CONTROLLER_TITLE.visible(), 'text') ===
            'VPN is on';
      });
    });

    it('BackendFailure when connected', async () => {
      await activate();
      await waitForCondition(async () => {
        return await getQueryProperty(
                   screenHome.CONTROLLER_TITLE.visible(), 'text') ===
            'VPN is on';
      });

      await forceHeartbeatFailure();

      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
      
      // VPN should not be disconnected after a heartbeat failure. 
      // This ensures that the VPN remains on to avoid potentially leaking traffic
      // and leaving the user unprotected.
      equal(
          await getQueryProperty(
              screenHome.CONTROLLER_TITLE.visible(), 'text'),
          'VPN is on');
    });

    it('disconnecting', async () => {
      await activate();
      await waitForCondition(() => {
        return lastNotification().title === 'VPN Connected';
      });
      await deactivate();

      await waitForCondition(async () => {
        const msg = await getQueryProperty(
            screenHome.CONTROLLER_TITLE.visible(), 'text');
        return msg === 'Disconnecting…' || msg === 'VPN is off';
      });

      await forceHeartbeatFailure();

      // Because the user has already initiated the deactivation of the VPN
      // even if we encounter a heartbeat failure during the process, 
      // we proceed with deactivation as usual.
      await waitForCondition(async () => {
        const msg = await getQueryProperty(
            screenHome.CONTROLLER_TITLE.visible(), 'text');
        return msg === 'VPN is off';
      });
    });
  });
});
