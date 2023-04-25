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
  let previousCity;
  let currentCountry;

  //   this.timeout(240000);
  this.timeout(30000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await vpn.waitForQueryAndClick(queries.screenHome.SERVER_LIST_BUTTON);
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    servers = await vpn.servers();
    currentCountryCode = await vpn.getMozillaProperty(
        'Mozilla.VPN', 'VPNCurrentServer', 'exitCountryCode');
    currentCity = await vpn.getMozillaProperty(
        'Mozilla.VPN', 'VPNCurrentServer', 'exitCityName');
    previousCity = await vpn.getMozillaProperty(
        'Mozilla.VPN', 'VPNCurrentServer', 'entryCityName');

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

  it('opening the entry and exit server list', async () => {
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());

    await vpn.waitForQuery(
        queries.screenHome.serverListView.VPN_COLLAPSIBLE_CARD.visible());
    assert.equal(
        await vpn.getQueryProperty(
            queries.screenHome.serverListView.VPN_COLLAPSIBLE_CARD, 'expanded'),
        'false');

    await vpn.waitForQuery(
        queries.screenHome.serverListView.ENTRY_BUTTON.visible());
    await vpn.waitForQuery(
        queries.screenHome.serverListView.EXIT_BUTTON.visible());
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.VPN_MULTHOP_CHEVRON.visible());

    assert.equal(
        await vpn.getQueryProperty(
            queries.screenHome.serverListView.VPN_COLLAPSIBLE_CARD, 'expanded'),
        'false');
  });

  it('check the countries and cities for multihop entries', async () => {
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.ENTRY_BUTTON.visible());

    for (let server of servers) {
      const countryId =
          queries.screenHome.serverListView.generateCountryId(server.code);
      await vpn.waitForQuery(countryId.visible());

      await vpn.setQueryProperty(
          queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
          parseInt(await vpn.getQueryProperty(countryId, 'y')));

      await vpn.wait();

      if (currentCountryCode === server.code) {
        assert.equal(
            await vpn.getQueryProperty(countryId, 'cityListVisible'), 'true');
      }

      if ((await vpn.getQueryProperty(countryId, 'cityListVisible')) ===
          'false') {
        await vpn.clickOnQuery(countryId);
      }

      for (let city of server.cities) {
        const cityId = queries.screenHome.serverListView.generateCityId(
            countryId, city.name);
        console.log('  Waiting for cityId:', cityId);
        await vpn.waitForQuery(cityId.visible());
      }
    }
  });

  it('check the countries and cities for multihop exits', async () => {
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    await vpn.wait();
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.EXIT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    for (let server of servers) {
      const countryId =
          queries.screenHome.serverListView.generateCountryId(server.code);
      await vpn.waitForQuery(countryId.visible());

      await vpn.setQueryProperty(
          queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
          parseInt(await vpn.getQueryProperty(countryId, 'y')));

      if (currentCountryCode === server.code) {
        assert(
            (await vpn.getQueryProperty(countryId, 'cityListVisible')) ===
            'true');
      }

      if ((await vpn.getQueryProperty(countryId, 'cityListVisible')) ===
          'false') {
        await vpn.clickOnQuery(countryId);
      }

      for (let city of server.cities) {
        const cityId = queries.screenHome.serverListView.generateCityId(
            countryId, city.name);
        console.log('  Waiting for cityId:', cityId);
        await vpn.waitForQuery(cityId.visible().prop(
            'checked',
            currentCountryCode === server.code &&
                currentCity === city.localizedName));
      }
    }
  });

  it('Pick cities for entries', async () => {
    let countryId;

    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.ENTRY_BUTTON.visible());

    for (let server of servers) {
      countryId =
          queries.screenHome.serverListView.generateCountryId(server.code);
      await vpn.waitForQuery(countryId.visible());

      await vpn.setQueryProperty(
          queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
          parseInt(await vpn.getQueryProperty(countryId, 'y')));

      await vpn.waitForQuery(countryId.visible());
      await vpn.scrollToQuery(
          queries.screenHome.serverListView.COUNTRY_VIEW, countryId);
      await vpn.clickOnQuery(countryId);

      for (let city of server.cities) {
        const cityId = countryId + '/serverCityList/serverCity-' +
            city.name.replace(/ /g, '_');
        await vpn.waitForQuery(cityId);

        await vpn.setQueryProperty(
            queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
            parseInt(await vpn.getQueryProperty(countryId, 'y')));

        await vpn.waitForQuery(cityId);
        const cityName = await vpn.getQueryProperty(
            cityId, 'radioButtonLabelText'.split(' '));

        assert(cityName.includes(city.name));
      }
    }
  });

  it('Pick cities for exits', async () => {
    let countryId;
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.EXIT_BUTTON.visible());

    for (let server of servers) {
      countryId =
          queries.screenHome.serverListView.generateCountryId(server.code);
      await vpn.waitForQuery(countryId.visible());

      await vpn.setQueryProperty(
          queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
          parseInt(await vpn.getQueryProperty(countryId, 'y')));

      await vpn.waitForQuery(countryId.visible());
      await vpn.scrollToQuery(
          queries.screenHome.serverListView.COUNTRY_VIEW, countryId);
      await vpn.clickOnQuery(countryId);

      for (let city of server.cities) {
        const cityId = countryId + '/serverCityList/serverCity-' +
            city.name.replace(/ /g, '_');
        await vpn.waitForQuery(cityId);

        await vpn.setQueryProperty(
            queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
            parseInt(await vpn.getQueryProperty(countryId, 'y')));

        await vpn.waitForQuery(cityId);
        const cityName = await vpn.getQueryProperty(
            cityId, 'radioButtonLabelText'.split(' '));
        assert(cityName.includes(city.name));
      }
    }
  });

  it.skip('Server switching -- same country different cities', async () => {
    await vpn.setSetting('serverSwitchNotification', true);
    await vpn.setSetting('connectionChangeNotification', true);

    // wait for select entry and select entry
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.ENTRY_BUTTON.visible());

    // exit server details
    const firstServer = servers[0];
    const cityTwo = firstServer.cities[0];
    const cityThree = firstServer.cities[1];
    const exitFirstCountryId =
        queries.screenHome.serverListView.generateCountryId(firstServer.code);

    // entry server details
    const secondServer = servers[1];
    const cityOne = secondServer.cities[0];
    const entryCountryId =
        queries.screenHome.serverListView.generateCountryId(secondServer.code);

    // select the first country
    await vpn.waitForQuery(entryCountryId.visible());
    await vpn.scrollToQuery(
        queries.screenHome.serverListView.COUNTRY_VIEW, entryCountryId);
    await vpn.clickOnQuery(entryCountryId);

    // select first city
    const cityOneId = queries.screenHome.serverListView.generateCityId(
        entryCountryId, cityOne.name);

    // await vpn.waitForQueryAndClick(entryCountryId.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    // await vpn.waitForQueryAndClick(cityOneId.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    // select exit country again
    const cityTwoId = queries.screenHome.serverListView.generateCityId(
        exitFirstCountryId, cityTwo.name);

    await vpn.wait();
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.EXIT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(exitFirstCountryId.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(cityTwoId.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    // navigate back to connection view
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    // connect vpn
    await vpn.activate();

    // wait and assert vpn connection
    await vpn.waitForCondition(async () => {
      return (
          (await vpn.getQueryProperty(
              queries.screenHome.CONTROLLER_TITLE, 'text')) == 'VPN is on');
    });
    assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');
    assert(vpn.lastNotification().message.includes(
        `Connected through ${cityTwo.localizedName}`));

    await vpn.waitForQueryAndClick(
        queries.screenHome.SERVER_LIST_BUTTON.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    // Back at the main view. select the exit entries
    await vpn.wait();
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.EXIT_BUTTON.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    // select first city in exit country
    const cityThreeId = queries.screenHome.serverListView.generateCityId(
        exitFirstCountryId, cityThree.name);

    await vpn.waitForQueryAndClick(entryCountryId.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(cityThreeId.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    // wait and assert server switching for multihop
    await vpn.waitForCondition(async () => {
      return vpn.lastNotification().title == 'VPN Switched Servers';
    }, 20);

    assert(vpn.lastNotification().message.includes(`Switched from ${
        cityTwo.localizedName} to ${cityThree.localizedName}`));

    await vpn.deactivate();
  });

  it.skip(
      'Server switching -- different country different cities', async () => {
        await vpn.setSetting('serverSwitchNotification', true);
        await vpn.setSetting('connectionChangeNotification', true);

        // wait for select entry and select entry
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.ENTRY_BUTTON.visible());

        // entry server details
        const secondServer = servers[1];
        const cityOne = secondServer.cities[0];
        const entryCountryId =
            queries.screenHome.serverListView.generateCountryId(
                secondServer.code);

        // second exit server details
        const thirdServer = servers[2];
        const cityThree = thirdServer.cities[0];
        const exitThirdCountryId =
            queries.screenHome.serverListView.generateCountryId(
                thirdServer.code);

        // exit server details
        const firstServer = servers[0];
        const cityTwo = firstServer.cities[0];
        const exitFirstCountryId =
            queries.screenHome.serverListView.generateCountryId(
                firstServer.code);

        // select the first country
        await vpn.waitForQuery(entryCountryId.visible());
        await vpn.scrollToQuery(
            queries.screenHome.serverListView.COUNTRY_VIEW, entryCountryId);
        await vpn.clickOnQuery(entryCountryId);

        // select first city
        const cityOneId = queries.screenHome.serverListView.generateCityId(
            entryCountryId, cityOne.name);

        // await vpn.waitForQueryAndClick(entryCountryId.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // await vpn.waitForQueryAndClick(cityOneId.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // select exit country
        const cityTwoId = queries.screenHome.serverListView.generateCityId(
            exitFirstCountryId, cityTwo.name);

        await vpn.wait();
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.EXIT_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        await vpn.waitForQueryAndClick(exitFirstCountryId.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // await vpn.waitForQueryAndClick(cityTwoId.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // navigate back to connection view
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.BACK_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // connect vpn
        await vpn.activate();

        // wait and assert vpn connection
        await vpn.waitForCondition(async () => {
          return (
              (await vpn.getQueryProperty(
                  queries.screenHome.CONTROLLER_TITLE, 'text')) == 'VPN is on');
        });
        assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');
        assert(vpn.lastNotification().message.includes(
            `Connected through ${cityOne.localizedName}`));

        // await vpn.waitForQueryAndClick(
        //     queries.screenHome.SERVER_LIST_BUTTON.visible());
        // await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // Back at the main view. select the exit entries
        // await vpn.wait();
        // await vpn.waitForQueryAndClick(
        //     queries.screenHome.serverListView.EXIT_BUTTON.visible());
        // await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // select first city in exit country
        // const cityThreeId = queries.screenHome.serverListView.generateCityId(
        //     exitThirdCountryId, cityThree.name);

        // await vpn.scrollToQuery(
        //     queries.screenHome.serverListView.COUNTRY_VIEW,
        //     exitThirdCountryId);
        // await vpn.waitForQueryAndClick(exitThirdCountryId.visible());
        // await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // await vpn.waitForQueryAndClick(cityThreeId.visible());
        // await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // await vpn.waitForQueryAndClick(
        //     queries.screenHome.serverListView.BACK_BUTTON.visible());
        // await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // // wait and assert server switching for multihop
        // await vpn.waitForCondition(async () => {
        //   return vpn.lastNotification().title == 'VPN Switched Servers';
        // }, 20);
        // assert.strictEqual(
        //     vpn.lastNotification().message,
        //     `Switched from ${cityTwo.localizedName} to ${
        //         cityThree.localizedName}`);

        // await vpn.deactivate();
      });

  it('Single and multihop switching', async () => {
    await vpn.setSetting('serverSwitchNotification', true);
    await vpn.setSetting('connectionChangeNotification', true);

    // wait for select entry and select entry
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.ENTRY_BUTTON.visible());

    // entry server details
    const secondServer = servers[1];
    const cityOne = secondServer.cities[0];
    const currentCity = cityOne.localizedName;
    const entryCountryId =
        queries.screenHome.serverListView.generateCountryId(secondServer.code);

    // select the first country
    console.log('first server is: ', secondServer);
    await vpn.waitForQuery(entryCountryId.visible());
    await vpn.scrollToQuery(
        queries.screenHome.serverListView.COUNTRY_VIEW, entryCountryId);
    await vpn.clickOnQuery(entryCountryId);

    // select first city
    const cityOneId = queries.screenHome.serverListView.generateCityId(
        entryCountryId, cityOne.name);

    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    // connect vpn
    await vpn.activate();

    // wait and assert vpn connection
    await vpn.waitForCondition(async () => {
      return (
          (await vpn.getQueryProperty(
              queries.screenHome.CONTROLLER_TITLE, 'text')) == 'VPN is on');
    });

    assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');
    assert(vpn.lastNotification().message.includes(
        `Connected through ${currentCity}`));

    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    // switch from multihop to singlehop
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.SINGLEHOP_SELECTOR_TAB.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.BACK_BUTTON.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    // wait and assert vpn connection
    await vpn.waitForCondition(async () => {
      return (
          (await vpn.getQueryProperty(
              queries.screenHome.CONTROLLER_TITLE, 'text')) == 'VPN is on');
    });

    // assert switch to single hop
    assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');
    assert(vpn.lastNotification().message.includes(
        `Connected through ${currentCity}`));

    await vpn.deactivate();
  });
});