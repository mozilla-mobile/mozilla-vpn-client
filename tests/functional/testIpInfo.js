/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const vpn = require('./helper.js');
const assert = require('assert');
const { homeScreen, generalElements } = require('./elements.js');

describe('IP info', function() {
  this.ctx.authenticationNeeded = true;

  it('Opens and closes IP info', async () => {
    await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
    await vpn.activate(true);

    // Open IP info panel
    await vpn.waitForElementAndClick(homeScreen.IP_INFO_TOGGLE);
    await vpn.waitForElement(homeScreen.IP_INFO_PANEL);
    assert(await vpn.getElementProperty(homeScreen.IP_INFO_PANEL, 'isOpen') === 'true');

    // Close IP info panel
    await vpn.waitForElementAndClick(homeScreen.IP_INFO_TOGGLE);
    assert(await vpn.getElementProperty(homeScreen.IP_INFO_PANEL, 'isOpen') === 'false');
  });
});
