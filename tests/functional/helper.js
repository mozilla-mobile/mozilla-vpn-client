/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const net = require('net');
const client = new net.Socket();

module.exports = {
  async connect() {
    client.on('data', (data) => {
      assert(this._waitRead, 'No waiting callback?');
      this._resolveWaitRead(String.fromCharCode.apply(String, data).trim());
    });

    client.on('close', () => {
      this._resolveWaitRead('');
    });

    for (let i = 0; i < 10; ++i) {
      const connected = await new Promise(resolve => {
        client.connect(8765, 'localhost', () => resolve(true));
        client.on('error', () => resolve(false));
      });
      if (connected) return;

      await this.wait();
    }

    throw new Error('Connection failed');
  },

  disconnect() {
    client.destroy();
  },

  async reset() {
    const buffer = await this._writeCommand('reset');
    assert(buffer == 'ok', 'Invalid answer');
  },

  async quit() {
    const buffer = await this._writeCommand('quit');
    assert(buffer == 'ok' || buffer == '', 'Invalid answer');
  },

  async hasElement(id) {
    const buffer = await this._writeCommand(`has ${id}`);
    if (buffer == 'ok') return true;

    assert(buffer == 'ko', 'Invalid answer');
    return false;
  },

  async waitForElement(id) {
    for (let i = 0; i < 10; ++i) {
      if (await this.hasElement(id)) return;
      await this.wait();
    }
    throw new Error('Timeout for element property');
  },

  async clickOnElement(id) {
    assert(await this.hasElement(id), 'Clicking on an non-existing element?!?');
    const buffer = await this._writeCommand(`click ${id}`);
    assert(buffer == 'ok', 'Invalid answer');
  },

  async getElementProperty(id, property) {
    assert(await this.hasElement(id), 'Property checks must be done on existing elements');
    const buffer = await this._writeCommand(`property ${id} ${property}`);
    if (buffer == 'ko') return null;
    if (buffer[0] != '-' || buffer[buffer.length - 1] != '-') return null;
    return buffer.substring(1, buffer.length - 1);
  },

  async waitForElementProperty(id, property, value) {
    assert(await this.hasElement(id), 'Property checks must be done on existing elements');
    for (let i = 0; i < 10; ++i) {
      const real = await this.getElementProperty(id, property);
      if (real == value) return;
      await this.wait();
    }
    throw new Error('Timeout for element property');
  },

  wait() {
    return new Promise(resolve => setTimeout(resolve, 1000));
  },

  // Internal methods.

  _writeCommand(command) {
    return new Promise(resolve => {
      this._waitRead = resolve;
      client.write(`${command}\n`);
    });
  },

  _waitRead: null,
  _resolveWaitRead(data) {
    if (this._waitRead) {
      const wr = this._waitRead;
      this._waitRead = null;
      wr(data);
    }
  },
};
