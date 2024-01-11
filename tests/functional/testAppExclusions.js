/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import assert, { equal } from 'assert';
import { appExclusionsView, navBar, screenSettings } from './queries.js';
import { waitForQueryAndClick, waitForQuery, getSetting, gleanTestGetValue } from './helper.js';
import { appendFile } from 'fs';

describe('Settings', function() {
  this.timeout(60000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await waitForQueryAndClick(navBar.SETTINGS.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());
    await waitForQueryAndClick(screenSettings.APP_EXCLUSIONS.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());
  });

  const getNumDisabledApps =
      async () => {
    const disabledApps = await getSetting('vpnDisabledApps');
    return disabledApps.length;
  }

  it('Clear all counter is correctly enabled and disabled',
     async () => {
       // Clear all button is correctly disabled when there are no disabled apps
       await waitForQuery(
           appExclusionsView.CLEAR_ALL.visible().prop('enabled', 'false'));
       await waitForQuery(screenSettings.STACKVIEW.ready());
       await waitForQueryAndClick(appExclusionsView.CHECKBOX1.visible());
       await waitForQuery(screenSettings.STACKVIEW.ready());

       // Clear all button is enabled when disabledAppsList.length > 0
       await waitForQuery(
           appExclusionsView.CLEAR_ALL.visible().prop('enabled', 'true'));
       await waitForQueryAndClick(appExclusionsView.CHECKBOX1);
       assert(await getNumDisabledApps() == '0');
     });

  it('Clear all button removes all apps from exclusion list and records telemetry', async () => {
    if (this.ctx.wasm) {
      // This test cannot run in wasm
      return;
    }
    
    await waitForQueryAndClick(appExclusionsView.CHECKBOX1.visible());
    await waitForQueryAndClick(appExclusionsView.CHECKBOX2.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    assert(await getNumDisabledApps() == '2');

    await waitForQueryAndClick(appExclusionsView.CLEAR_ALL.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());
    assert(await getNumDisabledApps() == '0');

    // Check that we collect telemetry
    const events = await gleanTestGetValue("interaction", "clearAppExclusionsSelected", "main");
    equal(events.length, 1);
    var element = events[0];
    equal(element.extra.screen, "app_exclusions");
  });

  it('Excluded apps are at the top of the list on initial load', async () => {
    await waitForQueryAndClick(appExclusionsView.CHECKBOX2);
    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());
    
    await waitForQueryAndClick(screenSettings.APP_EXCLUSIONS.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());
    
    await waitForQuery(appExclusionsView.APP_ROW1.visible().prop(
        'appIdForFunctionalTests', 'com.example.two'));
  });

  it('Back button works', async () => {
    await waitForQueryAndClick(screenSettings.BACK.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());
    await waitForQueryAndClick(screenSettings.APP_EXCLUSIONS.visible());
  });

  // Dummy VPN does not have the Add Application button so we cannot currently test this.
  // Jira issue: https://mozilla-hub.atlassian.net/browse/VPN-5974
  it('Record telemetry when user clicks on Add application', async () => {
    // await vpn.waitForQueryAndClick(appExclusionsView.ADD_APPLICATION_BUTTON.visible());
    // await vpn.waitForQuery(appExclusionsView.STACKVIEW.ready());

    // const events = await vpn.gleanTestGetValue("interaction", "addApplicationSelected", "main")
    // assert.equal(events.length, 1);
    // var element = events[0];
    // assert.equal(element.extra.screen, "app_exclusions");
  });

  it('Record telemetry when user clicks on App Exclusions', async () => {
    if (this.ctx.wasm) {
      // This test cannot run in wasm
      return;
    }
    await waitForQueryAndClick(navBar.SETTINGS.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());
    await waitForQueryAndClick(screenSettings.APP_EXCLUSIONS.visible());
    await waitForQuery(screenSettings.STACKVIEW.ready());

    // Check that we collect telemetry
    const events = await gleanTestGetValue("interaction", "appExclusionsSelected", "main");
    var element = events[0];
    equal(element.extra.screen, "settings");
  });
});
