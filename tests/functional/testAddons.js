/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');

describe('Addons', function() {
  this.ctx.authenticationNeeded = true;

  it('Empty addon index', async () => {
    await vpn.resetAddons('01_empty_manifest');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getElementProperty('VPNAddonManager', 'count'),
                 10) === 0;
    });
  });

  it('Broken addon index', async () => {
    await vpn.resetAddons('03_single_addon');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getElementProperty('VPNAddonManager', 'count'),
                 10) === 1;
    });

    await vpn.fetchAddons('02_broken_manifest');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getElementProperty('VPNAddonManager', 'count'),
                 10) === 1;
    });
  });

  it('Addons are loaded', async () => {
    await vpn.resetAddons('03_single_addon');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getElementProperty('VPNAddonManager', 'count'),
                 10) === 1;
    });

    await vpn.fetchAddons('01_empty_manifest');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getElementProperty('VPNAddonManager', 'count'),
                 10) === 0;
    });

    await vpn.fetchAddons('03_single_addon');
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getElementProperty('VPNAddonManager', 'count'),
                 10) === 1;
    });
  });
});
