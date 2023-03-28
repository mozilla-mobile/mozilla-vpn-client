/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require("assert");
const queries = require("./queries.js");
const vpn = require("./helper.js");

describe("Server list", function () {
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
      "Current city (localized):",
      currentCity,
      "| Current country code:",
      currentCountryCode
    );
  });

  it("multihop valid search", async () => {
    await vpn.waitForQueryAndClick(
      queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible()
    );
    await vpn.waitForQuery(
      queries.screenHome.serverListView.ENTRY_BUTTON.visible()
    );
    await vpn.waitForQueryAndClick(
      queries.screenHome.serverListView.EXIT_BUTTON.visible()
    );

    const server = servers[servers.length - 1];
    console.log("server name: ", server.name);
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenHome.serverListView.SEARCH_BAR_TEXT_FIELD.visible(),
      server.name
    );

    const countryId = queries.screenHome.serverListView.generateCountryId(
      server.code
    );
    await vpn.waitForQuery(countryId.visible());
    await vpn.scrollToQuery(
      queries.screenHome.serverListView.COUNTRY_VIEW,
      countryId
    );

    if (
      (await vpn.getQueryProperty(countryId, "cityListVisible")) === "false"
    ) {
      await vpn.waitForQueryAndClick(countryId.visible());
    }

    await vpn.waitForQuery(countryId.visible().prop("cityListVisible", true));

    const city = server.cities[0]
    console.log("Start test for city:", city);
    const cityId = queries.screenHome.serverListView.generateCityId(
      countryId,
      city.name
    );
    await vpn.waitForQuery(cityId.visible());

    await vpn.setQueryProperty(
      queries.screenHome.serverListView.COUNTRY_VIEW,
      "contentY",
      parseInt(await vpn.getQueryProperty(cityId, "y")) +
        parseInt(await vpn.getQueryProperty(countryId, "y"))
    );
    await vpn.waitForQuery(cityId.visible());

    await vpn.waitForQueryAndClick(cityId.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    // navigate back to connection view
    await vpn.waitForQueryAndClick(
      queries.screenHome.serverListView.BACK_BUTTON.visible()
    );

    // define connected server
    currentCity = city.localizedName;

    // connect vpn
    await vpn.activate();

    // wait and assert vpn connection
    await vpn.waitForCondition(async () => {
      return (
        (await vpn.getQueryProperty(
          queries.screenHome.CONTROLLER_TITLE,
          "text"
        )) == "VPN is on"
      );
    });
    assert.strictEqual(vpn.lastNotification().title, "VPN Connected");
    assert.strictEqual(
      vpn.lastNotification().message,
      `Connected through ${currentCity} via ${currentCity}`
    );
  });

  it("singlehop valid search", async () => {
    const server = servers[0];
    console.log("server name: ", server.name);
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenHome.serverListView.SEARCH_BAR_TEXT_FIELD.visible(),
      server.name
    );

    const countryId = queries.screenHome.serverListView.generateCountryId(
      server.code
    );
    await vpn.waitForQuery(countryId.visible());
    await vpn.scrollToQuery(
      queries.screenHome.serverListView.COUNTRY_VIEW,
      countryId
    );

    if (
      (await vpn.getQueryProperty(countryId, "cityListVisible")) === "false"
    ) {
      await vpn.waitForQueryAndClick(countryId.visible());
    }
    await vpn.waitForQuery(countryId.visible().prop("cityListVisible", true));

    console.log("Start test for city:", server.cities[0]);
    const cityId = queries.screenHome.serverListView.generateCityId(
      countryId,
      server.cities[0].name
    );
    await vpn.waitForQuery(cityId.visible());

    await vpn.setQueryProperty(
      queries.screenHome.serverListView.COUNTRY_VIEW,
      "contentY",
      parseInt(await vpn.getQueryProperty(cityId, "y")) +
        parseInt(await vpn.getQueryProperty(countryId, "y"))
    );
    await vpn.waitForQuery(cityId.visible());

    await vpn.waitForQueryAndClick(cityId.visible());
    await vpn.waitForQuery(queries.screenHome.STACKVIEW.ready());

    // currentCountryCode = server.code;
    currentCity = server.cities[0].localizedName;

    await vpn.activate();

    await vpn.waitForCondition(async () => {
      return (
        (await vpn.getQueryProperty(
          queries.screenHome.CONTROLLER_TITLE,
          "text"
        )) == "VPN is on"
      );
    });
    assert.strictEqual(vpn.lastNotification().title, "VPN Connected");
    assert.strictEqual(
      vpn.lastNotification().message,
      `Connected through ${currentCity}`
    );
  });

  it('Invalid searchs and multi results for singlehop', async () => {
    const australia = queries.screenHome.serverListView.generateCountryId("au");
    const austria = queries.screenHome.serverListView.generateCountryId("at");
    const belgium = queries.screenHome.serverListView.generateCountryId("be");

    // No result search
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenHome.serverListView.SEARCH_BAR_TEXT_FIELD.visible(),
      "invalid search"
    );
    await vpn.waitForQuery(queries.screenHome.serverListView.SEARCH_BAR_ERROR.visible())
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenHome.serverListView.SEARCH_BAR_TEXT_FIELD.visible(),
      ""
    );

    // multi result search
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenHome.serverListView.SEARCH_BAR_TEXT_FIELD.visible(),
      "Au"
    );

    await vpn.waitForQuery(australia.visible())
    await vpn.waitForQuery(austria.visible())
    await vpn.query(belgium.hidden())
  })

  it('Invalid searchs and multi results for multihop', async () => {
    const australia = queries.screenHome.serverListView.generateCountryId("au");
    const austria = queries.screenHome.serverListView.generateCountryId("at");
    const belgium = queries.screenHome.serverListView.generateCountryId("be");

    await vpn.waitForQueryAndClick(
      queries.screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible()
    );
    await vpn.waitForQuery(
      queries.screenHome.serverListView.ENTRY_BUTTON.visible()
    );
    await vpn.waitForQueryAndClick(
      queries.screenHome.serverListView.EXIT_BUTTON.visible()
    );

    // invalid server search
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenHome.serverListView.SEARCH_BAR_TEXT_FIELD.visible(),
      "invalid search"
    );
    await vpn.waitForQuery(queries.screenHome.serverListView.SEARCH_BAR_ERROR.visible())
    // await vpn.waitForQuery(australia.hidden())
    await vpn.waitForQuery(austria.hidden())
    await vpn.waitForQuery(belgium.hidden())

    await vpn.waitForQueryAndWriteInTextField(
      queries.screenHome.serverListView.SEARCH_BAR_TEXT_FIELD.visible(),
      ""
    );

    // multi result search
    await vpn.waitForQueryAndWriteInTextField(
      queries.screenHome.serverListView.SEARCH_BAR_TEXT_FIELD.visible(),
      "Au"
    );
    await vpn.waitForQuery(australia.visible())
    await vpn.waitForQuery(austria.visible())
    await vpn.waitForQuery(belgium.hidden())
  })
});
