/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const {appExclusionsView, navBar, screenSettings} = require('./queries.js');
const vpn = require('./helper.js');
const {appendFile} = require('fs');

describe('Settings', function() {
  this.timeout(60000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await vpn.waitForQueryAndClick(navBar.SETTINGS.visible());
    await vpn.waitForQuery(screenSettings.STACKVIEW.ready());
    await vpn.waitForQueryAndClick(screenSettings.APP_EXCLUSIONS.visible());
    await vpn.waitForQuery(screenSettings.STACKVIEW.ready());
  });

  const getNumDisabledApps =
      async () => {
    const disabledApps = await vpn.getSetting('vpnDisabledApps');
    return disabledApps.length;
  }

  it('Clear all counter is correctly enabled and disabled',
     async () => {
       // Clear all button is correctly disabled when there are no disabled apps
       await vpn.waitForQuery(
           appExclusionsView.CLEAR_ALL.visible().prop('enabled', 'false'));
       await vpn.waitForQuery(screenSettings.STACKVIEW.ready());
       await vpn.waitForQueryAndClick(appExclusionsView.CHECKBOX1.visible());
       await vpn.waitForQuery(screenSettings.STACKVIEW.ready());

       // Clear all button is enabled when disabledAppsList.length > 0
       await vpn.waitForQuery(
           appExclusionsView.CLEAR_ALL.visible().prop('enabled', 'true'));
       await vpn.waitForQueryAndClick(appExclusionsView.CHECKBOX1);
       assert(await getNumDisabledApps() == '0');
     });

  it('Clear all button removes all apps from exclusion list and records telemetry', async () => {
    await vpn.waitForQueryAndClick(appExclusionsView.CHECKBOX1.visible());
    await vpn.waitForQueryAndClick(appExclusionsView.CHECKBOX2.visible());
    await vpn.waitForQuery(screenSettings.STACKVIEW.ready());

    assert(await getNumDisabledApps() == '2');

    await vpn.waitForQueryAndClick(appExclusionsView.CLEAR_ALL.visible());
    await vpn.waitForQuery(screenSettings.STACKVIEW.ready());
    assert(await getNumDisabledApps() == '0');

    // Check that we collect telemetry
    const events = await vpn.gleanTestGetValue("interaction", "clearAppExclusionsSelected", "main");
    assert.equal(events.length, 1);
    var element = events[0];
    assert.equal(element.extra.screen, "app_exclusions");
  });

  it('Excluded apps are at the top of the list on initial load', async () => {
    await vpn.waitForQueryAndClick(appExclusionsView.CHECKBOX2);
    await vpn.waitForQueryAndClick(screenSettings.BACK.visible());
    await vpn.waitForQuery(screenSettings.STACKVIEW.ready());
    
    await vpn.waitForQueryAndClick(screenSettings.APP_EXCLUSIONS.visible());
    await vpn.waitForQuery(screenSettings.STACKVIEW.ready());
    
    await vpn.waitForQuery(appExclusionsView.APP_ROW1.visible().prop(
        'appIdForFunctionalTests', 'com.example.two'));
  });

  it('Back button works', async () => {
    await vpn.waitForQueryAndClick(screenSettings.BACK.visible());
    await vpn.waitForQuery(screenSettings.STACKVIEW.ready());
    await vpn.waitForQueryAndClick(screenSettings.APP_EXCLUSIONS.visible());
  });
});