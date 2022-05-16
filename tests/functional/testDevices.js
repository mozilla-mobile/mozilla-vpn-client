/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
const vpn = require('./helper.js');

describe('Devices', function() {
  this.ctx.authenticationNeeded = true;

  it('Opens and closes the device list', async () => {
    await vpn.waitForElement('deviceListButton');
    await vpn.waitForElementProperty('deviceListButton', 'visible', 'true');
    await vpn.wait();

    await vpn._writeCommand('dismiss_surveys');
    await vpn.wait();
    await vpn.clickOnElement('deviceListButton');
    await vpn.wait();

    await vpn.waitForElement('deviceListBackButton');
    await vpn.waitForElementProperty('deviceListBackButton', 'visible', 'true');
    await vpn.clickOnElement('deviceListBackButton');
    await vpn.wait();

    await vpn.waitForElement('deviceListButton');
    await vpn.waitForElementProperty('deviceListButton', 'visible', 'true');
  });

  // TODO: test the device title X of Y
  // TODO: check the device entries in the list
  // TODO: check the 'remove icon' visibility
  // TODO: remove a device
  // TODO: max number of devices
});
