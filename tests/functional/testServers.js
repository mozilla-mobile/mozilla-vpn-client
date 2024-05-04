/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');

describe('Server', function() {
  this.timeout(240000);
  this.ctx.authenticationNeeded = true;

  describe('General server list tests', function() {
    let servers;
    let currentCountryCode;
    let currentCity;

    beforeEach(async () => {
      await vpn.waitForQueryAndClick(
          queries.screenHome.SERVER_LIST_BUTTON.visible());
      await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());
      await vpn.waitForQueryAndClick(queries.screenHome.serverListView.ALL_SERVERS_TAB.visible());

      servers = await vpn.servers();
      currentCountryCode = await vpn.getMozillaProperty(
          'Mozilla.VPN', 'VPNCurrentServer', 'exitCountryCode');
      currentCity = await vpn.getMozillaProperty(
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
      await vpn.waitForQueryAndClick(
          queries.screenHome.serverListView.BACK_BUTTON.visible());
      await vpn.waitForQuery(queries.screenHome.SERVER_LIST_BUTTON.visible());
    });

    it('check the countries and cities', async () => {
      for (let server of servers) {
        const countryId =
            queries.screenHome.serverListView.generateCountryId(server.code);
        await vpn.navServerList(countryId);

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
        await vpn.navServerList(countryId);

        let countryY = parseInt(await vpn.getQueryProperty(countryId, 'y'));
        for (let city of server.cities) {
          console.log('  Start test for city:', city);
          const cityId = queries.screenHome.serverListView.generateCityId(
              countryId, city.name);
          await vpn.waitForQuery(cityId);

          await vpn.scrollToQuery(
              queries.screenHome.serverListView.COUNTRY_VIEW, cityId);
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
          await vpn.waitForQueryAndClick(queries.screenHome.serverListView.ALL_SERVERS_TAB.visible());

          // One selected
          await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());
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

      assert.equal(
          await vpn.getQueryProperty(
              queries.screenHome.CONNECTION_TIMER,
              'connectionTimeForFunctionalTest'),
          '00:00:00');

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
          `Connected through ${currentCity}`);

      await vpn.waitForQueryAndClick(
          queries.screenHome.SERVER_LIST_BUTTON.visible());
      await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());
      await vpn.waitForQueryAndClick(queries.screenHome.serverListView.ALL_SERVERS_TAB.visible());

      let server;
      while (true) {
        server = servers[Math.floor(Math.random() * servers.length)];
        if (server.code != currentCountryCode) break;
      }

      const countryId =
          queries.screenHome.serverListView.generateCountryId(server.code);
      let city = server.cities[Math.floor(Math.random() * server.cities.length)];
      const cityId =
          queries.screenHome.serverListView.generateCityId(countryId, city.name);
      await vpn.navServerList(countryId, cityId);

      await vpn.waitForQueryAndClick(cityId.visible());
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

      assert.notEqual(
          await vpn.getQueryProperty(
              queries.screenHome.CONNECTION_TIMER,
              'connectionTimeForFunctionalTest'),
          '00:00:00');

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

    it('check the location help sheet', async () => {
      await vpn.waitForQueryAndClick(queries.screenHome.serverListView.HELP_BUTTON.visible());
      await vpn.waitForQuery(queries.screenHome.serverListView.HELP_SHEET.opened());
      await vpn.waitForQueryAndClick(queries.screenHome.serverListView.HELP_SHEET_LEARN_MORE_BUTTON.visible());
      await vpn.waitForCondition(async () => {
        const url = await vpn.getLastUrl();
        return url === 'https://support.mozilla.org/kb/multi-hop-encrypt-your-data-twice-enhanced-security';
      });
      await vpn.waitForQueryAndClick(queries.screenHome.serverListView.HELP_SHEET_CLOSE_BUTTON.visible());
      await vpn.waitForQuery(queries.screenHome.serverListView.HELP_SHEET.closed());
    });
  });


  describe('checking location screen telemetry', function () {
    // No Glean on WASM.
    if(vpn.runningOnWasm()) {
      return;
    }

    const locationSheetTelemetryScreenId = "location_info"

    describe('checking location screen telemetry (singlehop)', function () {
      const singleHopTelemetryScreenId = "location_singlehop"

      it('checking location screen impression telemetry on server view open', async () => {
        //Ensures the server view opens to multihop
        await vpn.setSetting('serverData', '{"enter_city_name":"","enter_country_code":"","exit_city_name":"Melbourne","exit_country_code":"au"}');
        await vpn.waitForQueryAndClick(queries.screenHome.SERVER_LIST_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        const locationSinglehopScreenEvents = await vpn.gleanTestGetValue("impression", "locationSinglehopScreen", "main");
        assert.equal(locationSinglehopScreenEvents.length, 1);
        const locationSinglehopScreenEventsExtras = locationSinglehopScreenEvents[0].extra;
        assert.equal(singleHopTelemetryScreenId, locationSinglehopScreenEventsExtras.screen);
      });

      it('checking location screen impression telemetry on segment change', async () => {
        //Open's the server view to multihop
        await vpn.setSetting('serverData', '{"enter_city_name":"Atlanta","enter_country_code":"us","exit_city_name":"Melbourne","exit_country_code":"au"}');
        await vpn.waitForQueryAndClick(queries.screenHome.SERVER_LIST_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());
        await vpn.waitForQueryAndClick(queries.screenHome.serverListView.SINGLEHOP_SELECTOR_TAB.visible());

        const locationSinglehopScreenEvents = await vpn.gleanTestGetValue("impression", "locationSinglehopScreen", "main");
        assert.equal(locationSinglehopScreenEvents.length, 1);
        const locationSinglehopScreenEventsExtras = locationSinglehopScreenEvents[0].extra;
        assert.equal(singleHopTelemetryScreenId, locationSinglehopScreenEventsExtras.screen);
      });

      it('checking location help tooltip telemetry', async () => {
        await vpn.waitForQueryAndClick(queries.screenHome.SERVER_LIST_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        await vpn.waitForQueryAndClick(queries.screenHome.serverListView.HELP_BUTTON.visible());

        const helpTooltipSelectedEvents = await vpn.gleanTestGetValue("interaction", "helpTooltipSelected", "main");
        assert.equal(helpTooltipSelectedEvents.length, 1);
        const helpTooltipSelectedEventsExtras = helpTooltipSelectedEvents[0].extra;
        assert.equal(singleHopTelemetryScreenId, helpTooltipSelectedEventsExtras.screen);

        await vpn.waitForQuery(queries.screenHome.serverListView.HELP_SHEET.opened());

        const locationInfoScreenEvents = await vpn.gleanTestGetValue("impression", "locationInfoScreen", "main");
        assert.equal(locationInfoScreenEvents.length, 1);
        const locationInfoScreenEventsExtras = locationInfoScreenEvents[0].extra;
        assert.equal(locationSheetTelemetryScreenId, locationInfoScreenEventsExtras.screen);

        await vpn.waitForQueryAndClick(queries.screenHome.serverListView.HELP_SHEET_LEARN_MORE_BUTTON.visible());

        const learnMoreSelectedEvents = await vpn.gleanTestGetValue("interaction", "learnMoreSelected", "main");
        assert.equal(learnMoreSelectedEvents.length, 1);
        const learnMoreSelectedEventsExtras = learnMoreSelectedEvents[0].extra;
        assert.equal(locationSheetTelemetryScreenId, learnMoreSelectedEventsExtras.screen);
      });
    });

    describe('checking location screen telemetry (multihop)', function () {
      const multiHopTelemetryScreenId = "location_multihop"

      it('checking location screen impression telemetry on server view open', async () => {
        //Ensures the server view opens to multihop
        await vpn.setSetting('serverData', '{"enter_city_name":"Atlanta","enter_country_code":"us","exit_city_name":"Melbourne","exit_country_code":"au"}');
        await vpn.waitForQueryAndClick(queries.screenHome.SERVER_LIST_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        const locationMultihopScreenEvents = await vpn.gleanTestGetValue("impression", "locationMultihopScreen", "main");
        assert.equal(locationMultihopScreenEvents.length, 1);
        const locationMultihopScreenEventsExtras = locationMultihopScreenEvents[0].extra;
        assert.equal(multiHopTelemetryScreenId, locationMultihopScreenEventsExtras.screen);
      });

      it('checking location screen impression telemetry on segment change', async () => {
        //Open's the server view to singlehop
        await vpn.setSetting('serverData', '{"enter_city_name":"","enter_country_code":"","exit_city_name":"Melbourne","exit_country_code":"au"}');
        await vpn.waitForQueryAndClick(queries.screenHome.SERVER_LIST_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());
        await vpn.waitForQueryAndClick(queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());

        const locationMultihopScreenEvents = await vpn.gleanTestGetValue("impression", "locationMultihopScreen", "main");
        assert.equal(locationMultihopScreenEvents.length, 1);
        const locationMultihopScreenEventsExtras = locationMultihopScreenEvents[0].extra;
        assert.equal(multiHopTelemetryScreenId, locationMultihopScreenEventsExtras.screen);
      });

      it('checking select location help tooltip telemetry', async () => {
        //Open's the server view to multihop
        await vpn.setSetting('serverData', '{"enter_city_name":"Atlanta","enter_country_code":"us","exit_city_name":"Melbourne","exit_country_code":"au"}');
        await vpn.waitForQueryAndClick(queries.screenHome.SERVER_LIST_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        await vpn.waitForQueryAndClick(queries.screenHome.serverListView.HELP_BUTTON.visible());

        //We only need to test the helpTooltipSelected event as it has a different extra key value (multihop instead of singlehop)
        //Eventhing else related to the sheet is tested in the singlehop tests
        const helpTooltipSelectedEvents = await vpn.gleanTestGetValue("interaction", "helpTooltipSelected", "main");
        assert.equal(helpTooltipSelectedEvents.length, 1);
        const helpTooltipSelectedEventsExtras = helpTooltipSelectedEvents[0].extra;
        assert.equal(multiHopTelemetryScreenId, helpTooltipSelectedEventsExtras.screen);

        await vpn.waitForQuery(queries.screenHome.serverListView.HELP_SHEET.opened());
      });
    });

  });

  // TODO: server list disabled when reached the device limit

});
