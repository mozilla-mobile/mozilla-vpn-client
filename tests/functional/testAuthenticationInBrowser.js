/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');

describe('User authentication in browser', function() {
  this.timeout(60000);

  beforeEach(async () => {
    await vpn.flipFeatureOff('inAppAuthentication');
  })

  it('returns to main view on canceling authentication', async () => {
    await vpn.waitForInitialView();

    await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());

    if (!this.ctx.wasm) {
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        console.log('BAKU', url);
        return url.includes('/api/v2/vpn/login');
      });
    }
    console.log('A');

    await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
    console.log('AA');

        await vpn.vpn.waitForQueryAndClick(
            queries.screenInitialize.AUTHENTICATE_VIEW.visible());
        console.log('B');
        await vpn.waitForQueryAndClick(
            queries.screenAuthenticating.CANCEL_FOOTER_LINK.visible());
        console.log('C');

        await vpn.vpn.waitForQueryAndClick(
            queries.screenInitialize.SIGN_UP_BUTTON.visible());
        console.log('D');
  });

  it('Completes authentication', async () => {
    console.log('HERE?');
    await vpn.waitForInitialView();
    await vpn.authenticateInBrowser(true, true, this.ctx.wasm);
  });

  it('Completes authentication after logout', async () => {
    console.log('HERE??');
    await vpn.authenticateInBrowser(true, true, this.ctx.wasm);
    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS);

    await vpn.waitForQuery(queries.screenSettings.SIGN_OUT);
    await vpn.scrollToQuery(
        queries.screenSettings.SCREEN, queries.screenSettings.SIGN_OUT);
    await vpn.waitForQueryAndClick(queries.screenSettings.SIGN_OUT);
    await vpn.waitForInitialView();

    await vpn.authenticateInBrowser(false, false, this.ctx.wasm);
  });
});
