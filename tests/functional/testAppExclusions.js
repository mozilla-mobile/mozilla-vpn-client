/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');
const {appendFile} = require('fs');

describe('Settings', function() {
  this.timeout(60000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
    await vpn.waitForQueryAndClick(queries.screenSettings.APP_EXCLUSIONS.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
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
           queries.screenSettings.appExclusionsView.CLEAR_ALL.visible().prop('enabled', 'false'));
       await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
       await vpn.waitForQueryAndClick(queries.screenSettings.appExclusionsView.CHECKBOX1.visible());
       await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

       // Clear all button is enabled when disabledAppsList.length > 0
       await vpn.waitForQuery(
           queries.screenSettings.appExclusionsView.CLEAR_ALL.visible().prop('enabled', 'true'));
       await vpn.waitForQueryAndClick(queries.screenSettings.appExclusionsView.CHECKBOX1);
       assert(await getNumDisabledApps() == '0');
     });

  it('Clear all button removes all apps from exclusion list and records telemetry', async () => {
    if (this.ctx.wasm) {
      // This test cannot run in wasm
      return;
    }
    
    await vpn.waitForQueryAndClick(queries.screenSettings.appExclusionsView.CHECKBOX1.visible());
    await vpn.waitForQueryAndClick(queries.screenSettings.appExclusionsView.CHECKBOX2.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    assert(await getNumDisabledApps() == '2');

    await vpn.waitForQueryAndClick(queries.screenSettings.appExclusionsView.CLEAR_ALL.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
    assert(await getNumDisabledApps() == '0');

    // Check that we collect telemetry
    const events = await vpn.gleanTestGetValue("interaction", "clearAppExclusionsSelected", "main");
    assert.equal(events.length, 1);
    var element = events[0];
    assert.equal(element.extra.screen, "app_exclusions");
  });

  it('Excluded apps are at the top of the list on initial load', async () => {
    await vpn.waitForQueryAndClick(queries.screenSettings.appExclusionsView.CHECKBOX2);
    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
    
    await vpn.waitForQueryAndClick(queries.screenSettings.APP_EXCLUSIONS.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
    
    await vpn.waitForQuery(queries.screenSettings.appExclusionsView.APP_ROW1.visible().prop(
        'appIdForFunctionalTests', 'com.example.two'));
  });

  it('Back button works', async () => {
    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
    await vpn.waitForQueryAndClick(queries.screenSettings.APP_EXCLUSIONS.visible());
  });

  it('Checking the excluded apps help sheet', async () => {
    if (!(await vpn.isFeatureFlippedOn('helpSheets'))) {
      await vpn.flipFeatureOn('helpSheets');
    }

    //Test the "Open privacy features" button
    await vpn.waitForQueryAndClick(queries.screenSettings.appExclusionsView.HELP_BUTTON.visible());
    await vpn.waitForQuery(queries.screenSettings.appExclusionsView.HELP_SHEET.opened());
    await vpn.waitForQueryAndClick(queries.screenSettings.appExclusionsView.HELP_SHEET_OPEN_PRIVACY_FEATURES_BUTTON.visible());
    await vpn.waitForQuery(queries.screenSettings.appExclusionsView.HELP_SHEET.closed());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
    await vpn.waitForQuery(queries.screenSettings.privacyView.VIEW.visible());

    //Go back to the app exclusions view
    await vpn.waitForQueryAndClick(queries.screenSettings.BACK.visible());
    await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

    //Test the "Learn more" link
    await vpn.waitForQueryAndClick(queries.screenSettings.appExclusionsView.HELP_BUTTON.visible());
    await vpn.waitForQuery(queries.screenSettings.appExclusionsView.HELP_SHEET.opened());
    await vpn.waitForQueryAndClick(queries.screenSettings.appExclusionsView.HELP_SHEET_LEARN_MORE_BUTTON.visible());
    await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url === 'https://support.mozilla.org/kb/split-tunneling-app-permissions';
    });
    await vpn.waitForQueryAndClick(queries.screenSettings.appExclusionsView.HELP_SHEET_CLOSE_BUTTON.visible());
  });

  describe('Checking app exclusions screen telemetry', function () {
    // No Glean on WASM.
    if(vpn.runningOnWasm()) {
      return;
    }

    const appExclusionsTelemetryScreenId = "app_exclusions"

    it('Record telemetry when user clicks on App Exclusions', async () => {
      await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
      await vpn.waitForQueryAndClick(queries.screenSettings.APP_EXCLUSIONS.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());

      // Check that we collect telemetry
      const events = await vpn.gleanTestGetValue("interaction", "appExclusionsSelected", "main");
      var element = events[0];
      assert.equal(element.extra.screen, "settings");
    });

    it('Checking app exclusions screen impression telemetry', async () => {
      const appExclusionsSreenEvents = await vpn.gleanTestGetValue("impression", "appExclusionsScreen", "main");
      assert.equal(appExclusionsSreenEvents.length, 1);
      const appExclusionsSreenEventsExtras = appExclusionsSreenEvents[0].extra;
      assert.equal(appExclusionsTelemetryScreenId, appExclusionsSreenEventsExtras.screen);
    });

    it('Checking privacy help sheet telemetry', async () => {
      if (!(await vpn.isFeatureFlippedOn('helpSheets'))) {
        await vpn.flipFeatureOn('helpSheets');
      }

      const appExclusionsHelpSheetTelemetryScreenId = "app_exclusions_info"

      await vpn.waitForQueryAndClick(queries.screenSettings.appExclusionsView.HELP_BUTTON.visible());

      const helpTooltipSelectedEvents = await vpn.gleanTestGetValue("interaction", "helpTooltipSelected", "main");
      assert.equal(helpTooltipSelectedEvents.length, 1);
      const helpTooltipSelectedEventsExtras = helpTooltipSelectedEvents[0].extra;
      assert.equal(appExclusionsTelemetryScreenId, helpTooltipSelectedEventsExtras.screen);

      await vpn.waitForQuery(queries.screenSettings.appExclusionsView.HELP_SHEET.opened());

      const appExclusionsInfoScreenEvents = await vpn.gleanTestGetValue("impression", "appExclusionsInfoScreen", "main");
      assert.equal(appExclusionsInfoScreenEvents.length, 1);
      const appExclusionsInfoScreenEventsExtras = appExclusionsInfoScreenEvents[0].extra;
      assert.equal(appExclusionsHelpSheetTelemetryScreenId, appExclusionsInfoScreenEventsExtras.screen);

      await vpn.waitForQueryAndClick(queries.screenSettings.appExclusionsView.HELP_SHEET_LEARN_MORE_BUTTON.visible());

      const learnMoreSelectedEvents = await vpn.gleanTestGetValue("interaction", "learnMoreSelected", "main");
      assert.equal(learnMoreSelectedEvents.length, 1);
      const learnMoreSelectedEventsExtras = learnMoreSelectedEvents[0].extra;
      assert.equal(appExclusionsHelpSheetTelemetryScreenId, learnMoreSelectedEventsExtras.screen);
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
  });
});