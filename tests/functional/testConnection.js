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

describe('User authentication', function() {
  let driver;

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

  it('check the ui', async () => {
    await vpn.waitForElement('controllerTitle');
    await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
    assert(
        await vpn.getElementProperty('controllerTitle', 'text') ==
        'VPN is off');

    await vpn.waitForElementProperty('controllerSubTitle', 'visible', 'true');
    assert(
        await vpn.getElementProperty('controllerSubTitle', 'text') ==
        'Turn on to protect your privacy');

    await vpn.waitForElementProperty('controllerToggle', 'visible', 'true');
  });

  it('connecting', async () => {
    // TODO: investigate why the click doesn't work on github.
    // await vpn.clickOnElement('controllerToggle');
    await vpn.activate();

    await vpn.waitForCondition(async () => {
      let connectingMsg =
          await vpn.getElementProperty('controllerTitle', 'text');
      return connectingMsg == 'Connecting…' ||
          connectingMsg == 'vpn.controller.connectingState';
    });

    assert(
        await vpn.getElementProperty('controllerSubTitle', 'text') ==
        'Masking connection and location');
  });

  it('connected', async () => {
    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('controllerTitle', 'text') ==
          'VPN is on';
    });

    assert((await vpn.getElementProperty('controllerSubTitle', 'text'))
               .startsWith('Secure and private '));

    vpn.wait();
  });

  it('disconnecting', async () => {
    // TODO: investigate why the click doesn't work on github.
    // await vpn.clickOnElement('controllerToggle');
    await vpn.deactivate();

    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('controllerTitle', 'text') ==
          'Disconnecting…';
    });

    assert(
        await vpn.getElementProperty('controllerSubTitle', 'text') ==
        'Unmasking connection and location');

    vpn.wait();
  });

  it('disconnected', async () => {
    await vpn.waitForCondition(async () => {
      return await vpn.getElementProperty('controllerTitle', 'text') ==
          'VPN is off';
    });

    assert(
        await vpn.getElementProperty('controllerSubTitle', 'text') ==
        'Turn on to protect your privacy');

    vpn.wait();
  });

  it('Logout', async () => {
    await vpn.logout();
    await vpn.wait();
  });

  it('quit the app', async () => await vpn.quit());
});
