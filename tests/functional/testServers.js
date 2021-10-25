/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
const assert = require('assert');
const vpn = require('./helper.js');

describe('Server list', function() {
  let servers;
  let currentCountryCode;
  let currentCity;

  this.timeout(240000);

  beforeEach(async () => {
    await vpn.authenticate(true, true);
    await vpn.waitForElement('serverListButton');
    await vpn.waitForElementProperty('serverListButton', 'visible', 'true');
    await vpn.clickOnElement('serverListButton');
    await vpn.wait();

    servers = await vpn.servers();
    currentCountryCode = await vpn.getSetting('current-server-country-code');
    currentCity = await vpn.getSetting('current-server-city');

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
    await vpn.waitForElement('serverListBackButton');
    await vpn.waitForElementProperty('serverListBackButton', 'visible', 'true');
    await vpn.clickOnElement('serverListBackButton');
    await vpn.wait();

    await vpn.waitForElement('serverListButton');
    await vpn.waitForElementProperty('serverListButton', 'visible', 'true');
  });

  it('check the countries and cities', async () => {
    for (let server of servers) {
      const countryId = 'serverCountryList/serverCountry-' + server.code;
      await vpn.waitForElement(countryId);
      await vpn.waitForElementProperty(countryId, 'visible', 'true');

      await vpn.setElementProperty(
          'serverCountryView', 'contentY', 'i',
          parseInt(await vpn.getElementProperty(countryId, 'y')));
      await vpn.wait();

      if (currentCountryCode === server.code) {
        assert(
            await vpn.getElementProperty(countryId, 'cityListVisible') ===
            'true');
      }

      if (await vpn.getElementProperty(countryId, 'cityListVisible') ===
          'false') {
        await vpn.clickOnElement(countryId);
      }

      for (let city of server.cities) {
        const cityId = countryId + '/serverCityList/serverCity-' +
            city.name.replace(/ /g, '_');
        console.log('  Waiting for cityId:', cityId);
        await vpn.waitForElement(cityId);
        await vpn.waitForElementProperty(cityId, 'visible', 'true');
        await vpn.waitForElementProperty(
            cityId, 'checked',
            currentCountryCode === server.code &&
                    currentCity === city.localizedName ?
                'true' :
                'false');
      }
    }
  });

  it('Pick cities', async () => {
    for (let server of servers) {
      const countryId = 'serverCountryList/serverCountry-' + server.code;
      await vpn.waitForElement(countryId);

      await vpn.setElementProperty(
          'serverCountryView', 'contentY', 'i',
          parseInt(await vpn.getElementProperty(countryId, 'y')));
      await vpn.wait();

      if (await vpn.getElementProperty(countryId, 'cityListVisible') ===
          'false') {
        await vpn.clickOnElement(countryId);
      }

      await vpn.waitForElementProperty(countryId, 'cityListVisible', 'true');

      for (let city of server.cities) {
        console.log('  Start test for city:', city);
        const cityId = countryId + '/serverCityList/serverCity-' +
            city.name.replace(/ /g, '_');
        await vpn.waitForElement(cityId);

        await vpn.setElementProperty(
            'serverCountryView', 'contentY', 'i',
            parseInt(await vpn.getElementProperty(cityId, 'y')) +
                parseInt(await vpn.getElementProperty(countryId, 'y')));
        await vpn.waitForElementProperty(cityId, 'visible', 'true');

        const cityName =
            await vpn.getElementProperty(cityId, 'radioButtonLabelText');

        await vpn.wait();

        await vpn.clickOnElement(cityId);
        await vpn.wait();

        currentCountryCode = server.code;
        currentCity = city.localizedName;

        // Back to the main view.

        await vpn.waitForElement('serverListButton');
        await vpn.waitForElementProperty('serverListButton', 'visible', 'true');
        await vpn.waitForElementProperty(
            'serverListButton', 'subtitleText', cityName);

        await vpn.clickOnElement('serverListButton');
        await vpn.wait();

        // One selected
        await vpn.waitForElement(cityId);
        await vpn.waitForElementProperty(cityId, 'checked', 'true');
      }
    }
  });

  it('Server switching', async () => {
    await vpn.setSetting('server-switch-notification', 'true');
    await vpn.setSetting('connection-change-notification', 'true');
    await vpn.waitForElement('serverListBackButton');
    await vpn.waitForElementProperty('serverListBackButton', 'visible', 'true');
    await vpn.clickOnElement('serverListBackButton');
    await vpn.wait();

    await vpn.waitForElement('serverListButton');
    await vpn.waitForElementProperty('serverListButton', 'visible', 'true');
    await vpn.wait();

    await vpn.activate();

    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('controllerTitle', 'text') ==
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
        `Connected to ${currentCountry}, ${currentCity}`);

    await vpn.clickOnElement('serverListButton');
    await vpn.wait();

    let server;
    while (true) {
      server = servers[Math.floor(Math.random() * servers.length)];
      if (server.code != currentCountryCode) break;
    }

    const countryId = 'serverCountryList/serverCountry-' + server.code;
    await vpn.waitForElement(countryId);

    await vpn.setElementProperty(
        'serverCountryView', 'contentY', 'i',
        parseInt(await vpn.getElementProperty(countryId, 'y')));

    await vpn.clickOnElement(countryId);

    let city = server.cities[Math.floor(Math.random() * server.cities.length)];
    const cityId = countryId + '/serverCityList/serverCity-' +
        city.name.replace(/ /g, '_');
    await vpn.waitForElement(cityId);

    await vpn.setElementProperty(
        'serverCountryView', 'contentY', 'i',
        parseInt(await vpn.getElementProperty(cityId, 'y')) +
            parseInt(await vpn.getElementProperty(countryId, 'y')));
    await vpn.wait();

    await vpn.clickOnElement(cityId);

    const previousCountry = currentCountry;
    const previousCity = currentCity;

    currentCountryCode = server.code;
    currentCountry = server.localizedName;
    currentCity = city.localizedName;

    await vpn.waitForElement('controllerTitle');
    await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');

    await vpn.waitForCondition(async () => {
      let connectingMsg =
          await vpn.getElementProperty('controllerTitle', 'text');
      return connectingMsg === 'Switching…';
    });

    assert.strictEqual(
        await vpn.getElementProperty('controllerSubTitle', 'text'),
        `From ${previousCity} to ${currentCity}`);

    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('controllerTitle', 'text') ==
          'VPN is on';
    });

    assert.strictEqual(vpn.lastNotification().title, 'VPN Switched Servers');
    assert.strictEqual(
        vpn.lastNotification().message,
        `Switched from ${previousCountry}, ${previousCity} to ${
            currentCountry}, ${currentCity}`);
  });

  // TODO: server list disabled when reached the device limit

});
