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
    assert.equal(
        await vpn.getQueryProperty(
            queries.screenHome.IP_INFO_PANEL, 'isOpen'), 'true');

    // Close IP info panel
    await vpn.waitForQueryAndClick(queries.screenHome.IP_INFO_TOGGLE.visible());
    assert.equal(
        await vpn.getQueryProperty(
            queries.screenHome.IP_INFO_PANEL, 'isOpen'), 'false');
  });

  it('Closes when VPN is deactivated', async () => {
    await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    await vpn.activate(true);

    // Open IP info panel
    await vpn.waitForQueryAndClick(queries.screenHome.IP_INFO_TOGGLE.visible());
    await vpn.waitForQuery(queries.screenHome.IP_INFO_PANEL.visible());
    assert.equal(
        await vpn.getQueryProperty(
            queries.screenHome.IP_INFO_PANEL, 'isOpen'), 'true');

    // Deactivate VPN
    await vpn.deactivate();

    assert.equal(
        await vpn.getQueryProperty(
            queries.screenHome.IP_INFO_PANEL, 'isOpen'), 'false');
  });

  describe('ip info related telemetry tests', () => {
    if(vpn.runningOnWasm()) {
      // No Glean on WASM.
      return;
    }

    // Telemetry design is detailed at:
    // https://miro.com/app/board/uXjVM_QZzjA=/?share_link_id=616399368132

    it("records event on connection info modal interaction", async () => {
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
      await vpn.activate(true);

      const [ { extra: appStepExtras } ] = await vpn.gleanTestGetValue("impression", "mainScreen", "main")
      assert.strictEqual("main", appStepExtras.screen);

      // Open IP info panel
      await vpn.waitForQueryAndClick(queries.screenHome.IP_INFO_TOGGLE.visible());
      await vpn.waitForQuery(queries.screenHome.IP_INFO_PANEL.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenHome.IP_INFO_PANEL, 'isOpen'), 'true');

      const openedEventsList = await vpn.gleanTestGetValue("interaction", "openConnectionInfoSelected", "main")
      assert.strictEqual(openedEventsList.length, 1);
      const openedExtras = openedEventsList[0].extra;
      assert.strictEqual("main", openedExtras.screen);

      const [ { extra: connectionInfoOpenedExtras } ] = await vpn.gleanTestGetValue("impression", "connectionInfoScreen", "main")
      assert.strictEqual("connection_info", connectionInfoOpenedExtras.screen);

      // Close IP info panel
      await vpn.waitForQueryAndClick(queries.screenHome.IP_INFO_TOGGLE.visible());
      assert.equal(
          await vpn.getQueryProperty(
              queries.screenHome.IP_INFO_PANEL, 'isOpen'), 'false');

      const closedEventsList = await vpn.gleanTestGetValue("interaction", "closeSelected", "main")
      assert.strictEqual(closedEventsList.length, 1);
      const closedExtras = closedEventsList[0].extra;
      assert.strictEqual("connection_info", closedExtras.screen);
    });
  })
});
