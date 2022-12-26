/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');

describe('Tips and tricks intro modal', function () {
  this.timeout(60000);

  beforeEach(async () => {
    await vpn.resetAddons('04_tutorials_basic');
    await vpn.setSetting('tipsAndTricksIntroShown', 'false');
    await vpn.authenticateInApp(true, true);
  });

  it('Access and navigate tips and tricks via intro modal', async () => {
    await vpn.waitForQuery(
        queries.screenHome.TIPS_AND_TRICKS_POPUP_LOADER.prop('active', true));
    await vpn.waitForQueryAndClick(
        queries.screenHome.TIPS_AND_TRICKS_POPUP_DISCOVER.visible());

    await vpn.waitForQuery(queries.screenTipsAndTricks.VIEW);
    await vpn.waitForQueryAndClick(queries.screenHome.BACK);

    await vpn.waitForQuery(queries.screenHome.SCREEN);
  });

  it('Closing modal with close button', async () => {
    await vpn.waitForQuery(
        queries.screenHome.TIPS_AND_TRICKS_POPUP_LOADER.prop('active', true));
    await vpn.waitForQueryAndClick(
        queries.screenHome.TIPS_AND_TRICKS_POPUP_CLOSE);

    await vpn.waitForQuery(
        queries.screenHome.TIPS_AND_TRICKS_POPUP_LOADER.prop('active', false));
  });

  it('Closing modal with go back button', async () => {
    await vpn.waitForQuery(
        queries.screenHome.TIPS_AND_TRICKS_POPUP_LOADER.prop('active', true));
    await vpn.waitForQueryAndClick(
        queries.screenHome.TIPS_AND_TRICKS_POPUP_BACK);

    await vpn.waitForQuery(
        queries.screenHome.TIPS_AND_TRICKS_POPUP_LOADER.prop('active', false));
  });
})
