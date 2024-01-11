/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import assert from 'assert';
import { flipFeatureOn, flipFeatureOff, resetAddons, setSetting, authenticateInApp, waitForQueryAndClick, waitForQuery, servers as _servers, scrollToQuery, getQueryProperty, clickOnQuery } from './helper.js';
import { screenHome } from './queries.js';

describe('Addon content replacer', function() {
  this.timeout(60000);

  beforeEach(async () => {
    await flipFeatureOn('replacerAddon');
  });

  afterEach(async () => {
    await flipFeatureOff('replacerAddon');
  });

  describe('Addon replacer', function() {
    beforeEach(async () => {
      await resetAddons('07_replacer');
    });

    it('Replace the country/city names', async () => {
      // In this way we disable the 'home-replacement' addon.
      await setSetting('languageCode', 'it');
      await authenticateInApp(true, true);

      await waitForQueryAndClick(
          screenHome.SERVER_LIST_BUTTON.visible());
      await waitForQuery(screenHome.STACKVIEW.ready());
      await waitForQueryAndClick(
          screenHome.serverListView.ALL_SERVERS_TAB.visible());

      const servers = await _servers();

      for (let server of servers) {
        const countryId =
            screenHome.serverListView.generateCountryId(server.code);
        await waitForQuery(countryId.visible());

        await scrollToQuery(
            screenHome.serverListView.COUNTRY_VIEW, countryId);

        if (await getQueryProperty(countryId, 'cityListVisible') ===
            'false') {
          await clickOnQuery(countryId);
        }

        for (let city of server.cities) {
          const cityId = screenHome.serverListView.generateCityId(
              countryId, city.name);
          await waitForQuery(cityId.visible());
          const cityName =
              await getQueryProperty(cityId, 'radioButtonLabelText');
          assert(
              cityName.length > 0 && !cityName.includes('a') &&
              !cityName.includes('A') && !cityName.includes('e') &&
              !cityName.includes('E') && !cityName.includes('i') &&
              !cityName.includes('I') && !cityName.includes('o') &&
              !cityName.includes('O'))
        }
      }

      await setSetting('languageCode', '');
    });
  });
});
