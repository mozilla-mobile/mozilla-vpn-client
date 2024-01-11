/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 
import { strictEqual } from "assert";
import { screenHome } from "./queries.js";
import { waitForQueryAndClick, waitForQuery, servers as _servers, getMozillaProperty, waitForQueryAndWriteInTextField, scrollToQuery, getQueryProperty, setQueryProperty, activate, waitForCondition, lastNotification, query } from "./helper.js";

describe("Server list", function () {
  let servers;
  let currentCountryCode;
  let currentCity;

  this.timeout(240000);
  this.ctx.authenticationNeeded = true;

  beforeEach(async () => {
    await waitForQueryAndClick(screenHome.SERVER_LIST_BUTTON);
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
      "Current city (localized):",
      currentCity,
      "| Current country code:",
      currentCountryCode
    );
  });

  it("multihop valid search", async () => {
    await waitForQueryAndClick(
      screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible()
    );
    await waitForQuery(
      screenHome.serverListView.ENTRY_BUTTON.visible()
    );
    await waitForQueryAndClick(
      screenHome.serverListView.EXIT_BUTTON.visible()
    );

    const server = servers[servers.length - 1];
    console.log("server name: ", server.name);
    await waitForQueryAndWriteInTextField(
      screenHome.serverListView.SEARCH_BAR_TEXT_FIELD.visible(),
      server.name
    );

    const countryId = screenHome.serverListView.generateCountryId(
      server.code
    );
    await waitForQuery(countryId.visible());
    await scrollToQuery(
      screenHome.serverListView.COUNTRY_VIEW,
      countryId
    );

    if (
      (await getQueryProperty(countryId, "cityListVisible")) === "false"
    ) {
      await waitForQueryAndClick(countryId.visible());
    }

    await waitForQuery(countryId.visible().prop("cityListVisible", true));

    const city = server.cities[0]
    console.log("Start test for city:", city);
    const cityId = screenHome.serverListView.generateCityId(
      countryId,
      city.name
    );
    await waitForQuery(cityId.visible());

    await setQueryProperty(
      screenHome.serverListView.COUNTRY_VIEW,
      "contentY",
      parseInt(await getQueryProperty(cityId, "y")) +
        parseInt(await getQueryProperty(countryId, "y"))
    );
    await waitForQuery(cityId.visible());

    await waitForQueryAndClick(cityId.visible());
    await waitForQuery(screenHome.STACKVIEW.ready());

    // navigate back to connection view
    await waitForQueryAndClick(
      screenHome.serverListView.BACK_BUTTON.visible()
    );

    // define connected server
    currentCity = city.localizedName;

    // connect vpn
    await activate();

    // wait and assert vpn connection
    await waitForCondition(async () => {
      return (
        (await getQueryProperty(
          screenHome.CONTROLLER_TITLE,
          "text"
        )) == "VPN is on"
      );
    });
    strictEqual(lastNotification().title, "VPN Connected");
    strictEqual(
      lastNotification().message,
      `Connected through ${currentCity} via ${currentCity}`
    );
  });

  it("singlehop valid search", async () => {
    const server = servers[0];
    console.log("server name: ", server.name);
    await waitForQueryAndWriteInTextField(
      screenHome.serverListView.SEARCH_BAR_TEXT_FIELD.visible(),
      server.name
    );

    const countryId = screenHome.serverListView.generateCountryId(
      server.code
    );
    await waitForQuery(countryId.visible());
    await scrollToQuery(
      screenHome.serverListView.COUNTRY_VIEW,
      countryId
    );

    if (
      (await getQueryProperty(countryId, "cityListVisible")) === "false"
    ) {
      await waitForQueryAndClick(countryId.visible());
    }
    await waitForQuery(countryId.visible().prop("cityListVisible", true));

    console.log("Start test for city:", server.cities[0]);
    const cityId = screenHome.serverListView.generateCityId(
      countryId,
      server.cities[0].name
    );
    await waitForQuery(cityId.visible());

    await setQueryProperty(
      screenHome.serverListView.COUNTRY_VIEW,
      "contentY",
      parseInt(await getQueryProperty(cityId, "y")) +
        parseInt(await getQueryProperty(countryId, "y"))
    );
    await waitForQuery(cityId.visible());

    await waitForQueryAndClick(cityId.visible());
    await waitForQuery(screenHome.STACKVIEW.ready());

    // currentCountryCode = server.code;
    currentCity = server.cities[0].localizedName;

    await activate();

    await waitForCondition(async () => {
      return (
        (await getQueryProperty(
          screenHome.CONTROLLER_TITLE,
          "text"
        )) == "VPN is on"
      );
    });
    strictEqual(lastNotification().title, "VPN Connected");
    strictEqual(
      lastNotification().message,
      `Connected through ${currentCity}`
    );
  });

  it('Invalid searchs and multi results for singlehop', async () => {
    const australia = screenHome.serverListView.generateCountryId("au");
    const austria = screenHome.serverListView.generateCountryId("at");
    const belgium = screenHome.serverListView.generateCountryId("be");

    // No result search
    await waitForQueryAndWriteInTextField(
      screenHome.serverListView.SEARCH_BAR_TEXT_FIELD.visible(),
      "invalid search"
    );
    await waitForQuery(screenHome.serverListView.SEARCH_BAR_ERROR.visible())
    await waitForQueryAndWriteInTextField(
      screenHome.serverListView.SEARCH_BAR_TEXT_FIELD.visible(),
      ""
    );

    // multi result search
    await waitForQueryAndWriteInTextField(
      screenHome.serverListView.SEARCH_BAR_TEXT_FIELD.visible(),
      "Au"
    );

    await waitForQuery(australia.visible())
    await waitForQuery(austria.visible())
    await query(belgium.hidden())
  })

  it('Invalid searchs and multi results for multihop', async () => {
    const australia = screenHome.serverListView.generateCountryId("au");
    const austria = screenHome.serverListView.generateCountryId("at");
    const belgium = screenHome.serverListView.generateCountryId("be");

    await waitForQueryAndClick(
      screenHome.serverListView.MULTIHOP_SELECTOR_TAB.visible()
    );
    await waitForQuery(
      screenHome.serverListView.ENTRY_BUTTON.visible()
    );
    await waitForQueryAndClick(
      screenHome.serverListView.EXIT_BUTTON.visible()
    );

    // invalid server search
    await waitForQueryAndWriteInTextField(
      screenHome.serverListView.SEARCH_BAR_TEXT_FIELD.visible(),
      "invalid search"
    );
    await waitForQuery(screenHome.serverListView.SEARCH_BAR_ERROR.visible())
    // await vpn.waitForQuery(australia.hidden())
    await waitForQuery(austria.hidden())
    await waitForQuery(belgium.hidden())

    await waitForQueryAndWriteInTextField(
      screenHome.serverListView.SEARCH_BAR_TEXT_FIELD.visible(),
      ""
    );

    // multi result search
    await waitForQueryAndWriteInTextField(
      screenHome.serverListView.SEARCH_BAR_TEXT_FIELD.visible(),
      "Au"
    );
    await waitForQuery(australia.visible())
    await waitForQuery(austria.visible())
    await waitForQuery(belgium.hidden())
  })
});
