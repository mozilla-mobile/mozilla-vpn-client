/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import assert, { strictEqual, equal } from 'assert';
import { screenHome } from './queries.js';
import { setQueryProperty, getQueryProperty, waitForQueryAndClick, waitForQuery, servers as _servers, getMozillaProperty, wait, clickOnQuery, setSetting, activate, waitForCondition, lastNotification, getElementProperty, waitForElementProperty, waitForElementAndClick } from './helper.js';

async function selectCityFromList(cityId, countryId) {
  await setQueryProperty(
      screenHome.serverListView.COUNTRY_VIEW, 'contentY',
      parseInt(await getQueryProperty(cityId, 'y')) +
          parseInt(await getQueryProperty(countryId, 'y')));
}

async function selectCountryFromList(countryId) {
  await setQueryProperty(
      screenHome.serverListView.COUNTRY_VIEW, 'contentY',
      parseInt(await getQueryProperty(countryId, 'y')));
}

describe('Server list', function() {
  let servers;
  let currentCountryCode;
  let currentCity;

  this.timeout(240000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await waitForQueryAndClick(screenHome.SERVER_LIST_BUTTON);
    await waitForQuery(screenHome.STACKVIEW.ready());

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
        'Current exit city (localized):', currentCity,
        '| Current exit country code:', currentCountryCode);
  });

  it('opening the entry and exit server list', async () => {
    await waitForQueryAndClick(
        screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
    await waitForQuery(
        screenHome.serverListView.VPN_COLLAPSIBLE_CARD.visible());

    assert(
        await getQueryProperty(
            screenHome.serverListView.VPN_COLLAPSIBLE_CARD,
            'expanded') === 'false');

    await waitForQuery(
        screenHome.serverListView.ENTRY_BUTTON.visible());

    await waitForQuery(
        screenHome.serverListView.EXIT_BUTTON.visible());

    await waitForQueryAndClick(
        screenHome.serverListView.VPN_MULTHOP_CHEVRON.visible())
    assert(await getQueryProperty(
        screenHome.serverListView.VPN_COLLAPSIBLE_CARD, 'expanded'))
  });

  it('check the countries and cities for multihop entries', async () => {
    /**
     * This test navigates to the Multi-Hop -> ENTRY_BUTTON
     * View and Makes sure:
     * -> For every Server there are cities.
     */
    await waitForQueryAndClick(
        screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
    await waitForQueryAndClick(
        screenHome.serverListView.ENTRY_BUTTON.visible());
    // Scope all Queries to this one, so that we are sure to be using the right
    // element.
    const multiHopStackView = screenHome.serverListView.MULTIHOP_VIEW;


    for (let server of servers) {
      const country_query =
          screenHome.serverListView.generateCountryId(server.code);
      // Scope country_query to be inside multiHopStackView
      // make sure we are querying the correct list :)
      const countryId = multiHopStackView.query(country_query);
      await waitForQuery(countryId.visible());

      await setQueryProperty(
          screenHome.serverListView.COUNTRY_VIEW, 'contentY',
          parseInt(await getQueryProperty(countryId, 'y')));

      await wait();


      if (await getQueryProperty(countryId, 'cityListVisible') ===
          'false') {
        await clickOnQuery(countryId);
      }

      for (let city of server.cities) {
        const cityId_query = screenHome.serverListView.generateCityId(
            country_query, city.name);
        const cityId = multiHopStackView.query(cityId_query);
        console.log('  Waiting for cityId:', cityId);
        await waitForQuery(cityId.visible());
      }
    }
  })

  // Skipping cause too flaky on wasm.
  it.skip('check the countries and cities for multihop exits', async () => {
    /**
     * This test does the same as above?
     * But also checks:
     * -> The Selected Exit server is Expanded
     * -> The Selected City is selected
     */
    await waitForQueryAndClick(
        screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
    await waitForQueryAndClick(
        screenHome.serverListView.EXIT_BUTTON.visible());

    // Scope all Queries to this one, so that we are sure to be using the right
    // element.
    const multiHopStackView = screenHome.serverListView.MULTIHOP_VIEW;


    for (let server of servers) {
      const country =
          screenHome.serverListView.generateCountryId(server.code);
      const countryId = multiHopStackView.query(country);

      await waitForQuery(countryId.visible());

      await selectCountryFromList(countryId);
      await wait();

      if (currentCountryCode === server.code) {
        assert(
            (await getQueryProperty(countryId, 'cityListVisible')) ===
            'true');
      }

      if ((await getQueryProperty(countryId, 'cityListVisible')) ===
          'false') {
        await clickOnQuery(countryId);
      }

      for (let city of server.cities) {
        const cityId_query = screenHome.serverListView.generateCityId(
            country, city.name);
        const cityId = multiHopStackView.query(cityId_query);

        await waitForQuery(cityId);
        await getQueryProperty(cityId, 'visible', 'true');
        await getQueryProperty(
            cityId, 'checked',
            currentCountryCode === server.code &&
                    currentCity === city.localizedName ?
                'true' :
                'false');
      }
    }
  });

  it('Picking a  cities brings you back to Multi-Hop View', async () => {
    await waitForQueryAndClick(
        screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
    await waitForQueryAndClick(
        screenHome.serverListView.ENTRY_BUTTON.visible());

    // Scope all Queries to this one, so that we are sure to be using the right
    // element.
    const multiHopStackView = screenHome.serverListView.MULTIHOP_VIEW;
    let [server] = servers;
    let country =
        screenHome.serverListView.generateCountryId(server.code);
    let countryId = multiHopStackView.query(country);
    await waitForQuery(countryId.visible());
    await selectCountryFromList(countryId);
    await wait();

    if (await getQueryProperty(countryId, 'cityListVisible') === 'false') {
      await clickOnQuery(countryId);
    }
    let [city] = server.cities
    const cityId_query =
        screenHome.serverListView.generateCityId(country, city.name);
    const cityId = multiHopStackView.query(cityId_query);

    await waitForQuery(cityId);
    await clickOnQuery(cityId);
    // We should now be back on the overview
    await waitForQuery(
        screenHome.serverListView.ENTRY_BUTTON.visible());
  });

  it.skip('Pick cities for exits', async () => {
    let countryId;
    await waitForQueryAndClick(
        screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
    await waitForQueryAndClick(
        screenHome.serverListView.EXIT_BUTTON.visible());

    for (let server of servers) {
      countryId =
          screenHome.serverListView.generateCountryId(server.code);
      await waitForQuery(countryId.visible());

      await selectCountryFromList(countryId);
      await wait();

      if (await getQueryProperty(countryId, 'cityListVisible') ===
          'false') {
        await clickOnQuery(countryId);
      }

      for (let city of server.cities) {
        const cityId = countryId + '/serverCityList/serverCity-' +
            city.name.replace(/ /g, '_');
        await waitForQuery(cityId);

        await selectCityFromList(cityId, countryId)
            await getQueryProperty(cityId, 'visible', 'true');

        const cityName = await getQueryProperty(
            cityId, 'radioButtonLabelText'.split(' '));

        await wait();
        await clickOnQuery(cityId);
        await wait();

        // One selected
        await waitForQuery(cityId);
        await getQueryProperty(cityId, 'checked', 'true');
        assert(cityName.includes(city.name))
      }
    }
  });

  it.skip('Server switching -- same country different cities', async () => {
    await setSetting('serverSwitchNotification', true);
    await setSetting('connectionChangeNotification', true);

    let newCurrentCountry;
    let newCurrentCity;
    let currentCountry;
    let currentCity;

    // wait for select entry and select entry
    await waitForQueryAndClick(
        screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
    await waitForQueryAndClick(
        screenHome.serverListView.ENTRY_BUTTON.visible());

    // exit server details
    const firstServer = servers[0];
    const cityTwo = firstServer.cities[0];
    const cityThree = firstServer.cities[1];
    const exitFirstCountryId =
        screenHome.serverListView.generateCountryId(firstServer.code);

    // entry server details
    const secondServer = servers[1];
    const cityOne = secondServer.cities[0];
    const entryCountryId =
        screenHome.serverListView.generateCountryId(secondServer.code);

    // select the first country
    await selectCountryFromList(entryCountryId);
    await wait()
    if (await getQueryProperty(entryCountryId, 'cityListVisible') ===
        'false') {
      await clickOnQuery(entryCountryId);
    }
    await getQueryProperty(entryCountryId, 'cityListVisible', 'true');

    // select first city
    const cityOneId = screenHome.serverListView.generateCityId(
        entryCountryId, cityOne.name);

    await selectCityFromList(cityOneId, entryCountryId);
    await wait();
    await waitForQueryAndClick(cityOneId);

    // Back at the main view. select the exit entries
    await waitForQueryAndClick(
        screenHome.serverListView.EXIT_BUTTON.visible());

    // select first country again
    await selectCountryFromList(exitFirstCountryId);
    await wait();
    if (await getQueryProperty(exitFirstCountryId, 'cityListVisible') ===
        'false') {
      await clickOnQuery(exitFirstCountryId);
    }
    await getQueryProperty(exitFirstCountryId, 'cityListVisible', 'true');

    // select first city in exit country
    const cityTwoId = screenHome.serverListView.generateCityId(
        exitFirstCountryId, cityTwo.name);

    await selectCityFromList(cityTwoId, exitFirstCountryId);
    await wait();
    await waitForQueryAndClick(cityTwoId);

    // navigate back to connection view
    await waitForQueryAndClick(
        screenHome.serverListView.BACK_BUTTON.visible());

    // define connected server
    currentCountry = firstServer.localizedName;
    currentCity = cityTwo.localizedName;

    // connect vpn
    await activate();

    // wait and assert vpn connection
    await waitForCondition(async () => {
      return await getQueryProperty(
                 screenHome.CONTROLLER_TITLE, 'text') == 'VPN is on';
    });
    strictEqual(lastNotification().title, 'VPN Connected');
    strictEqual(
        lastNotification().message, `Connected through ${currentCity}`);

    // back to main view
    await waitForQueryAndClick(screenHome.SERVER_LIST_BUTTON);

    // Back at the main view. select the exit entries
    await waitForQueryAndClick(
        screenHome.serverListView.EXIT_BUTTON.visible());

    // select first country again
    await selectCountryFromList(exitFirstCountryId);
    await wait();
    if (await getElementProperty(exitFirstCountryId, 'cityListVisible') ===
        'false') {
      await clickOnQuery(exitFirstCountryId);
    }
    await waitForElementProperty(
        exitFirstCountryId, 'cityListVisible', 'true');

    // select first city in exit country
    const cityThreeId = screenHome.serverListView.generateCityId(
        exitFirstCountryId, cityThree.name);

    await selectCityFromList(cityThreeId, exitFirstCountryId);
    await waitForQueryAndClick(cityThreeId);

    // Back at the main view. select the exit entries
    await waitForQueryAndClick(
        screenHome.serverListView.BACK_BUTTON.visible());

    // define new connected server
    newCurrentCountry = firstServer.localizedName;
    newCurrentCity = cityThree.localizedName;

    // wait and assert server switching for multihop
    await waitForCondition(
        async () => {return lastNotification().title ==
                     'VPN Switched Servers'},
        20)
    strictEqual(
        lastNotification().message,
        `Switched from ${currentCountry}, ${currentCity} to ${
            newCurrentCountry}, ${newCurrentCity}`);
  });

  it.skip(
      'Server switching -- different country different cities', async () => {
        await setSetting('serverSwitchNotification', true);
        await setSetting('connectionChangeNotification', true);

        let newCurrentCountry;
        let newCurrentCity;
        let currentCountry;
        let currentCity;

        // wait for select entry and select entry
        await waitForElementAndClick(
            homeScreen.selectSingleHopServerView.MULTIHOP_SELECTOR_TAB);
        await waitForElementAndClick(
            homeScreen.selectMultiHopServerView.ENTRY_BUTTON);

        // exit server details
        const firstServer = servers[0];
        const cityTwo = firstServer.cities[0];
        const exitFirstCountryId =
            homeScreen.serverListView.generateCountryId(firstServer.code);

        // second exit server details
        const thirdServer = servers[2];
        const cityThree = thirdServer.cities[0];
        const exitThirdCountryId =
            homeScreen.serverListView.generateCountryId(thirdServer.code);

        // entry server details
        const secondServer = servers[1];
        const cityOne = secondServer.cities[0];
        const entryCountryId =
            homeScreen.serverListView.generateCountryId(secondServer.code);

        // select the first country
        await selectCountryFromList(entryCountryId);
        await wait();

        if (await getElementProperty(entryCountryId, 'cityListVisible') ===
            'false') {
          await clickOnQuery(entryCountryId);
        }
        await waitForElementProperty(
            entryCountryId, 'cityListVisible', 'true');

        // select first city
        const cityOneId = homeScreen.serverListView.generateCityId(
            entryCountryId, cityOne.name);
        await selectCityFromList(cityOneId, entryCountryId);
        await waitForElementAndClick(cityOneId);

        // Back at the main view. select the exit entries
        await waitForElementAndClick(
            homeScreen.selectMultiHopServerView.EXIT_BUTTON);

        // select first country again
        await selectCountryFromList(exitFirstCountryId);
        await wait();
        if (await getElementProperty(
                exitFirstCountryId, 'cityListVisible') === 'false') {
          await clickOnQuery(exitFirstCountryId);
        }
        await waitForElementProperty(
            exitFirstCountryId, 'cityListVisible', 'true');

        // select first city in exit country
        const cityTwoId = homeScreen.serverListView.generateCityId(
            exitFirstCountryId, cityTwo.name);
        await selectCityFromList(cityTwoId, exitFirstCountryId);
        await waitForElementAndClick(cityTwoId);

        // navigate back to connection view
        await waitForElementAndClick(
            homeScreen.selectSingleHopServerView.BACK_BUTTON);

        // define connected server
        currentCountry = firstServer.localizedName;
        currentCity = cityTwo.localizedName;

        // connect vpn
        await activate();

        // wait and assert vpn connection
        await waitForCondition(async () => {
          return await getElementProperty(
                     generalElements.CONTROLLER_TITLE, 'text') == 'VPN is on';
        });
        strictEqual(lastNotification().title, 'VPN Connected');
        strictEqual(
            lastNotification().message,
            `Connected to ${currentCountry}, ${currentCity}`);

        // back to main view
        await waitForElementAndClick(homeScreen.SERVER_LIST_BUTTON);

        // Back at the main view. select the exit entries
        await waitForElementAndClick(
            homeScreen.selectMultiHopServerView.EXIT_BUTTON);

        // select first country again
        await selectCountryFromList(exitThirdCountryId);
        await wait();
        if (await getElementProperty(
                exitThirdCountryId, 'cityListVisible') === 'false') {
          await clickOnQuery(exitThirdCountryId);
        }
        await waitForElementProperty(
            exitThirdCountryId, 'cityListVisible', 'true');

        // select first city in exit country
        const cityThreeId = homeScreen.serverListView.generateCityId(
            exitThirdCountryId, cityThree.name);
        await selectCityFromList(cityThreeId, exitThirdCountryId)
            await waitForElementAndClick(cityThreeId);

        // Back at the main view. select the exit entries
        await waitForElementAndClick(
            homeScreen.selectSingleHopServerView.BACK_BUTTON);

        // define new connected server
        newCurrentCountry = thirdServer.localizedName;
        newCurrentCity = cityThree.localizedName;

        // wait and assert server switching for multihop
        await waitForCondition(
            async () => {return lastNotification().title ==
                         'VPN Switched Servers'},
            20)
        strictEqual(
            lastNotification().message,
            `Switched from ${currentCountry}, ${currentCity} to ${
                newCurrentCountry}, ${newCurrentCity}`);
      });

  it.skip('Single and multihop switching', async () => {
    await setSetting('serverSwitchNotification', true);
    await setSetting('connectionChangeNotification', true);

    let currentCountry;
    let currentCity;

    // wait for select entry and select entry
    await waitForQueryAndClick(
        screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
    await waitForQueryAndClick(
        screenHome.serverListView.ENTRY_BUTTON.visible());

    // exit server details
    const firstServer = servers[0];
    const cityTwo = firstServer.cities[0];
    const exitFirstCountryId =
        screenHome.serverListView.generateCountryId(firstServer.code);

    // entry server details
    const secondServer = servers[1];
    const cityOne = secondServer.cities[0];
    const entryCountryId =
        screenHome.serverListView.generateCountryId(secondServer.code);

    // select the first country
    await selectCountryFromList(entryCountryId);
    await wait()
    if (await getQueryProperty(entryCountryId, 'cityListVisible') ===
        'false') {
      await clickOnQuery(entryCountryId);
    }
    await waitForElementProperty(entryCountryId, 'cityListVisible', 'true');
    equal(
        await getQueryProperty(entryCountryId, 'cityListVisible'), 'true');

    // select first city
    const cityOneId = screenHome.serverListView.generateCityId(
        entryCountryId, cityOne.name);
    await selectCityFromList(cityOneId, entryCountryId);
    await waitForQueryAndClick(cityOneId.visible());

    // Back at the main view. select the exit entries
    await waitForQueryAndClick(
        screenHome.serverListView.EXIT_BUTTON.visible());

    // select first country again
    await selectCountryFromList(exitFirstCountryId);
    await wait();
    if (await getQueryProperty(exitFirstCountryId, 'cityListVisible') ===
        'false') {
      await clickOnQuery(exitFirstCountryId);
    }
    await getQueryProperty(exitFirstCountryId, 'cityListVisible', 'true');

    // select first city in exit country
    const cityTwoId = screenHome.serverListView.generateCityId(
        exitFirstCountryId, cityTwo.name);
    await selectCityFromList(cityTwoId, exitFirstCountryId)
        await waitForQueryAndClick(cityTwoId);

    // navigate back to connection view
    await waitForQueryAndClick(
        screenHome.serverListView.BACK_BUTTON.visible());

    // define connected server
    currentCountry = firstServer.localizedName;
    currentCity = cityTwo.localizedName;

    // connect vpn
    await activate();

    // wait and assert vpn connection
    await waitForCondition(async () => {
      return await getQueryProperty(
                 screenHome.CONTROLLER_TITLE, 'text') == 'VPN is on';
    });
    strictEqual(lastNotification().title, 'VPN Connected');
    strictEqual(
        lastNotification().message,
        `Connected to ${currentCountry}, ${currentCity}`);

    // back to main view
    await waitForQueryAndClick(homeScreen.SERVER_LIST_BUTTON);


    // switch from multihop to singlehop
    await waitForQueryAndClick(
            homeScreen.selectSingleHopServerView.SINGLEHOP_SELECTOR_TAB)
            await waitForQueryAndClick(homeScreen.selectSingleHopServerView.BACK_BUTTON)

        // wait and assert vpn connection
        await waitForCondition(async () => {
          return await getElementProperty(
                     generalElements.CONTROLLER_TITLE, 'text') == 'VPN is on';
        });
    strictEqual(lastNotification().title, 'VPN Connected');
    strictEqual(
        lastNotification().message,
        `Connected to ${currentCountry}, ${currentCity}`);
  });
});
