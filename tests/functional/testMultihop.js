/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');
const { getQueryProperty, waitForQuery } = require('./helper.js');

describe('Server list', function() {
  let servers;
  let currentCountryCode;
  let currentCity;

  this.timeout(240000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await vpn.waitForQueryAndClick(queries.screenHome.SERVER_LIST_BUTTON.visible());
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


    it('verify what is multi-hop vpn collapsible', async () => {
        await vpn.waitForQueryAndClick(queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
        assert.equal(
            await vpn.getQueryProperty(
                queries.screenHome.serverListView.VPN_COLLAPSIBLE_CARD,
                'expanded'), 'false');

        await vpn.waitForQuery(queries.screenHome.serverListView.ENTRY_SERVER_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.serverListView.EXIT_SERVER_BUTTON.visible());

        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.VPN_MULTHOP_CHEVRON)
        assert.equal(await vpn.getQueryProperty(queries.screenHome.serverListView.VPN_COLLAPSIBLE_CARD, 'expanded'), 'true')
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

            if (currentCountryCode === server.code) {
              assert.equal(
                  await vpn.getQueryProperty(countryId, 'cityListVisible'),
                  'true');
            }
            await vpn.wait();


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
                console.log('  Waiting for cityId:', cityId);
                await vpn.waitForQuery(cityId.visible().prop(
                    'checked',
                    currentCountryCode === server.code &&
                        currentCity === city.localizedName));
            }
        }
    })


    it('check the countries and cities for multihop exits', async () => {
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.EXIT_SERVER_BUTTON.visible());

        for (let server of servers) {
            const countryId =
                queries.screenHome.serverListView.generateCountryId(server.code);
            await vpn.waitForQuery(countryId.visible());

            await vpn.setQueryProperty(
                queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
                parseInt(await vpn.getQueryProperty(countryId, 'y')));

             if (currentCountryCode === server.code) {
                assert(
                    await vpn.getQueryProperty(countryId, 'cityListVisible') ===
                    'true');
            }

            if (await vpn.getQueryProperty(countryId, 'cityListVisible') ===
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
    })


    it('Pick cities for entries', async () => {
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.ENTRY_SERVER_BUTTON.visible());

        for (let server of servers) {
            const countryId = queries.screenHome.serverListView.generateCountryId(server.code);

            await vpn.waitForQuery(countryId.visible());
            await vpn.scrollToQuery(
                queries.screenHome.serverListView.COUNTRY_VIEW, countryId);

            if (await vpn.getQueryProperty(countryId, 'cityListVisible') === 'false') {
                await vpn.waitForQueryAndClick(countryId.visible());
            }

            await vpn.waitForQuery(countryId.visible().prop('cityListVisible', true));

            for (let city of server.cities) {
                console.log('  Start test for city:', city);
                const cityId = queries.screenHome.serverListView.generateCityId(
                    countryId, city.name);
                await vpn.waitForQuery(cityId.visible());

                await vpn.setQueryProperty(
                    queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
                    parseInt(await vpn.getQueryProperty(cityId, 'y')) +
                        parseInt(await vpn.getQueryProperty(countryId, 'y')));
                await vpn.waitForQuery(cityId.visible());

                await vpn.getQueryProperty(cityId, 'radioButtonLabelText');

                await vpn.waitForQueryAndClick(cityId.visible());
                await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());
                // wait is for animation, as ENTRY and EXIT servers buttons are not interactible during this animation
                await vpn.wait()

                currentCountryCode = server.code;
                currentCity = city.localizedName;

                // Back to the main view.
                await vpn.waitForQueryAndClick(queries.screenHome.serverListView.ENTRY_SERVER_BUTTON.visible())
                await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

                // One selected
                await vpn.waitForQuery(cityId.checked());
            }
        }
    });

    it('Pick cities for exits', async () => {
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.EXIT_SERVER_BUTTON.visible());

        for (let server of servers) {
            const countryId = queries.screenHome.serverListView.generateCountryId(server.code);

            await vpn.waitForQuery(countryId.visible());
            await vpn.scrollToQuery(
                queries.screenHome.serverListView.COUNTRY_VIEW, countryId);

            if (await vpn.getQueryProperty(countryId, 'cityListVisible') === 'false') {
                await vpn.waitForQueryAndClick(countryId.visible());
            }

            await vpn.waitForQuery(countryId.visible().prop('cityListVisible', true));

            for (let city of server.cities) {
                console.log('  Start test for city:', city);
                const cityId = queries.screenHome.serverListView.generateCityId(
                    countryId, city.name);
                await vpn.waitForQuery(cityId.visible());

                await vpn.setQueryProperty(
                    queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
                    parseInt(await vpn.getQueryProperty(cityId, 'y')) +
                        parseInt(await vpn.getQueryProperty(countryId, 'y')));
                await vpn.waitForQuery(cityId.visible());

                await vpn.waitForQueryAndClick(cityId.visible());
                await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());
                // wait is for animation
                await vpn.wait();

                currentCountryCode = server.code;
                currentCity = city.localizedName;

                // Back to the main view.
                await vpn.waitForQueryAndClick(queries.screenHome.serverListView.EXIT_SERVER_BUTTON.visible())
                await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

                // One selected
                await vpn.waitForQuery(cityId.checked());
            }
        }
    });


    it('Server switching -- same country different cities', async () => {
        await vpn.setSetting('serverSwitchNotification', true);
        await vpn.setSetting('connectionChangeNotification', true);

        let newCurrentCity;
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
        await vpn.waitForQuery(entryCountryId.visible())

        await vpn.scrollToQuery(
            queries.screenHome.serverListView.COUNTRY_VIEW, entryCountryId);
        await vpn.clickOnQuery(entryCountryId);

        // select first city
        const cityOneId =
        queries.screenHome.serverListView.generateCityId(entryCountryId,
            cityOne.name);

        await vpn.setQueryProperty(
            queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
            parseInt(await vpn.getQueryProperty(cityOneId, 'y')) +
                parseInt(await vpn.getQueryProperty(entryCountryId, 'y')));

        await vpn.waitForQueryAndClick(cityOneId.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // Back at the main view. select the exit entries
        // wait is for animation
        await vpn.wait()
        await vpn.waitForQueryAndClick(queries.screenHome.serverListView.EXIT_SERVER_BUTTON.visible());

        // select exit country again
        await vpn.waitForQuery(exitFirstCountryId.visible())

        await vpn.scrollToQuery(
            queries.screenHome.serverListView.COUNTRY_VIEW, entryCountryId);
        await vpn.waitForQueryAndClick(entryCountryId.visible());

        // select first city in exit country
        const cityTwoId = queries.screenHome.serverListView.generateCityId(
            exitFirstCountryId, cityTwo.name);
        await vpn.setQueryProperty(
            queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
            parseInt(await vpn.getQueryProperty(cityTwoId, 'y')) +
                parseInt(await vpn.getQueryProperty(exitFirstCountryId, 'y')));
        await vpn.wait();

        await vpn.waitForQueryAndClick(cityTwoId.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // // navigate back to connection view
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.BACK_BUTTON.visible());

        // define connected server
        currentCity = cityTwo.localizedName;

        // connect vpn
        await vpn.activate();

        // wait and assert vpn connection
        await vpn.waitForCondition(async () => {
            return await vpn.getQueryProperty(queries.screenHome.CONTROLLER_TITLE, 'text') == 'VPN is on';
        });
        assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');
        assert.strictEqual(vpn.lastNotification().message, `Connected to ${currentCity}`);

        // back to main view
        await vpn.waitForQueryAndClick(queries.screenHome.SERVER_LIST_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // Back at the main view. select the exit entries
        await vpn.waitForQueryAndClick(queries.screenHome.serverListView.EXIT_SERVER_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // select first country again
        await vpn.waitForQuery(exitFirstCountryId.visible())

        await vpn.scrollToQuery(
            queries.screenHome.serverListView.COUNTRY_VIEW, entryCountryId);
        await vpn.waitForQueryAndClick(entryCountryId.visible());

        // select first city in exit country
        const cityThreeId = queries.screenHome.serverListView.generateCityId(
            exitFirstCountryId, cityThree.name);
        await vpn.setQueryProperty(
            queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
            parseInt(await vpn.getQueryProperty(cityThreeId, 'y')) +
                parseInt(await vpn.getQueryProperty(exitFirstCountryId, 'y')));
        await vpn.wait();

        await vpn.waitForQueryAndClick(cityThreeId.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // Back at the main view. select the exit entries
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.BACK_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // define new connected server
        newCurrentCity = cityThree.localizedName;

        // wait and assert server switching for multihop
        await vpn.waitForCondition(
            async () => {return vpn.lastNotification().title ==
                        'VPN Switched Servers'},
            20)
        assert.strictEqual(
            vpn.lastNotification().message,
            `Switched from ${currentCity} to ${newCurrentCity}`);

        await vpn.deactivate()
    });


    it('Server switching -- different country different cities', async () => {
        await vpn.setSetting('serverSwitchNotification', true);
        await vpn.setSetting('connectionChangeNotification', true);

        let newCurrentCity;
        let currentCity;

        // wait for select entry and select entry
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible());
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.ENTRY_BUTTON.visible());

        // exit server
        const firstServer = servers[0];
        const cityTwo = firstServer.cities[0];
        const exitFirstCountryId =
            queries.screenHome.serverListView.generateCountryId(firstServer.code);

        // second exit server
        const thirdServer = servers[2];
        const cityThree = thirdServer.cities[0];
        const exitThirdCountryId =
            queries.screenHome.serverListView.generateCountryId(thirdServer.code);

        // entry server
        const secondServer = servers[1];
        const cityOne = secondServer.cities[0];
        const entryCountryId =
            queries.screenHome.serverListView.generateCountryId(secondServer.code);

        // select the first country
        await vpn.waitForQuery(entryCountryId.visible())
        await vpn.scrollToQuery(
            queries.screenHome.serverListView.COUNTRY_VIEW, entryCountryId);
        await vpn.clickOnQuery(entryCountryId);

        // select first city
        const cityOneId =
            queries.screenHome.serverListView.generateCityId(entryCountryId,
        cityOne.name);
        await vpn.setQueryProperty(
            queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
            parseInt(await vpn.getQueryProperty(cityOneId, 'y')) +
                parseInt(await vpn.getQueryProperty(entryCountryId, 'y')));

        await vpn.waitForQueryAndClick(cityOneId.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // Back at the main view. select the exit entries
        // wait is for animation
        await vpn.wait()
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.EXIT_SERVER_BUTTON.visible());

        // select exit country
        await vpn.waitForQuery(exitFirstCountryId.visible())
        await vpn.scrollToQuery(
            queries.screenHome.serverListView.COUNTRY_VIEW, exitFirstCountryId);
        await vpn.waitForQueryAndClick(exitFirstCountryId.visible());

        // wait is for animation
        await vpn.wait();
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.EXIT_SERVER_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // select first city in exit country
        const cityTwoId = queries.screenHome.serverListView.generateCityId(
            exitFirstCountryId, cityTwo.name);

        await vpn.setQueryProperty(
            queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
            parseInt(await vpn.getQueryProperty(cityTwoId, 'y')) +
                parseInt(await vpn.getQueryProperty(exitFirstCountryId, 'y')));

        await vpn.waitForQueryAndClick(cityTwoId.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // navigate back to connection view
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.BACK_BUTTON.visible());

        // define connected server
        currentCity = cityTwo.localizedName;

        // connect vpn
        await vpn.activate();

        // wait and assert vpn connection
        await vpn.waitForCondition(async () => {
            return await vpn.getQueryProperty(queries.screenHome.CONTROLLER_TITLE, 'text') == 'VPN is on';
        });
        assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');
        assert.strictEqual(
            vpn.lastNotification().message,
            `Connected to ${currentCity}`);

        // back to main view
        await vpn.waitForQueryAndClick(queries.screenHome.SERVER_LIST_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // select the exit servers
        // wait is for animation
        await vpn.wait();
        await vpn.waitForQueryAndClick(queries.screenHome.serverListView.EXIT_SERVER_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // select first country again
        await vpn.waitForQuery(exitThirdCountryId.visible())

        await vpn.scrollToQuery(
            queries.screenHome.serverListView.COUNTRY_VIEW, exitThirdCountryId);
        await vpn.waitForQueryAndClick(exitThirdCountryId.visible());

        // select first city in 3rd exit country
        const cityThreeId = queries.screenHome.serverListView.generateCityId(
            exitThirdCountryId, cityThree.name);
        await vpn.setQueryProperty(
            queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
            parseInt(await vpn.getQueryProperty(cityThreeId, 'y')) +
                parseInt(await vpn.getQueryProperty(exitThirdCountryId, 'y')));

        await vpn.waitForQueryAndClick(cityThreeId.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // Back at the main view. select the exit entries
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.BACK_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // define new connected server
        newCurrentCity = cityThree.localizedName;

        // wait and assert server switching for multihop
        await vpn.waitForCondition(
            async () => {return vpn.lastNotification().title ==
                        'VPN Switched Servers'},
            20)
        assert.strictEqual(
            vpn.lastNotification().message,
            `Switched from ${currentCity} to ${newCurrentCity}`);

        await vpn.deactivate()
    });


    it('Single and multihop switching', async () => {
        await vpn.setSetting('serverSwitchNotification', true);
        await vpn.setSetting('connectionChangeNotification', true);

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
        await vpn.waitForQuery(entryCountryId.visible())
        await vpn.scrollToQuery(
            queries.screenHome.serverListView.COUNTRY_VIEW, entryCountryId);
        await vpn.clickOnQuery(entryCountryId);

        // select first city
        const cityOneId =
            queries.screenHome.serverListView.generateCityId(entryCountryId,
        cityOne.name);
        await vpn.setQueryProperty(
            queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
            parseInt(await vpn.getQueryProperty(cityOneId, 'y')) +
                parseInt(await vpn.getQueryProperty(entryCountryId, 'y')));

        await vpn.waitForQueryAndClick(cityOneId.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // wait is for animation
        await vpn.wait();
        await vpn.waitForQuery(queries.screenHome.serverListView.EXIT_SERVER_BUTTON.ready());
        await vpn.waitForQueryAndClick(queries.screenHome.serverListView.EXIT_SERVER_BUTTON.visible());

        // select exit country again
        await vpn.waitForQuery(exitFirstCountryId.visible())
        await vpn.scrollToQuery(
            queries.screenHome.serverListView.COUNTRY_VIEW, exitFirstCountryId);
        await vpn.waitForQueryAndClick(exitFirstCountryId.visible());

        // select first city in exit country
        const cityTwoId = queries.screenHome.serverListView.generateCityId(
            exitFirstCountryId, cityTwo.name);
        await vpn.setQueryProperty(
            queries.screenHome.serverListView.COUNTRY_VIEW, 'contentY',
            parseInt(await vpn.getQueryProperty(cityTwoId, 'y')) +
                parseInt(await vpn.getQueryProperty(exitFirstCountryId, 'y')));
        await vpn.wait();

        // navigate back to connection view
        await vpn.waitForQueryAndClick(
            queries.screenHome.serverListView.BACK_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // define connected server
        currentCity = cityTwo.localizedName;

        // connect vpn
        await vpn.activate();

        // wait and assert vpn connection
        await vpn.waitForCondition(async () => {
            return await vpn.getQueryProperty(queries.screenHome.CONTROLLER_TITLE, 'text') == 'VPN is on';
        });
        assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');
        assert.strictEqual(
            vpn.lastNotification().message,
            `Connected to ${currentCity}`);

        // back to main view
        await vpn.waitForQueryAndClick(queries.screenHome.SERVER_LIST_BUTTON.visible());
        await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

        // switch from multihop to singlehop
        await vpn.waitForQueryAndClick(queries.screenHome.serverListView.SINGLEHOP_SELECTOR_TAB.visible())
        await vpn.waitForQueryAndClick(queries.screenHome.serverListView.BACK_BUTTON)

        // wait and assert vpn connection
        await vpn.waitForCondition(async () => {
            return await vpn.getQueryProperty(queries.screenHome.CONTROLLER_TITLE, 'text') == 'VPN is on';
        });

        assert.strictEqual(vpn.lastNotification().title, 'VPN Connected');
        assert.strictEqual(
            vpn.lastNotification().message,
            `Connected to ${currentCity}`);

        await vpn.deactivate()
    });

});
