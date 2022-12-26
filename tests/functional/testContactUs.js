/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const vpn = require('./helper.js');
const assert = require('assert');
const queries = require('./queries.js');


describe('Contact us view', function() {
  it('VPNUserInfo is not visible to unathenticated users', async () => {
    await vpn.waitForQueryAndClick(queries.screenInitialize.GET_HELP_LINK);
    await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());

    await vpn.waitForQueryAndClick(queries.screenGetHelp.SUPPORT);

    await vpn.waitForQuery(
        queries.screenGetHelp.contactSupportView.USER_INFO.hidden());
    assert(
        await vpn.getQueryProperty(
            queries.screenGetHelp.contactSupportView.USER_INFO, 'visible') ===
        'false');
  });

  it('Email inputs are visible to unauthenticated user', async () => {
    await vpn.waitForQueryAndClick(queries.screenInitialize.GET_HELP_LINK);
    await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());

    await vpn.waitForQueryAndClick(queries.screenGetHelp.SUPPORT);

    await vpn.waitForQuery(
        queries.screenGetHelp.contactSupportView.UNAUTH_USER_INPUTS.visible());
  });

  describe('Contact us view - authenticated user', function() {
    this.ctx.authenticationNeeded = true;

    async function openContactUsInSettings() {
      await vpn.waitForQueryAndClick(queries.navBar.SETTINGS);

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

      await vpn.waitForQueryAndClick(queries.screenSettings.GET_HELP.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(queries.screenGetHelp.LINKS.visible());
      await vpn.waitForQueryAndClick(queries.screenGetHelp.SUPPORT);
    }

    it('VPNUserInfo visible to authenticated users', async () => {
      await openContactUsInSettings();

      await vpn.waitForQuery(
          queries.screenGetHelp.contactSupportView.USER_INFO.visible());
    });

    it('VPNUserInfo is disabled', async () => {
      await openContactUsInSettings();

      await vpn.waitForQuery(
          queries.screenGetHelp.contactSupportView.USER_INFO.visible()
              .disabled());
    });

    it('Email inputs are not visible to authenticated user', async () => {
      it('Email inputs are visible to unauthenticated user', async () => {
        await openContactUsInSettings();

        await vpn.waitForQuery(queries.screenGetHelp.contactSupportView
                                   .UNAUTH_USER_INPUTS.hidden());
      });
    });
  });
});
