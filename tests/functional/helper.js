/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const websocket = require('websocket').w3cwebsocket;
const FirefoxHelper = require('./firefox.js');

const webdriver = require('selenium-webdriver'), By = webdriver.By,
      Keys = webdriver.Key, until = webdriver.until;

let client;

let waitReadBuffer = [];
let waitReadCallback = null;

module.exports = {
  async connect() {
    await this.waitForCondition(async () => {
      return await new Promise(resolve => {
        client = new websocket('ws://localhost:8765/', '');

        client.onopen =
            async () => {
          const buffer = await this._writeCommand('stealurls');
          assert(buffer.length === 1 && buffer[0] === 'ok', 'Invalid answer');
          resolve(true);
        }

                        client.onclose = () => this._resolveWaitRead();
        client.onerror = () => resolve(false);

        client.onmessage = data => {
          // Ignoring status and logs.
          if (data.data[0] == '!' || data.data[0] == '@') return;

          assert(waitReadCallback, 'No waiting callback?');

          const msg = data.data.trim();
          waitReadBuffer.push(msg);

          if (msg === 'ok' || msg === 'ko') {
            this._resolveWaitRead();
          }
        }
      });
    });
  },

  disconnect() {
    client.close();
  },

  async reset() {
    const buffer = await this._writeCommand('reset');
    assert(buffer.length === 1 && buffer[0] === 'ok', 'Invalid answer');
  },

  async quit() {
    const buffer = await this._writeCommand('quit');
    assert(
        buffer.length === 0 || (buffer.length === 1 && (buffer[0] === 'ok')),
        'Invalid answer');
  },

  async hasElement(id) {
    const buffer = await this._writeCommand(`has ${id}`);
    assert(buffer.length === 1, 'Invalid answer');
    if (buffer[0] == 'ok') return true;

    assert(buffer[0] == 'ko', 'Invalid answer');
    return false;
  },

  async waitForElement(id) {
    return this.waitForCondition(async () => {
      return await this.hasElement(id);
    });
  },

  async clickOnElement(id) {
    assert(await this.hasElement(id), 'Clicking on an non-existing element?!?');
    const buffer = await this._writeCommand(`click ${id}`);
    assert(buffer.length === 1 && buffer[0] === 'ok', 'Invalid answer');
  },

  async getElementProperty(id, property) {
    assert(await this.hasElement(id), 'Property checks must be done on existing elements');
    const buffer = await this._writeCommand(`property ${id} ${property}`);
    assert(buffer.length === 1 || buffer.length === 2, 'Invalid answer');

    const msg = buffer[0];

    if (msg == 'ko') return null;
    if (msg[0] != '-' || msg[msg.length - 1] != '-') return null;
    return msg.substring(1, msg.length - 1);
  },

  async waitForElementProperty(id, property, value) {
    assert(await this.hasElement(id), 'Property checks must be done on existing elements');
    return this.waitForCondition(async () => {
      const real = await this.getElementProperty(id, property);
      return real == value;
    });
  },

  async getLastUrl() {
    const buffer = await this._writeCommand('lasturl');
    assert(buffer.length === 1 || buffer.length === 2, 'Invalid answer');

    const msg = buffer[0];
    if (msg[0] != '-' || msg[msg.length - 1] != '-') return null;
    return msg.substring(1, msg.length - 1);
  },

  async waitForCondition(condition) {
    for (let i = 0; i < 10; ++i) {
      if (await condition()) return;
      await this.wait();
    }
    throw new Error('Timeout for waitForCondition');
  },

  wait() {
    return new Promise(resolve => setTimeout(resolve, 1000));
  },

  async authenticate(driver, resetting = true) {
    if (resetting) await this.reset();

    await this.waitForElement('getHelpLink');
    await this.waitForElementProperty('getHelpLink', 'visible', 'true');
    assert(await this.getElementProperty('getStarted', 'visible') == 'true');
    assert(await this.getElementProperty('learnMoreLink', 'visible') == 'true');

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

  // Internal methods.

  _writeCommand(command) {
    return new Promise(resolve => {
      waitReadCallback = resolve;
      client.send(`${command}`);
    });
  },

  _resolveWaitRead() {
    if (waitReadCallback) {
      const wr = waitReadCallback;
      waitReadCallback = null;

      const buffer = waitReadBuffer;
      waitReadBuffer = [];

      wr(buffer);
    }
  },
};
