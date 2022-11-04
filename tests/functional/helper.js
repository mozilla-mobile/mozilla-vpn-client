/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const {URL} = require('node:url');
const http = require('http')

let client;

let waitReadCallback = null;

let _lastNotification = {
  title: null,
  message: null,
};

module.exports = {
  async connect(impl, options) {
    client = impl;
    await this.waitForCondition(async () => {
      return await impl.connect(
          options,
          async () => {
            const json = await this._writeCommand('stealurls');
            assert(
                json.type === 'stealurls' && !('error' in json),
                `Command failed: ${json.error}`);
          },
          () => this._resolveWaitRead({}),
          json => {
            // Ignoring logs.
            if (json.type === 'log') return;
            if (json.type === 'network') return;

            // Store the last notification
            if (json.type === 'notification') {
              _lastNotification.title = json.title;
              _lastNotification.message = json.message;
              return;
            }

            assert(waitReadCallback, 'No waiting callback?');
            this._resolveWaitRead(json);
          });
    });
  },

  disconnect() {
    client.close();
  },

  async activate(awaitConnectionOkay = false) {
    const json = await this._writeCommand('activate');
    assert(
        json.type === 'activate' && !('error' in json),
        `Command failed: ${json.error}`);

    if (awaitConnectionOkay) {
      await this.waitForCondition(async () => {
        let title = await this.getElementProperty('controllerTitle', 'text');
        let unsettled =
            await this.getElementProperty('VPNConnectionHealth', 'unsettled');
        return (title == 'VPN is on') && (unsettled == 'false');
      });
    }
  },

  async deactivate() {
    const json = await this._writeCommand('deactivate');
    assert(
        json.type === 'deactivate' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async hardReset() {
    const json = await this._writeCommand('hard_reset');
    assert(
        json.type === 'hard_reset' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async reset() {
    const json = await this._writeCommand('reset');
    assert(
        json.type === 'reset' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async waitForMainView() {
    await this.waitForElement('getHelpLink');
    await this.waitForElementProperty('getHelpLink', 'visible', 'true');
    assert(await this.getElementProperty('getStarted', 'visible') === 'true');
    assert(
        await this.getElementProperty('learnMoreLink', 'visible') === 'true');
  },

  async forceHeartbeatFailure() {
    const json = await this._writeCommand('force_heartbeat_failure');
    assert(
        json.type === 'force_heartbeat_failure' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async forceUnsecuredNetworkAlert() {
    const json = await this._writeCommand('force_unsecured_network');
    assert(
        json.type === 'force_unsecured_network' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async forceServerUnavailable() {
    const json = await this._writeCommand('force_server_unavailable');
    assert(
        json.type === 'force_server_unavailable' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async forceSubscriptionManagementReauth() {
    const json = await this._writeCommand(
        'force_subscription_management_reauthentication');
    assert(
        json.type === 'force_subscription_management_reauthentication' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async forceCaptivePortalDetection() {
    const json = await this._writeCommand('force_captive_portal_detection');
    assert(
        json.type === 'force_captive_portal_detection' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async quit() {
    const json = await this._writeCommand('quit');
    assert(
        !('type' in json) || (json.type === 'quit' && !('error' in json)),
        `Command failed: ${json.error}`);
  },

  async hasElement(id) {
    const json = await this._writeCommand(`has ${id}`);
    assert(
        json.type === 'has' && !('error' in json),
        `Command failed: ${json.error}`);
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
        `Command failed: ${json.error}`);
  },

  async waitForElementAndClick(id) {
    await this.waitForElementAndProperty(id, 'visible', 'true');
    await this.clickOnElement(id)
    await this.wait()
  },

  async clickOnNotification() {
    const json = await this._writeCommand('click_notification');
    assert(
        json.type === 'click_notification' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async scrollToElement(view, id) {
    assert(await this.hasElement(view), 'Scrolling on an non-existing view?!?');
    assert(await this.hasElement(id), 'Requesting an non-existing element?!?');

    const contentHeight =
        parseInt(await this.getElementProperty(view, 'contentHeight'));
    const height = parseInt(await this.getElementProperty(view, 'height'));
    let maxScroll = (contentHeight > height) ? contentHeight - height : 0;
    let elementY = parseInt(await this.getElementProperty(id, 'y'));

    let contentY = elementY - (height / 2);
    if (contentY < 0) contentY = 0;
    if (contentY > maxScroll) contentY = maxScroll;

    await this.setElementProperty(view, 'contentY', 'i', contentY);
    await this.wait();
  },

  async getElementProperty(id, property) {
    assert(
        await this.hasElement(id),
        'Property checks must be done on existing elements');
    const json = await this._writeCommand(`property ${id} ${property}`);
    assert(
        json.type === 'property' && !('error' in json),
        `Command failed: ${json.error}`);
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
        `Command failed: ${json.error}`);
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

  async waitForElementAndProperty(id, property, value) {
    await this.waitForElement(id)
    await this.waitForElementProperty(id, property, value)
  },

  async setGleanAutomationHeader() {
    const json = await this._writeCommand('set_glean_source_tags automation');
    assert(
        json.type === 'set_glean_source_tags' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value || null;
  },

  async getLastUrl() {
    return await this.getElementProperty('VPNUrlOpener', 'lastUrl');
  },

  async waitForCondition(condition, waitTimeInMilliSecs = 500) {
    while (true) {
      if (await condition()) return;
      await new Promise(resolve => setTimeout(resolve, waitTimeInMilliSecs));
    }
  },

  wait(waitTimeInMilliSecs = 500) {
    return new Promise(resolve => setTimeout(resolve, waitTimeInMilliSecs));
  },

  // TODO - The expected staging urls are hardcoded, we may want to
  // move these hardcoded urls out if testing in alternate environments.
  async authenticateInBrowser(clickOnPostAuthenticate, acceptTelemetry, wasm) {
    // This method must be called when the client is on the "Get Started" view.
    await this.waitForMainView();
    await this.setElementProperty('VPNUrlOpener', 'lastUrl', 's', '');

    // Click on get started and wait for authenticating view
    await this.clickOnElement('getStarted');

    if (!wasm) {
      await this.waitForCondition(async () => {
        const url = await this.getLastUrl();
        return url.includes('/api/v2/vpn/login');
      });
      await this.wait();

      // We don't really want to go through the authentication flow because we
      // are mocking everything.
      const url = await this.getLastUrl();
      const urlObj = new URL(url);

      const options = {
        hostname: urlObj.hostname,
        port: parseInt(urlObj.searchParams.get('port'), 10),
        path: '/?code=the_code',
        method: 'GET',
      };

      await new Promise(resolve => {
        const req = http.request(options, res => {});
        req.on('close', resolve);
        req.on('error', error => {
          throw new error(
              `Unable to connect to ${urlObj.hostname} to complete the auth`);
        });
        req.end();
      });
    }

    // Wait for VPN client screen to move from spinning wheel to next screen
    await this.waitForElementProperty('VPN', 'userState', 'UserAuthenticated');
    await this.waitForElement('postAuthenticationButton');

    // Clean-up extra devices (otherwise test account will fill up in a
    // heartbeats)
    await this._maybeRemoveExistingDevices();

    if (clickOnPostAuthenticate) {
      await this.clickOnElement('postAuthenticationButton');
      await this.wait();
    }
    if (acceptTelemetry) {
      await this.waitForElement('telemetryPolicyButton');
      await this.clickOnElement('telemetryPolicyButton');
      await this.waitForElement('controllerTitle');
    }
  },

  async authenticateInApp(
      clickOnPostAuthenticate = false, acceptTelemetry = false) {
    if (!(await this.isFeatureFlippedOn('inAppAuthentication'))) {
      await this.flipFeatureOn('inAppAuthentication');
    }

    // This method must be called when the client is on the "Get Started" view.
    await this.waitForMainView();

    // Click on get started and wait for authenticating view
    await this.clickOnElement('getStarted');
    await this.waitForElement('authStart-textInput');
    await this.setElementProperty(
        'authStart-textInput', 'text', 's', 'test@test');
    await this.waitForElement('authStart-button');
    await this.clickOnElement('authStart-button');

    await this.waitForElement('authSignIn-passwordInput');
    await this.setElementProperty(
        'authSignIn-passwordInput', 'text', 's', 'password');

    await this.clickOnElement('authSignIn-button');

    // Wait for VPN client screen to move from spinning wheel to next screen
    await this.waitForElementProperty('VPN', 'userState', 'UserAuthenticated');
    await this.waitForElement('postAuthenticationButton');

    // Clean-up extra devices (otherwise test account will fill up in a
    // heartbeats)
    await this._maybeRemoveExistingDevices();

    if (clickOnPostAuthenticate) {
      await this.clickOnElement('postAuthenticationButton');
      await this.wait();
    }
    if (acceptTelemetry) {
      await this.waitForElement('telemetryPolicyButton');
      await this.clickOnElement('telemetryPolicyButton');
      await this.waitForElement('controllerTitle');
    }
  },

  async logout() {
    const json = await this._writeCommand('logout');
    assert(
        json.type === 'logout' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async isFeatureFlippedOn(key) {
    const json = await this._writeCommand(`is_feature_flipped_on ${key}`);
    assert(
        json.type === 'is_feature_flipped_on' && !('error' in json),
        `Command failed: ${json.error}`);
    return !!json.value;
  },

  async isFeatureFlippedOff(key) {
    const json = await this._writeCommand(`is_feature_flipped_off ${key}`);
    assert(
        json.type === 'is_feature_flipped_off' && !('error' in json),
        `Command failed: ${json.error}`);
    return !!json.value;
  },

  async flipFeatureOn(key) {
    const json = await this._writeCommand(`flip_on_feature ${key}`);
    assert(
        json.type === 'flip_on_feature' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async flipFeatureOff(key) {
    const json = await this._writeCommand(`flip_off_feature ${key}`);
    assert(
        json.type === 'flip_off_feature' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async setSetting(key, value) {
    const json = await this._writeCommand(`set_setting ${key} ${value}`);
    assert(
        json.type === 'set_setting' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async getSetting(key) {
    const json = await this._writeCommand(`setting ${key}`);
    assert(
        json.type === 'setting' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value;
  },

  lastNotification() {
    return _lastNotification;
  },

  resetLastNotification() {
    _lastNotification.title = null;
    _lastNotification.message = null;
  },

  async settingsFileName() {
    const json = await this._writeCommand('settings_filename');
    assert(
        json.type === 'settings_filename' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value;
  },

  async languages() {
    const json = await this._writeCommand('languages');
    assert(
        json.type === 'languages' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value;
  },

  async servers() {
    const json = await this._writeCommand('servers');
    assert(
        json.type === 'servers' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value;
  },

  async guides() {
    const json = await this._writeCommand('guides');
    assert(
        json.type === 'guides' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value;
  },

  async screenCapture() {
    const json = await this._writeCommand('screen_capture');
    assert(
        json.type === 'screen_capture' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value;
  },

  async getDevices() {
    const json = await this._writeCommand('devices');
    assert(
      json.type === 'devices' && !('error' in json),
      `Command failed: ${json.error}`);
    return json.value;
  },

  async getPublicKey() {
    const json = await this._writeCommand('public_key');
    assert(
      json.type === 'public_key' && !('error' in json),
      `Command failed: ${json.error}`);
    return json.value;
  },

  async sendPushMessageDeviceDeleted(key) {
    const json =
        await this._writeCommand(`send_push_message_device_deleted ${key}`);
    assert(
        json.type === 'send_push_message_device_deleted' && !('error' in json),
        `Command failed: ${json.error}`);
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

  async _maybeRemoveExistingDevices() {
    const json = await this._writeCommand('devices');
    assert(
        json.type === 'devices' && !('error' in json),
        `Command failed: ${json.error}`);

    if (json.value.find(device => device.currentDevice)) {
      return;
    }

    const addJson = await this._writeCommand('reset_devices');
    assert(
        addJson.type === 'reset_devices' && !('error' in addJson),
        `Command failed: ${addJson.error}`);

    await this.waitForCondition(async () => {
      const json = await this._writeCommand('devices');
      assert(
          json.type === 'devices' && !('error' in json),
          `Command failed: ${json.error}`);
      return json.value.find(device => device.currentDevice);
    });
  },
};
