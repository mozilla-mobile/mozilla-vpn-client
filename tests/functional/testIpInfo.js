/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const vpn = require('./helper.js');
const assert = require('assert');
const queries = require('./queries.js');

describe('IP info', function() {
  this.ctx.authenticationNeeded = true;

  it('Opens and closes IP info', async () => {
    await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    await vpn.activate(true);

    // Open IP info panel
    await vpn.waitForQueryAndClick(queries.screenHome.IP_INFO_TOGGLE.visible());
    await vpn.waitForQuery(queries.screenHome.IP_INFO_PANEL.visible());
    assert(
        await vpn.getQueryProperty(
            queries.screenHome.IP_INFO_PANEL, 'isOpen') === 'true');

    // Close IP info panel
    await vpn.waitForQueryAndClick(queries.screenHome.IP_INFO_TOGGLE.visible());
    assert(
        await vpn.getQueryProperty(
            queries.screenHome.IP_INFO_PANEL, 'isOpen') === 'false');
  });

  it('Closes when VPN is deactivated', async () => {
    await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    await vpn.activate(true);

    // Open IP info panel
    await vpn.waitForQueryAndClick(queries.screenHome.IP_INFO_TOGGLE.visible());
    await vpn.waitForQuery(queries.screenHome.IP_INFO_PANEL.visible());
    assert(
        await vpn.getQueryProperty(
            queries.screenHome.IP_INFO_PANEL, 'isOpen') === 'true');

    // Deactivate VPN
    await vpn.deactivate();

    assert(
        await vpn.getQueryProperty(
            queries.screenHome.IP_INFO_PANEL, 'isOpen') === 'false');
  });
});
