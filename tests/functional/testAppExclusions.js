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
    await vpn.waitForQueryAndClick(screenSettings.APP_EXCLUSIONS.visible());
  });

  const getClearAllCount = async () => {
    return await vpn.getQueryProperty(
        appExclusionsView.CLEAR_ALL.visible(), 'numDisabledApps');
  };

  const getNumDisabledApps =
      async () => {
    const disabledApps = await vpn.getSetting('vpnDisabledApps');
    return disabledApps.length;
  }

  it('Clear all button shows accurate number of disabled apps', async () => {
    assert(await getNumDisabledApps() == await getClearAllCount());
  });

  it('Clear all counter is correctly updated when changing app exclusion status',
     async () => {
       await vpn.waitForQuery(screenSettings.STACKVIEW.ready());
       // Clear all button is correctly disabled when there are no disabled apps
       await vpn.waitForQuery(
           appExclusionsView.CLEAR_ALL.visible().prop('enabled', 'false'));

       // Clear all button Counter is correctly updated as
       // disabledAppsList.length changes
       await vpn.waitForQueryAndClick(appExclusionsView.CHECKBOX1.visible());
       assert(await getNumDisabledApps() == await getClearAllCount());

       // Clear button is enabled when disabledAppsList.length > 0
       await vpn.waitForQuery(
           appExclusionsView.CLEAR_ALL.visible().prop('enabled', 'true'));
       await vpn.waitForQueryAndClick(appExclusionsView.CHECKBOX1);
       assert(await getNumDisabledApps() == await getClearAllCount());
     });

  it('Clear all button enables all apps', async () => {
    await vpn.waitForQuery(screenSettings.STACKVIEW.ready());
    await vpn.waitForQueryAndClick(appExclusionsView.CHECKBOX1.visible());
    await vpn.waitForQueryAndClick(appExclusionsView.CHECKBOX2.visible());
    await vpn.waitForQuery(screenSettings.STACKVIEW.ready());

    assert(await getNumDisabledApps() == await getClearAllCount());
    assert(await getClearAllCount() == 2);

    await vpn.waitForQueryAndClick(appExclusionsView.CLEAR_ALL.visible());
    await vpn.waitForQuery(screenSettings.STACKVIEW.ready());
    assert(await getNumDisabledApps() == await getClearAllCount());
    assert(await getClearAllCount() == '0');
  });

  it('Disabled apps are at the top of the list on initial load', async () => {
    await vpn.waitForQuery(screenSettings.STACKVIEW.ready());
    await vpn.waitForQueryAndClick(appExclusionsView.CHECKBOX2);
    await vpn.waitForQueryAndClick(screenSettings.BACK.visible());
    await vpn.waitForQuery(screenSettings.STACKVIEW.ready());
    await vpn.waitForQueryAndClick(screenSettings.APP_EXCLUSIONS.visible());
    await vpn.waitForQuery(screenSettings.STACKVIEW.ready());
    await vpn.waitForQuery(appExclusionsView.APP_ROW2.visible().prop(
        'idxForFunctionalTests', '0'));
  });

  it('Back button works', async () => {
    await vpn.waitForQueryAndClick(screenSettings.BACK.visible());
    await vpn.waitForQuery(screenSettings.STACKVIEW.ready());
    await vpn.waitForQueryAndClick(screenSettings.APP_EXCLUSIONS.visible());

    await vpn.waitForQueryAndClick(screenSettings.BACK.visible());
    assert(await getNumDisabledApps() == await getClearAllCount());
  });
});