/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 const assert = require('assert');
 const util = require('util');
 const vpn = require('./helper.js');
 
 const exec = util.promisify(require('child_process').exec);
 
 describe('Devices', function() {
   this.timeout(300000);
 
   before(async () => {
     await vpn.connect();
   });
 
   beforeEach(() => {});
 
   afterEach(vpn.dumpFailure);
 
   after(async () => {
     vpn.disconnect();
   });
 
   it('authenticate', async () => await vpn.authenticate());
 
   it('Post authentication view', async () => {
     await vpn.waitForElement('postAuthenticationButton');
     await vpn.clickOnElement('postAuthenticationButton');
     await vpn.wait();
   });
 
   it('Telemetry policy view', async () => {
     await vpn.waitForElement('telemetryPolicyButton');
     await vpn.waitForElementProperty(
         'telemetryPolicyButton', 'visible', 'true');
     await vpn.clickOnElement('telemetryPolicyButton');
     await vpn.wait();
   });
 
   it('opening the device list', async () => {
     await vpn.waitForElement('deviceListButton');
     await vpn.waitForElementProperty('deviceListButton', 'visible', 'true');
     await vpn.wait();
 
     await vpn._writeCommand("dismiss_surveys");
     await vpn.wait();
     await vpn.clickOnElement('deviceListButton');
     await vpn.wait();
 
     await vpn.waitForElement('deviceListBackButton');
     await vpn.waitForElementProperty('deviceListBackButton', 'visible', 'true');
     await vpn.clickOnElement('deviceListBackButton');
     await vpn.wait();
 
     await vpn.waitForElement('deviceListButton');
     await vpn.waitForElementProperty('deviceListButton', 'visible', 'true');
     await vpn.wait();
   });
 
   // TODO: test the device title X of Y
   // TODO: check the device entries in the list
   // TODO: check the 'remove icon' visibility
   // TODO: remove a device
   // TODO: max number of devices
 
   it('Logout', async () => {
     await vpn.logout();
     await vpn.wait();
   });
 
   it('quit the app', async () => await vpn.quit());
 });
 