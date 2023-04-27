/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');

async function selectCityFromList(cityId, countryId) {
  await vpn.setQueryProperty(
      queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
      parseInt(await vpn.getQueryProperty(cityId, 'y')) +
          parseInt(await vpn.getQueryProperty(countryId, 'y')));
}

async function selectCountryFromList(countryId) {
  await vpn.setQueryProperty(
      queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
      parseInt(await vpn.getQueryProperty(countryId, 'y')));
}

describe('Server list', function() {
  let servers;
  let currentCountryCode;
  let currentCity;

  this.timeout(240000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await vpn.waitForQueryAndClick(queries.screenHome.SERVER_LIST_BUTTON);
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

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

  it('opening the entry and exit server list', async () => {
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
    await vpn.waitForQuery(
        queries.screenHome.serverListView.VPN_COLLAPSIBLE_CARD.visible());

    assert(
        await vpn.getQueryProperty(
            queries.screenHome.serverListView.VPN_COLLAPSIBLE_CARD,
            'expanded') === 'false');

    await vpn.waitForQuery(
        queries.screenHome.serverListView.ENTRY_BUTTON.visible());

    await vpn.waitForQuery(
        queries.screenHome.serverListView.EXIT_BUTTON.visible());

    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.VPN_MULTHOP_CHEVRON.visible())
    assert(await vpn.getQueryProperty(
        queries.screenHome.serverListView.VPN_COLLAPSIBLE_CARD, 'expanded'))
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

      if (await vpn.getQueryProperty(countryId, 'cityListVisible') ===
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
  })


  it('check the countries and cities for multihop exits', async () => {
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.EXIT_BUTTON.visible());

    for (let server of servers) {
      const countryId =
          queries.screenHome.serverListView.generateCountryId(server.code);

      await vpn.waitForQuery(countryId.visible());

      await selectCountryFromList(countryId);
      await vpn.wait();

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
        const cityId = countryId + '/serverCityList/serverCity-' +
            city.name.replace(/ /g, '_');

        await vpn.waitForQuery(cityId);
        await vpn.getQueryProperty(cityId, 'visible', 'true');
        await vpn.getQueryProperty(
            cityId, 'checked',
            currentCountryCode === server.code &&
                    currentCity === city.localizedName ?
                'true' :
                'false');
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
      await selectCountryFromList(countryId);
      await vpn.wait();

      if (await vpn.getQueryProperty(countryId, 'cityListVisible') ===
          'false') {
        await vpn.clickOnQuery(countryId);
      }

      for (let city of server.cities) {
        const cityId = countryId + '/serverCityList/serverCity-' +
            city.name.replace(/ /g, '_');

        await vpn.waitForQuery(cityId);
        await selectCityFromList(cityId, countryId);
        await vpn.getQueryProperty(cityId, 'visible', 'true');

        const cityName = await vpn.getQueryProperty(
            cityId, 'radioButtonLabelText'.split(' '));

        await vpn.wait();
        await vpn.clickOnQuery(cityId);
        await vpn.wait();

        assert(cityName.includes(city.name))
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

      await selectCountryFromList(countryId);
      await vpn.wait();

      if (await vpn.getQueryProperty(countryId, 'cityListVisible') ===
          'false') {
        await vpn.clickOnQuery(countryId);
      }

      for (let city of server.cities) {
        const cityId = countryId + '/serverCityList/serverCity-' +
            city.name.replace(/ /g, '_');
        await vpn.waitForQuery(cityId);

        await selectCityFromList(cityId, countryId)
            await vpn.getQueryProperty(cityId, 'visible', 'true');

        const cityName = await vpn.getQueryProperty(
            cityId, 'radioButtonLabelText'.split(' '));

        await vpn.wait();
        await vpn.clickOnQuery(cityId);
        await vpn.wait();

        // One selected
        await vpn.waitForQuery(cityId);
        await vpn.getQueryProperty(cityId, 'checked', 'true');
        assert(cityName.includes(city.name))
      }
    }
  });

  it.skip('Server switching -- same country different cities', async () => {
    await vpn.setSetting('serverSwitchNotification', true);
    await vpn.setSetting('connectionChangeNotification', true);

    let newCurrentCountry;
    let newCurrentCity;
    let currentCountry;
    let currentCity;

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
    await selectCountryFromList(entryCountryId);
    await vpn.wait()
    if (await vpn.getQueryProperty(entryCountryId, 'cityListVisible') ===
        'false') {
      await vpn.clickOnQuery(entryCountryId);
    }
    await vpn.getQueryProperty(entryCountryId, 'cityListVisible', 'true');

    // select first city
    const cityOneId = queries.screenHome.serverListView.generateCityId(
        entryCountryId, cityOne.name);

    await selectCityFromList(cityOneId, entryCountryId);
    await vpn.wait();
    await vpn.waitForQueryAndClick(cityOneId);

    // // Back at the main view. select the exit entries
    // await vpn.waitForQueryAndClick(
    //     queries.screenHome.serverListView.EXIT_BUTTON.visible());

    // select first country again
    await selectCountryFromList(exitFirstCountryId);
    await vpn.wait();
    if (await vpn.getQueryProperty(exitFirstCountryId, 'cityListVisible') ===
        'false') {
      await vpn.clickOnQuery(exitFirstCountryId);
    }
    await vpn.getQueryProperty(exitFirstCountryId, 'cityListVisible', 'true');

    // select first city in exit country
    const cityTwoId = queries.screenHome.serverListView.generateCityId(
        exitFirstCountryId, cityTwo.name);

    await selectCityFromList(cityTwoId, exitFirstCountryId);
    await vpn.wait();
    await vpn.waitForQueryAndClick(cityTwoId);

    // navigate back to connection view
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.BACK_BUTTON.visible());

    // define connected server
    currentCountry = firstServer.localizedName;
    currentCity = cityTwo.localizedName;

    // connect vpn
    await vpn.activate();

    // wait and assert vpn connection
    await vpn.waitForCondition(async () => {
      return await vpn.getQueryProperty(
                 queries.screenHome.CONTROLLER_TITLE, 'text') == 'VPN is on';
    });
    assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');
    // assert.strictEqual(
    //     vpn.lastNotification().message,
    //     `Connected through ${currentCity}`);

    // // back to main view
    // await vpn.waitForQueryAndClick(queries.screenHome.SERVER_LIST_BUTTON);

    // // Back at the main view. select the exit entries
    // await vpn.waitForQueryAndClick(
    //     queries.screenHome.serverListView.EXIT_BUTTON.visible());

    // select first country again
    await selectCountryFromList(exitFirstCountryId);
    await vpn.wait();
    if (await vpn.getElementProperty(exitFirstCountryId, 'cityListVisible') ===
        'false') {
      await vpn.clickOnElement(exitFirstCountryId);
    }
    await vpn.waitForElementProperty(
        exitFirstCountryId, 'cityListVisible', 'true');

    // // select first city in exit country
    // const cityThreeId = queries.screenHome.serverListView.generateCityId(
    //     exitFirstCountryId, cityThree.name);

    // await selectCityFromList(cityThreeId, exitFirstCountryId);
    // await vpn.waitForQueryAndClick(cityThreeId);

    // // Back at the main view. select the exit entries
    // await vpn.waitForQueryAndClick(
    //     queries.screenHome.serverListView.BACK_BUTTON.visible());

    // // define new connected server
    // newCurrentCountry = firstServer.localizedName;
    // newCurrentCity = cityThree.localizedName;

    // // wait and assert server switching for multihop
    // await vpn.waitForCondition(
    //     async () => {return vpn.lastNotification().title ==
    //                  'VPN Switched Servers'},
    //     20)
    // assert.strictEqual(
    //     vpn.lastNotification().message,
    //     `Switched from ${currentCountry}, ${currentCity} to ${
    //         newCurrentCountry}, ${newCurrentCity}`);
  });

  it.skip(
      'Server switching -- different country different cities', async () => {
        await vpn.setSetting('serverSwitchNotification', true);
        await vpn.setSetting('connectionChangeNotification', true);

        let newCurrentCountry;
        let newCurrentCity;
        let currentCountry;
        let currentCity;

        // wait for select entry and select entry
        await vpn.waitForElementAndClick(
            homeScreen.selectSingleHopServerView.MULTIHOP_SELECTOR_TAB);
        await vpn.waitForElementAndClick(
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
        await vpn.wait();

        if (await vpn.getElementProperty(entryCountryId, 'cityListVisible') ===
            'false') {
          await vpn.clickOnElement(entryCountryId);
        }
        await vpn.waitForElementProperty(
            entryCountryId, 'cityListVisible', 'true');

        // select first city
        const cityOneId = homeScreen.serverListView.generateCityId(
            entryCountryId, cityOne.name);
        await selectCityFromList(cityOneId, entryCountryId);
        await vpn.waitForElementAndClick(cityOneId);

        // Back at the main view. select the exit entries
        await vpn.waitForElementAndClick(
            homeScreen.selectMultiHopServerView.EXIT_BUTTON);

        // select first country again
        await selectCountryFromList(exitFirstCountryId);
        await vpn.wait();
        if (await vpn.getElementProperty(
                exitFirstCountryId, 'cityListVisible') === 'false') {
          await vpn.clickOnElement(exitFirstCountryId);
        }
        await vpn.waitForElementProperty(
            exitFirstCountryId, 'cityListVisible', 'true');

        // select first city in exit country
        const cityTwoId = homeScreen.serverListView.generateCityId(
            exitFirstCountryId, cityTwo.name);
        await selectCityFromList(cityTwoId, exitFirstCountryId);
        await vpn.waitForElementAndClick(cityTwoId);

        // navigate back to connection view
        await vpn.waitForElementAndClick(
            homeScreen.selectSingleHopServerView.BACK_BUTTON);

        // define connected server
        currentCountry = firstServer.localizedName;
        currentCity = cityTwo.localizedName;

        // connect vpn
        await vpn.activate();

        // wait and assert vpn connection
        await vpn.waitForCondition(async () => {
          return await vpn.getElementProperty(
                     generalElements.CONTROLLER_TITLE, 'text') == 'VPN is on';
        });
        assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');
        assert.strictEqual(
            vpn.lastNotification().message,
            `Connected to ${currentCountry}, ${currentCity}`);

        // back to main view
        await vpn.waitForElementAndClick(homeScreen.SERVER_LIST_BUTTON);

        // Back at the main view. select the exit entries
        await vpn.waitForElementAndClick(
            homeScreen.selectMultiHopServerView.EXIT_BUTTON);

        // select first country again
        await selectCountryFromList(exitThirdCountryId);
        await vpn.wait();
        if (await vpn.getElementProperty(
                exitThirdCountryId, 'cityListVisible') === 'false') {
          await vpn.clickOnElement(exitThirdCountryId);
        }
        await vpn.waitForElementProperty(
            exitThirdCountryId, 'cityListVisible', 'true');

        // select first city in exit country
        const cityThreeId = homeScreen.serverListView.generateCityId(
            exitThirdCountryId, cityThree.name);
        await selectCityFromList(cityThreeId, exitThirdCountryId)
            await vpn.waitForElementAndClick(cityThreeId);

        // Back at the main view. select the exit entries
        await vpn.waitForElementAndClick(
            homeScreen.selectSingleHopServerView.BACK_BUTTON);

        // define new connected server
        newCurrentCountry = thirdServer.localizedName;
        newCurrentCity = cityThree.localizedName;

        // wait and assert server switching for multihop
        await vpn.waitForCondition(
            async () => {return vpn.lastNotification().title ==
                         'VPN Switched Servers'},
            20)
        assert.strictEqual(
            vpn.lastNotification().message,
            `Switched from ${currentCountry}, ${currentCity} to ${
                newCurrentCountry}, ${newCurrentCity}`);
      });

  it.skip('Single and multihop switching', async () => {
    await vpn.setSetting('serverSwitchNotification', true);
    await vpn.setSetting('connectionChangeNotification', true);

    let currentCountry;
    let currentCity;

    // wait for select entry and select entry
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.ENTRY_BUTTON.visible());

    // exit server details
    const firstServer = servers[0];
    const cityTwo = firstServer.cities[0];
    const exitFirstCountryId =
        queries.screenHome.serverListView.generateCountryId(firstServer.code);

    // entry server details
    const secondServer = servers[1];
    const cityOne = secondServer.cities[0];
    const entryCountryId =
        queries.screenHome.serverListView.generateCountryId(secondServer.code);

    // select the first country
    await selectCountryFromList(entryCountryId);
    await vpn.wait()
    if (await vpn.getQueryProperty(entryCountryId, 'cityListVisible') ===
        'false') {
      await vpn.clickOnQuery(entryCountryId);
    }
    // await vpn.waitForElementProperty(entryCountryId, 'cityListVisible',
    // 'true');
    assert.equal(
        await vpn.getQueryProperty(entryCountryId, 'cityListVisible'), 'true');

    // select first city
    const cityOneId = queries.screenHome.serverListView.generateCityId(
        entryCountryId, cityOne.name);
    await selectCityFromList(cityOneId, entryCountryId);
    await vpn.waitForQueryAndClick(cityOneId.visible());

    // Back at the main view. select the exit entries
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.EXIT_BUTTON.visible());

    // select first country again
    await selectCountryFromList(exitFirstCountryId);
    await vpn.wait();
    if (await vpn.getQueryProperty(exitFirstCountryId, 'cityListVisible') ===
        'false') {
      await vpn.clickOnQuery(exitFirstCountryId);
    }
    await vpn.getQueryProperty(exitFirstCountryId, 'cityListVisible', 'true');

    // select first city in exit country
    const cityTwoId = queries.screenHome.serverListView.generateCityId(
        exitFirstCountryId, cityTwo.name);
    await selectCityFromList(cityTwoId, exitFirstCountryId)
        await vpn.waitForQueryAndClick(cityTwoId);

    // navigate back to connection view
    await vpn.waitForQueryAndClick(
        queries.screenHome.serverListView.BACK_BUTTON.visible());

    // define connected server
    currentCountry = firstServer.localizedName;
    currentCity = cityTwo.localizedName;

    // connect vpn
    await vpn.activate();

    // wait and assert vpn connection
    await vpn.waitForCondition(async () => {
      return await vpn.getQueryProperty(
                 queries.screenHome.CONTROLLER_TITLE, 'text') == 'VPN is on';
    });
    assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');
    assert.strictEqual(
        vpn.lastNotification().message,
        `Connected to ${currentCountry}, ${currentCity}`);

    // back to main view
    // await vpn.waitForQueryAndClick(homeScreen.SERVER_LIST_BUTTON);


    // switch from multihop to singlehop
    // await vpn
    //     .waitForQueryAndClick(
    //         homeScreen.selectSingleHopServerView.SINGLEHOP_SELECTOR_TAB)
    //         await vpn
    //     .waitForQueryAndClick(
    //         homeScreen.selectSingleHopServerView.BACK_BUTTON)

    //     // wait and assert vpn connection
    //     await vpn.waitForCondition(
    //         async () => {return await vpn.getElementProperty(
    //                                 generalElements.CONTROLLER_TITLE,
    //                                 'text') == 'VPN is on'; });
    // assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');
    // assert.strictEqual(
    //     vpn.lastNotification().message,
    //     `Connected to ${currentCountry}, ${currentCity}`);
  });
});
