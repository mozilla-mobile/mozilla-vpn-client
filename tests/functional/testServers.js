/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 
import assert, { equal, strictEqual } from 'assert';
import { screenHome } from './queries.js';
import { waitForQueryAndClick, waitForQuery, servers as _servers, getMozillaProperty, scrollToQuery, getQueryProperty, clickOnQuery, setQueryProperty, setSetting, activate, waitForCondition, lastNotification, wait } from './helper.js';

describe('Server list', function() {
  let servers;
  let currentCountryCode;
  let currentCity;

  this.timeout(240000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await waitForQueryAndClick(
        screenHome.SERVER_LIST_BUTTON.visible());
    await waitForQuery(screenHome.STACKVIEW.ready());
    await waitForQueryAndClick(screenHome.serverListView.ALL_SERVERS_TAB.visible());

    servers = await _servers();
    currentCountryCode = await getMozillaProperty(
        'Mozilla.VPN', 'VPNCurrentServer', 'exitCountryCode');
    currentCity = await getMozillaProperty(
        'Mozilla.VPN', 'VPNCurrentServer', 'exitCityName');

    for (let server of servers) {
      if (currentCountryCode === server.code) {
        for (let city of server.cities) {
          if (city.name == currentCity) {
            currentCity = city.localizedName;
            break;
          }
        }
      }
    }
    console.log(
        'Current city (localized):', currentCity,
        '| Current country code:', currentCountryCode);
  });

  it('opening the server list', async () => {
    await waitForQueryAndClick(
        screenHome.serverListView.BACK_BUTTON.visible());
    await waitForQuery(screenHome.SERVER_LIST_BUTTON.visible());
  });

  it('check the countries and cities', async () => {
    for (let server of servers) {
      const countryId =
          screenHome.serverListView.generateCountryId(server.code);
      await waitForQuery(countryId.visible());

      await scrollToQuery(
          screenHome.serverListView.COUNTRY_VIEW, countryId);

      if (currentCountryCode === server.code) {
        equal(
            await getQueryProperty(countryId, 'cityListVisible'),
            'true');
      }

      if (await getQueryProperty(countryId, 'cityListVisible') ===
          'false') {
        await clickOnQuery(countryId);
      }

      for (let city of server.cities) {
        const cityId = screenHome.serverListView.generateCityId(
            countryId, city.name);
        await waitForQuery(cityId.visible().prop(
            'checked',
            currentCountryCode === server.code &&
                    currentCity === city.localizedName ?
                'true' :
                'false'));
      }
    }
  });

  it('Pick cities', async () => {
    for (let server of servers) {
      const countryId =
          screenHome.serverListView.generateCountryId(server.code);
      await waitForQuery(countryId.visible());

      await scrollToQuery(
          screenHome.serverListView.COUNTRY_VIEW, countryId);

      if (await getQueryProperty(countryId, 'cityListVisible') ===
          'false') {
        await clickOnQuery(countryId);
        await waitForQuery(countryId.ready());
      }

      await waitForQuery(countryId.prop('cityListVisible', true));

      for (let city of server.cities) {
        console.log('  Start test for city:', city);
        const cityId = screenHome.serverListView.generateCityId(
            countryId, city.name);
        await waitForQuery(cityId);

        await setQueryProperty(
            screenHome.serverListView.COUNTRY_VIEW, 'contentY',
            parseInt(await getQueryProperty(cityId, 'y')) +
                parseInt(await getQueryProperty(countryId, 'y')));
        await waitForQuery(cityId.visible());

        const cityName =
            await getQueryProperty(cityId, 'radioButtonLabelText');

        await clickOnQuery(cityId);
        await waitForQuery(screenHome.STACKVIEW.ready());

        currentCountryCode = server.code;
        currentCity = city.localizedName;

        // Back to the main view.

        await waitForQuery(screenHome.SERVER_LIST_BUTTON.visible());
        await waitForQuery(
            screenHome.SERVER_LIST_BUTTON_LABEL.visible().prop(
                'text', cityName));
        await waitForQueryAndClick(
            screenHome.SERVER_LIST_BUTTON.visible());
        await waitForQuery(screenHome.STACKVIEW.ready());
        await waitForQueryAndClick(screenHome.serverListView.ALL_SERVERS_TAB.visible());

        // One selected
        await waitForQuery(screenHome.STACKVIEW.ready());
        await waitForQuery(cityId.checked());
      }
    }
  });

  it('Server switching', async () => {
    await setSetting('serverSwitchNotification', 'true');
    await setSetting('connectionChangeNotification', 'true');

    await waitForQueryAndClick(
        screenHome.serverListView.BACK_BUTTON.visible());

    await activate();

    await waitForCondition(async () => {
      return await getQueryProperty(
                 screenHome.CONTROLLER_TITLE.visible(), 'text') ==
          'VPN is on';
    });

    let currentCountry = '';
    for (let server of servers) {
      if (server.code === currentCountryCode) {
        currentCountry = server.localizedName;
        break;
      }
    }

    assert(currentCountry != '');

    strictEqual(lastNotification().title, 'VPN Connected');
    strictEqual(
        lastNotification().message,
        `Connected through ${currentCity}`);

    await waitForQueryAndClick(
        screenHome.SERVER_LIST_BUTTON.visible());
    await waitForQuery(screenHome.STACKVIEW.ready());
    await waitForQueryAndClick(screenHome.serverListView.ALL_SERVERS_TAB.visible());

    let server;
    while (true) {
      server = servers[Math.floor(Math.random() * servers.length)];
      if (server.code != currentCountryCode) break;
    }

    const countryId =
        screenHome.serverListView.generateCountryId(server.code);
    await waitForQuery(countryId.visible());

    await scrollToQuery(
        screenHome.serverListView.COUNTRY_VIEW, countryId);
    await clickOnQuery(countryId);

    let city = server.cities[Math.floor(Math.random() * server.cities.length)];
    const cityId =
        screenHome.serverListView.generateCityId(countryId, city.name);
    await waitForQuery(cityId.visible());

    await setQueryProperty(
        screenHome.serverListView.COUNTRY_VIEW, 'contentY',
        parseInt(await getQueryProperty(cityId, 'y')) +
            parseInt(await getQueryProperty(countryId, 'y')));
    await wait();

    await clickOnQuery(cityId);
    await waitForQuery(screenHome.STACKVIEW.ready());

    const previousCountry = currentCountry;
    const previousCity = currentCity;

    currentCountryCode = server.code;
    currentCountry = server.localizedName;
    currentCity = city.localizedName;

    await waitForQuery(screenHome.CONTROLLER_TITLE.visible());

    await waitForCondition(async () => {
      let connectingMsg = await getQueryProperty(
          screenHome.CONTROLLER_TITLE.visible(), 'text');
      return connectingMsg === 'Switching…' || connectingMsg === 'VPN is on';
    });

    // Often the `From ${previousCity} to ${currentCity}` is too fast to be
    // visible. Let's keep this test.

    await waitForCondition(async () => {
      return await getQueryProperty(
                 screenHome.CONTROLLER_TITLE.visible(), 'text') ===
          'VPN is on';
    });

    strictEqual(lastNotification().title, 'VPN Switched Servers');
    strictEqual(
        lastNotification().message,
        `Switched from ${previousCity} to ${currentCity}`);
  });

  it('ensuring search message appears appropriately', async () => {
    // open it up
    await waitForQueryAndClick(
        screenHome.serverListView.SEARCH_BAR.visible());

    // ensure no message visible
    await waitForQuery(
        screenHome.serverListView.SEARCH_BAR_ERROR.hidden());
    // search down to one item - need to modify text within it
    await setQueryProperty(
        screenHome.serverListView.SEARCH_BAR_TEXT_FIELD, 'text',
        'Austra');
    // ensure no message visible
    await waitForQuery(
        screenHome.serverListView.SEARCH_BAR_ERROR.hidden());
    // search to zero items
    await setQueryProperty(
        screenHome.serverListView.SEARCH_BAR_TEXT_FIELD, 'text',
        'Austraz');
    // ensure message is visible
    await waitForQuery(
        screenHome.serverListView.SEARCH_BAR_ERROR.visible());
    // add another character
    await setQueryProperty(
        screenHome.serverListView.SEARCH_BAR_TEXT_FIELD, 'text',
        'Austrazz');
    // ensure message
    await waitForQuery(
        screenHome.serverListView.SEARCH_BAR_ERROR.visible());
    // delete a couple characters
    await setQueryProperty(
        screenHome.serverListView.SEARCH_BAR_TEXT_FIELD, 'text',
        'Austra');
    // ensure message disappears
    await waitForQuery(
        screenHome.serverListView.SEARCH_BAR_ERROR.hidden());
  });

  // TODO: server list disabled when reached the device limit

});
