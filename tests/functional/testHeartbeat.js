/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');

describe('Backend failure', function() {
  this.timeout(300000);

  async function backendFailureAndRestore() {
    await vpn.forceHeartbeatFailure();
    await vpn.waitForQueryAndClick(
        queries.screenBackendFailure.HEARTBEAT_TRY_BUTTON.visible());
  }

  it('Backend failure during the main view', async () => {
    await vpn.waitForInitialView();
    await backendFailureAndRestore();

    await vpn.waitForQueryAndClick(queries.screenInitialize.GET_HELP_LINK);
    await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());
  });

  /* TODO:
    it('Backend failure in the help menu', async () => {
      await vpn.waitForQueryAndClick(
        queries.screenInitialize.GET_HELP_LINK.visible());

      await
    vpn.waitForQuery(queries.screenGetHelp.BACK_BUTTON.visible());

      await backendFailureAndRestore();
    });
  */

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

    await backendFailureAndRestore();
    await vpn.waitForQuery(queries.screenInitialize.GET_HELP_LINK.visible());
  });

  it('BackendFailure in the Post authentication view', async () => {
    await vpn.authenticateInApp();
    await vpn.waitForQuery(queries.screenPostAuthentication.BUTTON.visible());
    await backendFailureAndRestore();
    await vpn.waitForQuery(queries.screenPostAuthentication.BUTTON.visible());
  });

  it('BackendFailure in the Telemetry policy view', async () => {
    await vpn.authenticateInApp(true, false);
    await vpn.waitForQuery(queries.screenTelemetry.BUTTON.visible());
    await backendFailureAndRestore();
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

      await backendFailureAndRestore();

      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenHome.CONTROLLER_TITLE.visible(), 'text'),
          'VPN is off');
    });

    it('BackendFailure when connecting', async () => {
      await vpn.activate();

      // This bit is commented out because the UI has slightly changed 
      // with the work in VPN-5312 and as a result the Connecting screen doesn't show.
      // Once the UI is tidied up, we should re-enable this.
      // await vpn.waitForCondition(async () => {
      //   let connectingMsg = await vpn.getQueryProperty(
      //       queries.screenHome.CONTROLLER_TITLE.visible(), 'text');
      //   return connectingMsg === 'Connecting…';
      // });

      // assert.equal(
      //     await vpn.getQueryProperty(
      //         queries.screenHome.CONTROLLER_SUBTITLE, 'text'),
      //     'Masking connection and location');

      await backendFailureAndRestore();

      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenHome.CONTROLLER_TITLE.visible(), 'text'),
          'VPN is off');
    });

    it.skip('BackendFailure when connected', async () => {
      await vpn.activate();
      await vpn.waitForCondition(async () => {
        return await vpn.getQueryProperty(
                   queries.screenHome.CONTROLLER_TITLE.visible(), 'text') ===
            'VPN is on';
      });

      await backendFailureAndRestore();

      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenHome.CONTROLLER_TITLE.visible(), 'text'),
          'VPN is off');
    });


    it('disconnecting', async () => {
      await vpn.activate();
      await vpn.waitForCondition(() => {
        return vpn.lastNotification().title === 'VPN Connected';
      });
      await vpn.deactivate();

      // await vpn.waitForCondition(async () => {
      //   const msg = await vpn.getQueryProperty(
      //       queries.screenHome.CONTROLLER_TITLE.visible(), 'text');
      //   return msg === 'Disconnecting…' || msg === 'VPN is off';
      // });

      await backendFailureAndRestore();

      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenHome.CONTROLLER_TITLE.visible(), 'text'),
          'VPN is off');
    });
  });
});
