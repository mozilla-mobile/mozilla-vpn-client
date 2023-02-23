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

    await vpn.waitForQueryAndClick(queries.screenInitialize.SIGN_UP_BUTTON.visible());

    if (!this.ctx.wasm) {
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });
    }

    await vpn.waitForQueryAndClick(
        queries.screenInitialize.AUTHENTICATE_VIEW.visible());
    await vpn.waitForQueryAndClick(
        queries.screenAuthenticating.CANCEL_FOOTER_LINK.visible());

    await vpn.waitForQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
  });

  it('Completes authentication', async () => {
    await vpn.waitForInitialView();
    console.log('line 38: ===== ', this.ctx.wasm);
    await vpn.authenticateInBrowser(true, true, this.ctx.wasm);
  });

  it.skip('Completes authentication after logout', async () => {
    await vpn.authenticateInBrowser(true, true, this.ctx.wasm);
    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());

    await vpn.waitForQuery(queries.screenSettings.SIGN_OUT.visible());
    await vpn.scrollToQuery(
        queries.screenSettings.SCREEN, queries.screenSettings.SIGN_OUT.visible());
    await vpn.waitForQueryAndClick(queries.screenSettings.SIGN_OUT.visible());
    await vpn.waitForInitialView();

    await vpn.authenticateInBrowser(false, false, this.ctx.wasm);
  });
});
