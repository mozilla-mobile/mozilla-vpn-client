/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const websocket = require('websocket').w3cwebsocket;
const FirefoxHelper = require('./firefox.js');

const webdriver = require('selenium-webdriver'), By = webdriver.By,
      Keys = webdriver.Key, until = webdriver.until;

let client;

let waitReadCallback = null;

let _lastNotification = {
  title: null,
  message: null,
};

module.exports = {
  async connect(hostname = '127.0.0.1') {
    await this.waitForCondition(async () => {
      return await new Promise(resolve => {
        client = new websocket(`ws://${hostname}:8765/`, '');

        client.onopen = async () => {
          const json = await this._writeCommand('stealurls');
          assert(
              json.type === 'stealurls' && !('error' in json),
              `Invalid answer: ${json.error}`);
          resolve(true);
        };

        client.onclose = () => this._resolveWaitRead({});
        client.onerror = () => resolve(false);

        client.onmessage = data => {
          const json = JSON.parse(data.data);

          // Ignoring logs.
          if (json.type === 'log') return;

          // Store the last notification
          if (json.type === 'notification') {
            _lastNotification.title = json.title;
            _lastNotification.message = json.message;
            return;
          }

          assert(waitReadCallback, 'No waiting callback?');
          this._resolveWaitRead(json);
        };
      });
    });
  },

  disconnect() {
    client.close();
  },

  async activate() {
    const json = await this._writeCommand('activate');
    assert(
        json.type === 'activate' && !('error' in json),
        `Invalid answer: ${json.error}`);
  },

  async deactivate() {
    const json = await this._writeCommand('deactivate');
    assert(
        json.type === 'deactivate' && !('error' in json),
        `Invalid answer: ${json.error}`);
  },

  async reset() {
    const json = await this._writeCommand('reset');
    assert(
        json.type === 'reset' && !('error' in json),
        `Invalid answer: ${json.error}`);
  },

  async forceHeartbeatFailure() {
    const json = await this._writeCommand('force_heartbeat_failure');
    assert(
        json.type === 'force_heartbeat_failure' && !('error' in json),
        `Invalid answer: ${json.error}`);
  },

  async forceUnsecuredNetworkAlert() {
    const json = await this._writeCommand('force_unsecured_network');
    assert(
        json.type === 'force_unsecured_network' && !('error' in json),
        `Invalid answer: ${json.error}`);
  },

  async forceCaptivePortalDetection() {
    const json = await this._writeCommand('force_captive_portal_detection');
    assert(
        json.type === 'force_captive_portal_detection' && !('error' in json),
        `Invalid answer: ${json.error}`);
  },

  async quit() {
    const json = await this._writeCommand('quit');
    assert(
        !('type' in json) || (json.type === 'quit' && !('error' in json)),
        `Invalid answer: ${json.error}`);
  },

  async hasElement(id) {
    const json = await this._writeCommand(`has ${id}`);
    assert(
        json.type === 'has' && !('error' in json),
        `Invalid answer: ${json.error}`);
    return json.value || false;
  },

  async waitForElement(id) {
    return this.waitForCondition(async () => {
      return await this.hasElement(id);
    });
  },

  async clickOnElement(id) {
    assert(await this.hasElement(id), 'Clicking on an non-existing element?!?');
    const json = await this._writeCommand(`click ${id}`);
    assert(
        json.type === 'click' && !('error' in json),
        `Invalid answer: ${json.error}`);
  },

  async clickOnNotification() {
    const json = await this._writeCommand('click_notification');
    assert(
        json.type === 'click_notification' && !('error' in json),
        `Invalid answer: ${json.error}`);
  },

  async getElementProperty(id, property) {
    assert(
        await this.hasElement(id),
        'Property checks must be done on existing elements');
    const json = await this._writeCommand(`property ${id} ${property}`);
    assert(
        json.type === 'property' && !('error' in json),
        `Invalid answer: ${json.error}`);
    return json.value || '';
  },

  async setElementProperty(id, property, type, value) {
    assert(
        await this.hasElement(id),
        'Property checks must be done on existing elements');
    const json = await this._writeCommand(
        `set_property ${id} ${property} ${type} ${value}`);
    assert(
        json.type === 'set_property' && !('error' in json),
        `Invalid answer: ${json.error}`);
  },

  async waitForElementProperty(id, property, value) {
    assert(
        await this.hasElement(id),
        'Property checks must be done on existing elements');
    try {
      return this.waitForCondition(async () => {
        const real = await this.getElementProperty(id, property);
        return real === value;
      });
    } catch (e) {
      const real = await this.getElementProperty(id, property);
      throw new Error(`Timeout for waitForElementProperty - property: ${
          property} - value: ${real} - expected: ${value}`);
    }
  },

  async getLastUrl() {
    const json = await this._writeCommand('lasturl');
    assert(
        json.type === 'lasturl' && !('error' in json),
        `Invalid answer: ${json.error}`);
    return json.value || '';
  },

  async waitForCondition(condition) {
    for (let i = 0; i < 50; ++i) {
      if (await condition()) return;
      await new Promise(resolve => setTimeout(resolve, 200));
    }
    throw new Error('Timeout for waitForCondition');
  },

  wait() {
    return new Promise(resolve => setTimeout(resolve, 500));
  },

  async authenticate(driver, resetting = true) {
    if (resetting) await this.reset();

    await this.waitForElement('getHelpLink');
    await this.waitForElementProperty('getHelpLink', 'visible', 'true');
    assert(await this.getElementProperty('getStarted', 'visible') === 'true');
    assert(
        await this.getElementProperty('learnMoreLink', 'visible') === 'true');

    await this.clickOnElement('getStarted');

    await this.waitForCondition(async () => {
      const url = await this.getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });

    await this.wait();

    await this.waitForElement('authenticatingView');
    await this.waitForElementProperty('authenticatingView', 'visible', 'true');

    const url = await this.getLastUrl();

    await driver.setContext('content');
    await driver.navigate().to(url);

    await FirefoxHelper.waitForURL(
        driver, 'https://accounts.stage.mozaws.net/oauth/');

    const emailField = await driver.findElement(By.className('email'));
    assert.ok(!!emailField);
    await emailField.sendKeys(process.env.ACCOUNT_EMAIL);

    let buttonElm = await driver.findElement(By.id('submit-btn'));
    assert.ok(!!buttonElm);
    buttonElm.click();

    await FirefoxHelper.waitForURL(
        driver, 'https://accounts.stage.mozaws.net/oauth/signin');

    const passwordField = await driver.findElement(By.id('password'));
    assert.ok(!!passwordField);
    passwordField.sendKeys(process.env.ACCOUNT_PASSWORD);

    buttonElm = await driver.findElement(By.id('submit-btn'));
    assert.ok(!!buttonElm);
    await buttonElm.click();

    await FirefoxHelper.waitForURL(
        driver,
        'https://stage-vpn.guardian.nonprod.cloudops.mozgcp.net/vpn/client/login/success');

    await this.wait();
  },

  async logout() {
    const json = await this._writeCommand('logout');
    assert(
        json.type === 'logout' && !('error' in json),
        `Invalid answer: ${json.error}`);
  },

  async setSetting(key, value) {
    const json = await this._writeCommand(`set_setting ${key} ${value}`);
    assert(
        json.type === 'set_setting' && !('error' in json),
        `Invalid answer: ${json.error}`);
  },

  async getSetting(key) {
    const json = await this._writeCommand(`setting ${key}`);
    assert(
        json.type === 'setting' && !('error' in json),
        `Invalid answer: ${json.error}`);
    return json.value;
  },

  lastNotification() {
    return _lastNotification;
  },

  resetLastNotification() {
    _lastNotification.title = null;
    _lastNotification.message = null;
  },

  async languages() {
    const json = await this._writeCommand('languages');
    assert(
        json.type === 'languages' && !('error' in json),
        `Invalid answer: ${json.error}`);
    return json.value;
  },

  async servers() {
    const json = await this._writeCommand('servers');
    assert(
        json.type === 'servers' && !('error' in json),
        `Invalid answer: ${json.error}`);
    return json.value;
  },

  async screenCapture() {
    const json = await this._writeCommand('screen_capture');
    assert(
        json.type === 'screen_capture' && !('error' in json),
        `Invalid answer: ${json.error}`);
    return json.value;
  },

  // Internal methods.

  _writeCommand(command) {
    return new Promise(resolve => {
      waitReadCallback = resolve;
      client.send(`${command}`);
    });
  },

  _resolveWaitRead(json) {
    if (waitReadCallback) {
      const wr = waitReadCallback;
      waitReadCallback = null;

      wr(json);
    }
  },
};
