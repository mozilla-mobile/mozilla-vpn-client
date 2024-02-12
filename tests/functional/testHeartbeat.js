/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');

describe('Backend failure', function() {
  this.timeout(300000);

  it('Backend failure during the main view', async () => {
    await vpn.waitForInitialView();
    await vpn.forceHeartbeatFailure();

    await vpn.waitForQueryAndClick(queries.screenInitialize.GET_HELP_LINK);
    await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());
  });

  it('Backend failure in the help menu', async () => {
      await vpn.waitForQueryAndClick(
        queries.screenInitialize.GET_HELP_LINK.visible());

      await
    vpn.waitForQuery(queries.screenGetHelp.BACK_BUTTON.visible());

    await vpn.forceHeartbeatFailure();

    await
    vpn.waitForQuery(queries.screenGetHelp.BACK_BUTTON.visible());
  });

  it('BackendFailure during browser authentication', async () => {
    if (this.ctx.wasm) {
      // Ignore this test in wasm
      return;
    }

    await vpn.flipFeatureOff('inAppAuthentication');

    await vpn.waitForInitialView();
    await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());

    await vpn.waitForCondition(async () => {
      const url = await vpn.getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });

    await vpn.waitForQuery(
        queries.screenInitialize.AUTHENTICATE_VIEW.visible());

    // Ensure that in the event of a Guardian error during authentication,
    // the "something went wrong" screen is displayed to the user.
    await vpn.forceHeartbeatFailure();
    await vpn.waitForQueryAndClick(
      queries.screenBackendFailure.HEARTBEAT_TRY_BUTTON.visible());

    await vpn.waitForQuery(queries.screenInitialize.GET_HELP_LINK.visible());
  });

  it('BackendFailure in the Post authentication view', async function () {
    //Post auth view does not exist in new onboarding
    if (await vpn.isFeatureEnabled("newOnboarding")) {
      await vpn.flipFeatureOff("newOnboarding");
    }

    await vpn.authenticateInApp(false);
    await vpn.waitForQuery(queries.screenPostAuthentication.BUTTON.visible());
    await vpn.forceHeartbeatFailure();
    await vpn.waitForQuery(queries.screenPostAuthentication.BUTTON.visible());
  });

  it('BackendFailure in the Telemetry policy view', async function () {
    //Telemetry policy view does not exist in new onboarding
    if (await vpn.isFeatureEnabled('newOnboarding')) {
      await vpn.flipFeatureOff("newOnboarding");
    }

    await vpn.authenticateInApp(false);
    await vpn.completePostAuthentication();
    await vpn.waitForQuery(queries.screenTelemetry.BUTTON.visible());
    await vpn.forceHeartbeatFailure();
    await vpn.waitForQuery(queries.screenTelemetry.BUTTON.visible());
  });

  describe('Post-auth tests', function() {
    this.ctx.authenticationNeeded = true;

    it('BackendFailure in the Controller view', async () => {
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenHome.CONTROLLER_TITLE.visible(), 'text'),
          'VPN is off');

      await vpn.forceHeartbeatFailure();

      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenHome.CONTROLLER_TITLE.visible(), 'text'),
          'VPN is off');
    });

    it('BackendFailure when connecting', async () => {
      await vpn.activate();

      await vpn.waitForCondition(async () => {
        let connectingMsg = await vpn.getQueryProperty(
            queries.screenHome.CONTROLLER_TITLE.visible(), 'text');
        return connectingMsg === 'Connecting…';
      });

      assert.equal(
          await vpn.getQueryProperty(
              queries.screenHome.CONTROLLER_SUBTITLE, 'text'),
          'Masking connection and location');

      await vpn.forceHeartbeatFailure();

      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());

      // VPN should not be disconnected after a heartbeat failure. 
      // This ensures that the VPN remains on to avoid potentially leaking traffic
      // and leaving the user unprotected.
      await vpn.waitForCondition(async () => {
        return await vpn.getQueryProperty(
                   queries.screenHome.CONTROLLER_TITLE.visible(), 'text') ===
            'VPN is on';
      });
    });

    it('BackendFailure when connected', async () => {
      await vpn.activate();
      await vpn.waitForCondition(async () => {
        return await vpn.getQueryProperty(
                   queries.screenHome.CONTROLLER_TITLE.visible(), 'text') ===
            'VPN is on';
      });

      await vpn.forceHeartbeatFailure();

      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      
      // VPN should not be disconnected after a heartbeat failure. 
      // This ensures that the VPN remains on to avoid potentially leaking traffic
      // and leaving the user unprotected.
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenHome.CONTROLLER_TITLE.visible(), 'text'),
          'VPN is on');
    });

    it('disconnecting', async () => {
      await vpn.activate();
      await vpn.waitForCondition(() => {
        return vpn.lastNotification().title === 'VPN Connected';
      });
      await vpn.deactivate();

      await vpn.waitForCondition(async () => {
        const msg = await vpn.getQueryProperty(
            queries.screenHome.CONTROLLER_TITLE.visible(), 'text');
        return msg === 'Disconnecting…' || msg === 'VPN is off';
      });

      await vpn.forceHeartbeatFailure();

      // Because the user has already initiated the deactivation of the VPN
      // even if we encounter a heartbeat failure during the process, 
      // we proceed with deactivation as usual.
      await vpn.waitForCondition(async () => {
        const msg = await vpn.getQueryProperty(
            queries.screenHome.CONTROLLER_TITLE.visible(), 'text');
        return msg === 'VPN is off';
      });
    });
  });
});
