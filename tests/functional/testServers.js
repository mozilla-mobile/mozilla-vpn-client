/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');

describe('Server list', function() {
  let servers;
  let currentCountryCode;
  let currentCity;

  this.timeout(240000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await vpn.waitForQueryAndClick(
        queries.screenHome.SERVER_LIST_BUTTON.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    servers = await vpn.servers();
    currentCountryCode =
        await vpn.getVPNProperty('VPNCurrentServer', 'exitCountryCode');
    currentCity = await vpn.getVPNProperty('VPNCurrentServer', 'exitCityName');

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
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.screenHome.SERVER_LIST_BUTTON.visible());
  });

  it('check the countries and cities', async () => {
    for (let server of servers) {
      const countryId =
          queries.screenHome.serverListView.generateCountryId(server.code);
      await vpn.waitForQuery(countryId.visible());

      await vpn.scrollToQuery(
          queries.screenHome.serverListView.COUNTRY_VIEW, countryId);

      if (currentCountryCode === server.code) {
        assert.equal(
            await vpn.getQueryProperty(countryId, 'cityListVisible'),
            'true');
      }

      if (await vpn.getQueryProperty(countryId, 'cityListVisible') ===
          'false') {
        await vpn.clickOnQuery(countryId);
      }

      for (let city of server.cities) {
        const cityId = queries.screenHome.serverListView.generateCityId(
            countryId, city.name);
        await vpn.waitForQuery(cityId.visible().prop(
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
          queries.screenHome.serverListView.generateCountryId(server.code);
      await vpn.waitForQuery(countryId.visible());

      await vpn.scrollToQuery(
          queries.screenHome.serverListView.COUNTRY_VIEW, countryId);

      if (await vpn.getQueryProperty(countryId, 'cityListVisible') ===
          'false') {
        await vpn.clickOnQuery(countryId);
      }

      await vpn.waitForQuery(countryId.prop('cityListVisible', true));

      for (let city of server.cities) {
        console.log('  Start test for city:', city);
        const cityId = queries.screenHome.serverListView.generateCityId(
            countryId, city.name);
        await vpn.waitForQuery(cityId);

        await vpn.setQueryProperty(
            queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
            parseInt(await vpn.getQueryProperty(cityId, 'y')) +
                parseInt(await vpn.getQueryProperty(countryId, 'y')));
        await vpn.waitForQuery(cityId.visible());

        const cityName =
            await vpn.getQueryProperty(cityId, 'radioButtonLabelText');

        await vpn.clickOnQuery(cityId);
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        currentCountryCode = server.code;
        currentCity = city.localizedName;

        // Back to the main view.

        await vpn.waitForQuery(queries.screenHome.SERVER_LIST_BUTTON.visible());
        await vpn.waitForQuery(
            queries.screenHome.SERVER_LIST_BUTTON_LABEL.visible().prop(
                'text', cityName));
        await vpn.waitForQueryAndClick(
            queries.screenHome.SERVER_LIST_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // One selected
        await vpn.waitForQuery(cityId.checked());
      }
    }
  });

  it('Server switching', async () => {
    await vpn.setSetting('serverSwitchNotification', 'true');
    await vpn.setSetting('connectionChangeNotification', 'true');

    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.BACK_BUTTON.visible());

    await vpn.activate();

    await vpn.waitForCondition(async () => {
      return await vpn.getQueryProperty(
                 queries.screenHome.CONTROLLER_TITLE.visible(), 'text') ==
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

    assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');
    assert.strictEqual(
        vpn.lastNotification().message,
        `Connected to ${currentCity}`);

    await vpn.waitForQueryAndClick(
        queries.screenHome.SERVER_LIST_BUTTON.visible());

    let server;
    while (true) {
      server = servers[Math.floor(Math.random() * servers.length)];
      if (server.code != currentCountryCode) break;
    }

    const countryId =
        queries.screenHome.serverListView.generateCountryId(server.code);
    await vpn.waitForQuery(countryId.visible());

    await vpn.scrollToQuery(
        queries.screenHome.serverListView.COUNTRY_VIEW, countryId);
    await vpn.clickOnQuery(countryId);

    let city = server.cities[Math.floor(Math.random() * server.cities.length)];
    const cityId =
        queries.screenHome.serverListView.generateCityId(countryId, city.name);
    await vpn.waitForQuery(cityId.visible());

    await vpn.setQueryProperty(
        queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
        parseInt(await vpn.getQueryProperty(cityId, 'y')) +
            parseInt(await vpn.getQueryProperty(countryId, 'y')));
    await vpn.wait();

    await vpn.clickOnQuery(cityId);
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    const previousCountry = currentCountry;
    const previousCity = currentCity;

    currentCountryCode = server.code;
    currentCountry = server.localizedName;
    currentCity = city.localizedName;

    await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());

    await vpn.waitForCondition(async () => {
      let connectingMsg = await vpn.getQueryProperty(
          queries.screenHome.CONTROLLER_TITLE.visible(), 'text');
      return connectingMsg === 'Switching…' || connectingMsg === 'VPN is on';
    });

    // Often the `From ${previousCity} to ${currentCity}` is too fast to be
    // visible. Let's keep this test.

    await vpn.waitForCondition(async () => {
      return await vpn.getQueryProperty(
                 queries.screenHome.CONTROLLER_TITLE.visible(), 'text') ===
          'VPN is on';
    });

    assert.strictEqual(vpn.lastNotification().title, 'VPN Switched Servers');
    assert.strictEqual(
        vpn.lastNotification().message,
        `Switched from ${previousCity} to ${currentCity}`);
  });

  it('ensuring search message appears appropriately', async () => {
    // open it up
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.SEARCH_BAR.visible());

    // ensure no message visible
    await vpn.waitForQuery(
        queries.screenHome.serverListView.SEARCH_BAR_ERROR.hidden());
    // search down to one item - need to modify text within it
    await vpn.setQueryProperty(
        queries.screenHome.serverListView.SEARCH_BAR_TEXT_FIELD, 'text',
        'Austra');
    // ensure no message visible
    await vpn.waitForQuery(
        queries.screenHome.serverListView.SEARCH_BAR_ERROR.hidden());
    // search to zero items
    await vpn.setQueryProperty(
        queries.screenHome.serverListView.SEARCH_BAR_TEXT_FIELD, 'text',
        'Austraz');
    // ensure message is visible
    await vpn.waitForQuery(
        queries.screenHome.serverListView.SEARCH_BAR_ERROR.visible());
    // add another character
    await vpn.setQueryProperty(
        queries.screenHome.serverListView.SEARCH_BAR_TEXT_FIELD, 'text',
        'Austrazz');
    // ensure message
    await vpn.waitForQuery(
        queries.screenHome.serverListView.SEARCH_BAR_ERROR.visible());
    // delete a couple characters
    await vpn.setQueryProperty(
        queries.screenHome.serverListView.SEARCH_BAR_TEXT_FIELD, 'text',
        'Austra');
    // ensure message disappears
    await vpn.waitForQuery(
        queries.screenHome.serverListView.SEARCH_BAR_ERROR.hidden());
  });

  // TODO: server list disabled when reached the device limit

});
