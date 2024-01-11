/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import assert from 'assert';
import { screenInitialize, screenAuthenticating, navBar, screenSettings } from './queries.js';
import { flipFeatureOff, waitForInitialView, clickOnQuery, waitForCondition, getLastUrl, waitForQueryAndClick, waitForQuery, authenticateInBrowser, scrollToQuery } from './helper.js';

describe('User authentication in browser', function() {
  this.timeout(60000);

  beforeEach(async () => {
    await flipFeatureOff('inAppAuthentication');
  })

  it('returns to main view on canceling authentication', async () => {
    await waitForInitialView();

    await clickOnQuery(screenInitialize.SIGN_UP_BUTTON.visible());

    if (!this.ctx.wasm) {
      await waitForCondition(async () => {
        const url = await getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });
    }

    await clickOnQuery(
        screenInitialize.AUTHENTICATE_VIEW.visible());
    await waitForQueryAndClick(
        screenAuthenticating.CANCEL_FOOTER_LINK.visible());

    await waitForQuery(screenInitialize.SIGN_UP_BUTTON.visible());
  });

  it('Completes authentication', async () => {
    await waitForInitialView();
    await authenticateInBrowser(true, true, this.ctx.wasm);
  });

  it('Completes authentication after logout', async () => {
    await authenticateInBrowser(true, true, this.ctx.wasm);
    await waitForQueryAndClick(navBar.SETTINGS);

    await waitForQuery(screenSettings.SIGN_OUT);
    await scrollToQuery(
        screenSettings.SCREEN, screenSettings.SIGN_OUT);
    await waitForQueryAndClick(screenSettings.SIGN_OUT);
    await waitForInitialView();

    await authenticateInBrowser(false, false, this.ctx.wasm);
  });
});
