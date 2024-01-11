/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { waitForQueryAndClick, waitForQuery, getQueryProperty } from './helper.js';
import { equal } from 'assert';
import { screenInitialize, screenGetHelp, navBar, global, screenSettings } from './queries.js';


describe('Contact us view', function() {
  it('VPNUserInfo is not visible to unathenticated users', async () => {
    await waitForQueryAndClick(screenInitialize.GET_HELP_LINK);
    await waitForQuery(screenGetHelp.LINKS.visible());

    await waitForQueryAndClick(screenGetHelp.SUPPORT);

    await waitForQuery(
        screenGetHelp.contactSupportView.USER_INFO.hidden());
    equal(
        await getQueryProperty(
            screenGetHelp.contactSupportView.USER_INFO, 'visible'),
        'false');
  });

  it('Email inputs are visible to unauthenticated user', async () => {
    await waitForQueryAndClick(screenInitialize.GET_HELP_LINK);
    await waitForQuery(screenGetHelp.LINKS.visible());

    await waitForQueryAndClick(screenGetHelp.SUPPORT);

    await waitForQuery(
        screenGetHelp.contactSupportView.UNAUTH_USER_INPUTS.visible());
  });

  describe('Contact us view - authenticated user', function() {
    this.ctx.authenticationNeeded = true;

    async function openContactUsInSettings() {
      await waitForQueryAndClick(navBar.SETTINGS);

      await waitForQuery(global.SCREEN_LOADER.ready());

      await waitForQueryAndClick(screenSettings.GET_HELP.visible());

      await waitForQuery(global.SCREEN_LOADER.ready());
      await waitForQuery(screenGetHelp.LINKS.visible());
      await waitForQueryAndClick(screenGetHelp.SUPPORT);
    }

    it('VPNUserInfo visible to authenticated users', async () => {
      await openContactUsInSettings();

      await waitForQuery(
          screenGetHelp.contactSupportView.USER_INFO.visible());
    });

    it('VPNUserInfo is disabled', async () => {
      await openContactUsInSettings();

      await waitForQuery(
          screenGetHelp.contactSupportView.USER_INFO.visible()
              .disabled());
    });

    it('Email inputs are not visible to authenticated user', async () => {
      it('Email inputs are visible to unauthenticated user', async () => {
        await openContactUsInSettings();

        await waitForQuery(screenGetHelp.contactSupportView
                                   .UNAUTH_USER_INPUTS.hidden());
      });
    });
  });
});
