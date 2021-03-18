/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const util = require('util');
const vpn = require('./helper.js');
const FirefoxHelper = require('./firefox.js');

const webdriver = require('selenium-webdriver'), By = webdriver.By,
      Keys = webdriver.Key, until = webdriver.until;

const exec = util.promisify(require('child_process').exec);

describe('Server list', function() {
  let driver;
  let servers;
  let currentCountryCode;
  let currentCity;

  this.timeout(200000);

  before(async () => {
    await vpn.connect();
    driver = await FirefoxHelper.createDriver();
  });

  beforeEach(() => {});

  afterEach(() => {});

  after(async () => {
    await driver.quit();
    vpn.disconnect();
  });

  it('authenticate', async () => await vpn.authenticate(driver));

  it('Post authentication view', async () => {
    await vpn.waitForElement('postAuthenticationButton');
    await vpn.clickOnElement('postAuthenticationButton');
    await vpn.wait();
  });

  it('opening the server list', async () => {
    await vpn.waitForElement('serverListButton');
    await vpn.waitForElementProperty('serverListButton', 'visible', 'true');
    await vpn.wait();

    await vpn.clickOnElement('serverListButton');
    await vpn.wait();

    await vpn.waitForElement('serverListBackButton');
    await vpn.waitForElementProperty('serverListBackButton', 'visible', 'true');
    await vpn.clickOnElement('serverListBackButton');
    await vpn.wait();

    await vpn.waitForElement('serverListButton');
    await vpn.waitForElementProperty('serverListButton', 'visible', 'true');
    await vpn.wait();

    await vpn.clickOnElement('serverListButton');
    await vpn.wait();
  });

  it('retrieve list of servers and the current one', async () => {
    servers = await vpn.servers();
    currentCountryCode = await vpn.getSetting('current-server-country-code');
    currentCity = await vpn.getSetting('current-server-city');
  });

  it('check the countries and cities', async () => {
    for (let server of servers) {
      const countryId = 'serverCountryList/serverCountry-' + server.code;
      await vpn.waitForElement(countryId);
      await vpn.waitForElementProperty(countryId, 'visible', 'true');
      assert(
          await vpn.getElementProperty(countryId, 'serverCountryName') ===
          server.name);

      await vpn.setElementProperty(
          'serverCountryView', 'contentY', 'i',
          parseInt(await vpn.getElementProperty(countryId, 'y')));

      if (currentCountryCode !== server.code) {
        await vpn.clickOnElement(countryId);
      }

      for (let city of server.cities) {
        const cityId = countryId + '/serverCityList/serverCity-' +
            city.name.replace(/ /g, '_');
        await vpn.waitForElement(cityId);
        await vpn.waitForElementProperty(cityId, 'visible', 'true');
        await vpn.waitForElementProperty(
            cityId, 'radioButtonLabelText', city.name);
        await vpn.waitForElementProperty(
            cityId, 'checked',
            currentCountryCode === server.code && currentCity === city.name ?
                'true' :
                'false');
      }
    }
  });

  it('pick cities', async () => {
    // let's check the first 4 servers only...
    let checkServers = 4;
    while (checkServers-- > 0) {
      let server = servers[Math.floor(Math.random() * servers.length)];

      const countryId = 'serverCountryList/serverCountry-' + server.code;
      await vpn.waitForElement(countryId);

      await vpn.setElementProperty(
          'serverCountryView', 'contentY', 'i',
          parseInt(await vpn.getElementProperty(countryId, 'y')));

      if (currentCountryCode !== server.code) {
        await vpn.clickOnElement(countryId);
      }

      let city =
          server.cities[Math.floor(Math.random() * server.cities.length)];
      const cityId = countryId + '/serverCityList/serverCity-' +
          city.name.replace(/ /g, '_');
      await vpn.waitForElement(cityId);

      await vpn.setElementProperty(
          'serverCountryView', 'contentY', 'i',
          parseInt(await vpn.getElementProperty(cityId, 'y')) +
              parseInt(await vpn.getElementProperty(countryId, 'y')));
      await vpn.wait();

      await vpn.clickOnElement(cityId);
      await vpn.wait();

      currentCountryCode = server.code;
      currentCity = city.name;

      // Back to the main view.

      await vpn.waitForElement('serverListButton');
      await vpn.waitForElementProperty('serverListButton', 'visible', 'true');
      await vpn.waitForElementProperty(
          'serverListButton', 'subtitleText', city.name);
      await vpn.wait();

      await vpn.clickOnElement('serverListButton');
      await vpn.wait();

      // One selected
      await vpn.waitForElement(cityId);
      await vpn.waitForElementProperty(cityId, 'checked', 'true');

      // All the others, unselected
      for (let otherServer of servers) {
        const otherCountryId =
            'serverCountryList/serverCountry-' + otherServer.code;
        await vpn.setElementProperty(
            'serverCountryView', 'contentY', 'i',
            parseInt(await vpn.getElementProperty(otherCountryId, 'y')));

        if (currentCountryCode !== otherServer.code) {
          await vpn.clickOnElement(otherCountryId);
        }

        for (let otherCity of otherServer.cities) {
          if (city.code === otherCity.code) continue;

          const otherCityId = otherCountryId + '/serverCityList/serverCity-' +
              otherCity.name.replace(/ /g, '_');
          await vpn.waitForElementProperty(otherCityId, 'checked', 'false');
        }
      }
    }
  });

  it('server switching', async () => {
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
        currentCountry = server.name;
        break;
      }
    }

    assert(currentCountry != '');

    assert(vpn.lastNotification().title === 'VPN Connected');
    assert(
        vpn.lastNotification().message ===
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
    currentCountry = server.name;
    currentCity = city.name;

    await vpn.waitForElement('controllerTitle');
    await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');

    await vpn.waitForCondition(async () => {
      let connectingMsg =
          await vpn.getElementProperty('controllerTitle', 'text');
      return connectingMsg === 'Switching…';
    });

    assert(
        await vpn.getElementProperty('controllerSubTitle', 'text') ===
        `From ${previousCity} to ${currentCity}`);

    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('controllerTitle', 'text') ==
          'VPN is on';
    });

    assert(vpn.lastNotification().title === 'VPN Switched Servers');
    assert(
        vpn.lastNotification().message ===
        `Switched from ${previousCountry}, ${previousCity} to ${
            currentCountry}, ${currentCity}`);
  });

  // TODO: server list disabled when reached the device limit

  it('Logout', async () => {
    await vpn.logout();
    await vpn.wait();
  });

  it('quit the app', async () => await vpn.quit());
});
