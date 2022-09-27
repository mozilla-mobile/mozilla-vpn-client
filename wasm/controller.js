/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

class Controller {
  wait() {
    return new Promise(resolve => setTimeout(resolve, 1000));
  }

  async waitForCondition(condition) {
    while (true) {
      if (await condition()) return;
      await new Promise(resolve => setTimeout(resolve, 200));
    }
  }

  async hasElement(id) {
    const json = await this._writeCommand(`has ${id}`);
    assert(
        json.type === 'has' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value || false;
  }

  async getElementProperty(id, property) {
    assert(
        await this.hasElement(id),
        'Property checks must be done on existing elements');
    const json = await this._writeCommand(`property ${id} ${property}`);
    assert(
        json.type === 'property' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value || '';
  }

  async setSetting(key, value) {
    const json = await this._writeCommand(`set_setting ${key} ${value}`);
    assert(
        json.type === 'set_setting' && !('error' in json),
        `Command failed: ${json.error}`);
  }

  async waitForElement(id) {
    return this.waitForCondition(async () => {
      return await this.hasElement(id);
    });
  }

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
  }

  async clickOnElement(id) {
    assert(await this.hasElement(id), 'Clicking on an non-existing element?!?');
    const json = await this._writeCommand(`click ${id}`);
    assert(
        json.type === 'click' && !('error' in json),
        `Command failed: ${json.error}`);
  }

  async waitForMainView() {
    await this.waitForElement('getHelpLink');
    await this.waitForElementProperty('getHelpLink', 'visible', 'true');
    assert(await this.getElementProperty('getStarted', 'visible') === 'true');
    assert(
        await this.getElementProperty('learnMoreLink', 'visible') === 'true');
  }

  async hardReset() {
    const json = await this._writeCommand('hard_reset');
    assert(
        json.type === 'hard_reset' && !('error' in json),
        `Command failed: ${json.error}`);
  }

  async reset() {
    const json = await this._writeCommand('reset');
    assert(
        json.type === 'reset' && !('error' in json),
        `Command failed: ${json.error}`);
  }

  async flipFeatureOn(key) {
    const json = await this._writeCommand(`flip_on_feature ${key}`);
    assert(
        json.type === 'flip_on_feature' && !('error' in json),
        `Command failed: ${json.error}`);
  }

  async forceUpdateCheck(version) {
    const json = await this._writeCommand(`force_update_check ${version}`);
    assert(
        json.type === 'force_update_check' && !('error' in json),
        `Command failed: ${json.error}`);
  }

  async backButtonClicked() {
    const json = await this._writeCommand('back_button_clicked');
    assert(
        json.type === 'back_button_clicked' && !('error' in json),
        `Command failed: ${json.error}`);
  }

  _writeCommand(cmd) {
    return new Promise(resolve => {
      this._waitReadCallback = resolve;
      Module.inspectorCommand(cmd);
    });
  }

  _messageReceived(obj) {
    if (!this._initialized) {
      this._initialized = true;
      this._initialize();
    }

    // Ignoring logs.
    if (obj.type === 'log') return;
    if (obj.type === 'network') return;
    if (obj.type === 'notification') return;

    assert(this._waitReadCallback, 'No waiting callback?');
    const wr = this._waitReadCallback;
    this._waitReadCallback = null;
    wr(obj);
  }

  async _initialize() {
    const json = await this._writeCommand(`languages`);
    assert(
        json.type === 'languages' && !('error' in json),
        `Command failed: ${json.error}`);
    mvpnWasm.updateLanguages(json.value);
  }
};

const controller = new Controller();

function inspectorMessage(obj) {
  controller._messageReceived(obj);
}
