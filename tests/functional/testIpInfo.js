/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { waitForQuery, activate, waitForQueryAndClick, getQueryProperty, deactivate, runningOnWasm, gleanTestGetValue } from './helper.js';
import { equal, strictEqual } from 'assert';
import { screenHome } from './queries.js';

describe('IP info', function() {
  this.ctx.authenticationNeeded = true;

  it('Opens and closes IP info', async () => {
    await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
    await activate(true);

    // Open IP info panel
    await waitForQueryAndClick(screenHome.IP_INFO_TOGGLE.visible());
    await waitForQuery(screenHome.IP_INFO_PANEL.visible());
    equal(
        await getQueryProperty(
            screenHome.IP_INFO_PANEL, 'isOpen'), 'true');

    // Close IP info panel
    await waitForQueryAndClick(screenHome.IP_INFO_TOGGLE.visible());
    equal(
        await getQueryProperty(
            screenHome.IP_INFO_PANEL, 'isOpen'), 'false');
  });

  it('Closes when VPN is deactivated', async () => {
    await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
    await activate(true);

    // Open IP info panel
    await waitForQueryAndClick(screenHome.IP_INFO_TOGGLE.visible());
    await waitForQuery(screenHome.IP_INFO_PANEL.visible());
    equal(
        await getQueryProperty(
            screenHome.IP_INFO_PANEL, 'isOpen'), 'true');

    // Deactivate VPN
    await deactivate();

    equal(
        await getQueryProperty(
            screenHome.IP_INFO_PANEL, 'isOpen'), 'false');
  });

  describe('ip info related telemetry tests', () => {
    if(runningOnWasm()) {
      // No Glean on WASM.
      return;
    }

    // Telemetry design is detailed at:
    // https://miro.com/app/board/uXjVM_QZzjA=/?share_link_id=616399368132

    it("records event on connection info modal interaction", async () => {
      await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
      await activate(true);

      const [ { extra: appStepExtras } ] = await gleanTestGetValue("impression", "mainScreen", "main")
      strictEqual("main", appStepExtras.screen);

      // Open IP info panel
      await waitForQueryAndClick(screenHome.IP_INFO_TOGGLE.visible());
      await waitForQuery(screenHome.IP_INFO_PANEL.visible());
      equal(
          await getQueryProperty(
              screenHome.IP_INFO_PANEL, 'isOpen'), 'true');

      const openedEventsList = await gleanTestGetValue("interaction", "openConnectionInfoSelected", "main")
      strictEqual(openedEventsList.length, 1);
      const openedExtras = openedEventsList[0].extra;
      strictEqual("main", openedExtras.screen);

      const [ { extra: connectionInfoOpenedExtras } ] = await gleanTestGetValue("impression", "connectionInfoScreen", "main")
      strictEqual("connection_info", connectionInfoOpenedExtras.screen);

      // Close IP info panel
      await waitForQueryAndClick(screenHome.IP_INFO_TOGGLE.visible());
      equal(
          await getQueryProperty(
              screenHome.IP_INFO_PANEL, 'isOpen'), 'false');

      const closedEventsList = await gleanTestGetValue("interaction", "closeSelected", "main")
      strictEqual(closedEventsList.length, 1);
      const closedExtras = closedEventsList[0].extra;
      strictEqual("connection_info", closedExtras.screen);
    });
  })
});
