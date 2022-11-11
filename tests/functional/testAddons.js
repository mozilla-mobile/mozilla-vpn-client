/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 const assert = require('assert');
 const vpn = require('./helper.js');
 
 describe('Addons', function() {
   this.ctx.authenticationNeeded = true;
 
   beforeEach(async () => {    
    await vpn.setSetting(
      'addon/customServerAddress',
      'https://mozilla-mobile.github.io/mozilla-vpn-client/addons/');
    await vpn.wait(5000)

    if (!(await vpn.isFeatureFlippedOff('addonSignature'))) {
      await vpn.flipFeatureOff('addonSignature');      
      assert(await vpn.isFeatureFlippedOff('addonSignature'));
    }
   });

   it('Addons load completed', async () => {
    await vpn.setSetting('addon/customServer', 'false');

    await vpn.waitForElementProperty(
        'VPNAddonManager', 'loadCompleted', 'true');

    await vpn.waitForCondition(async () => {
      count = parseInt(
        await vpn.getElementProperty('VPNAddonManager', 'count'), 10);
      return count > 0;
    });

    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getElementProperty('VPNAddonManager', 'count'),
                 10) === count;
    });
  });

   it('Empty manifest addon is loaded', async () => {
    await vpn.setSetting('addon/customServer', 'true');
    await vpn.setSetting(
        'addon/customServerAddress',
        'https://bakulf.github.io/vpn-addons-test/empty_manifest/');
    
    await vpn.waitForCondition(async () => {
      return parseInt(
                 await vpn.getElementProperty('VPNAddonManager', 'count'),
                 10) === 0;
    });
  });

   it('Addons are loaded from archieve', async () => {
    await vpn.setSetting('addon/customServer', 'true');
    await vpn.setSetting(
        'addon/customServerAddress',
        'https://archive.mozilla.org/pub/vpn/addons/releases/latest/');
    
    await vpn.waitForCondition(async () => {
      return parseInt(
                  await vpn.getElementProperty('VPNAddonManager', 'count'),
                  10) > 0;
    });
  });  
 });
 