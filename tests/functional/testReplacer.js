/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');
const queries = require('./queries.js');

describe('Addon content replacer', function() {
  this.timeout(0);  // DEV

  beforeEach(async () => {
    await vpn.flipFeatureOn('replacerAddon');
  });

  afterEach(async () => {
    await vpn.flipFeatureOff('replacerAddon');
  });

  describe('Addon replacer', function() {
    beforeEach(async () => {
      await vpn.resetAddons('07_replacer');
    });

    it('Replace the country/city names', async () => {
      // In this way we disable the 'home-replacement' addon.
      await vpn.setSetting('languageCode', 'it');
      await vpn.authenticateInApp(true, true);

      await vpn.waitForQueryAndClick(
          queries.screenHome.SERVER_LIST_BUTTON.visible());
      await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());
      await vpn.waitForQueryAndClick(
          queries.screenHome.serverListView.ALL_SERVERS_TAB.visible());

      const servers = await vpn.servers();

      for (let server of servers) {
        const countryId =
            queries.screenHome.serverListView.generateCountryId(server.code);
        await vpn.waitForQuery(countryId.visible());

        await vpn.scrollToQuery(
            queries.screenHome.serverListView.COUNTRY_VIEW, countryId);

        if (await vpn.getQueryProperty(countryId, 'cityListVisible') ===
            'false') {
          await vpn.clickOnQuery(countryId);
        }

        for (let city of server.cities) {
          const cityId = queries.screenHome.serverListView.generateCityId(
              countryId, city.name);
          await vpn.waitForQuery(cityId.visible());
          const cityName =
              await vpn.getQueryProperty(cityId, 'radioButtonLabelText');
          assert(
              cityName.length > 0 && !cityName.includes('a') &&
              !cityName.includes('A') && !cityName.includes('e') &&
              !cityName.includes('E') && !cityName.includes('i') &&
              !cityName.includes('I') && !cityName.includes('o') &&
              !cityName.includes('O'))
        }
      }

      await vpn.setSetting('languageCode', '');
    });
  });
});
