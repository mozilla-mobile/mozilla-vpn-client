/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 const assert = require('assert');
 const vpn = require('./helper.js');
 
 describe('Verify Alert Handling', function() {
   describe('Verify athentication failed alert', function() {
     it('Enable unsecured-network-alert feature', async () => {
       vpn.resetLastNotification();
 
       await vpn.setSetting('unsecured-network-alert', 'false');
       assert(await vpn.getSetting('unsecured-network-alert') === 'false');
 
       await vpn.setSetting('unsecured-network-alert', 'true');
       assert(await vpn.getSetting('unsecured-network-alert') === 'true');
     });
 
     it('Verify connection failed alert', async () => {
       assert(await vpn.getLastUrl() === '');
 
       await vpn.waitForElement('getHelpLink');
       await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
 
       await vpn.forceUnsecuredNetworkAlert();
       await vpn.wait();
 
       // No notifications during the main view.
       assert(vpn.lastNotification().title === null);
     });
 
     it('Verify No connection alert', async () => {
       if (this.ctx.wasm) {
         // In wasm, the auth is fake and we cannot cancel the auth flow
         return;
       }
 
       await vpn.waitForElement('getHelpLink');
       await vpn.waitForElementProperty('getHelpLink', 'visible', 'true');
 
       await vpn.clickOnElement('getStarted');
 
       await vpn.waitForCondition(async () => {
         const url = await vpn.getLastUrl();
         return url.includes('/api/v2/vpn/login');
       });
 
       await vpn.wait();
 
       await vpn.waitForElement('authenticatingView');
       await vpn.waitForElementProperty('authenticatingView', 'visible', 'true');
 
       await vpn.forceUnsecuredNetworkAlert();
       await vpn.wait();
 
       // No notifications during the main view.
       assert(vpn.lastNotification().title === null);
 
       await vpn.waitForElement('cancelFooterLink');
       await vpn.waitForElementProperty('cancelFooterLink', 'visible', 'true');
 
       await vpn.clickOnElement('cancelFooterLink');
       await vpn.wait();
 
       await vpn.waitForElement('getStarted');
       await vpn.waitForElementProperty('getStarted', 'visible', 'true');
     });
 
     it('Verify auth code sent alert', async () => {
       await vpn.authenticateInApp(false, false);
       await vpn.waitForElement('postAuthenticationButton');
 
       await vpn.forceUnsecuredNetworkAlert();
       await vpn.wait();
 
       // Notifications are not OK yet.
       assert(vpn.lastNotification().title === null);
 
       await vpn.clickOnElement('postAuthenticationButton');
       await vpn.wait();
     });
 
     it('Verify geo ip restriction alert', async () => {
       await vpn.authenticateInApp(true, false);
       await vpn.waitForElement('telemetryPolicyButton');
 
       await vpn.forceUnsecuredNetworkAlert();
       await vpn.wait();
 
       // Notifications are not OK yet.
       assert(vpn.lastNotification().title === null);
 
       await vpn.clickOnElement('telemetryPolicyButton');
       await vpn.wait();
     });     
   });
 });
 