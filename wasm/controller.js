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

  async query(id) {
    const json = await this._writeCommand(`query ${encodeURIComponent(id)}`);
    assert(
        json.type === 'query' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value || false;
  }

  async getVPNProperty(id, property) {
    const json = await this._writeCommand(
        `property ${encodeURIComponent(id)} ${encodeURIComponent(property)}`);
    assert(
        json.type === 'property' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value || '';
  }

  async setSetting(key, value) {
    const json = await this._writeCommand(
        `set_setting ${encodeURIComponent(key)} ${encodeURIComponent(value)}`);
    assert(
        json.type === 'set_setting' && !('error' in json),
        `Command failed: ${json.error}`);
  }

  async waitForQuery(id) {
    return this.waitForCondition(async () => {
      return await this.query(id);
    });
  }

  async waitForVPNProperty(id, property, value) {
    try {
      return this.waitForCondition(async () => {
        const real = await this.getVPNProperty(id, property);
        return real === value;
      });
    } catch (e) {
      const real = await this.getVPNProperty(id, property);
      throw new Error(`Timeout for waitForVPNProperty - property: ${
          property} - value: ${real} - expected: ${value}`);
    }
  }

  async waitForQueryAndClick(id) {
    await this.waitForQuery(id);
    await this.clickOnQuery(id);
  }

  async clickOnQuery(query) {
    assert(await this.query(query), 'Clicking on an non-existing element?!?');
    const json = await this._writeCommand(`click ${encodeURIComponent(query)}`);
    assert(
        json.type === 'click' && !('error' in json),
        `Command failed: ${json.error}`);
  }

  async waitForInitialView() {
    await this.waitForQuery('//getHelpLink{visible=true}');
    assert(await this.query('//signUpButton{visible=true}'));
    assert(await this.query('//alreadyASubscriberLink{visible=true}'));
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

  async flipFeatureOff(key) {
    const json =
        await this._writeCommand(`flip_off_feature ${encodeURIComponent(key)}`);
    assert(
        json.type === 'flip_off_feature' && !('error' in json),
        `Command failed: ${json.error}`);
  }

  async flipFeatureOn(key) {
    const json =
        await this._writeCommand(`flip_on_feature ${encodeURIComponent(key)}`);
    assert(
        json.type === 'flip_on_feature' && !('error' in json),
        `Command failed: ${json.error}`);
  }

  async forceUpdateCheck(version) {
    const json = await this._writeCommand(
        `force_update_check ${encodeURIComponent(version)}`);
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
    if (obj.type === 'addon_load_completed') return;

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

let fontLoaded = false;
function inspectorMessage(obj) {
  if (!fontLoaded) {
    fontLoaded = true;

    for (const font
             of ['NotoSansPhagsPa-Regular.ttf', 'NotoSansSC-Regular.otf',
                 'NotoSansTC-Regular.otf']) {
      fetch(font)
          .then(r => r.arrayBuffer())
          .then(content => Module.mzLoadFont(font, content));
    }
  }

  controller._messageReceived(obj);
}
