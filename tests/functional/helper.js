/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const {URL} = require('node:url');
const http = require('http')
const queries = require('./queries.js');
const addonServer = require('./servers/addon.js');

let client;

let waitReadCallback = null;

let _lastNotification = {
  title: null,
  message: null,
};

let _lastAddonLoadingCompleted = false;

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

            if (json.type === 'addon_load_completed') {
              _lastAddonLoadingCompleted = true;
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

  async activateViaToggle() {
    await this.waitForQueryAndClick(
        queries.screenHome.CONTROLLER_TOGGLE.visible().prop(
            'state', 'stateOff'));
  },

  async activate(awaitConnectionOkay = false) {
    const json = await this._writeCommand('activate');
    assert(
        json.type === 'activate' && !('error' in json),
        `Command failed: ${json.error}`);

    if (awaitConnectionOkay) {
      await this.waitForCondition(async () => {
        let title = await this.getQueryProperty(
            queries.screenHome.CONTROLLER_TITLE.visible(), 'text');
        let unsettled =
            await this.getVPNProperty('VPNConnectionHealth', 'unsettled');
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

  async waitForInitialView() {
    await this.waitForQuery(queries.screenInitialize.GET_HELP_LINK.visible());
    assert(await this.query(queries.screenInitialize.SIGN_UP_BUTTON.visible()));
    assert(await this.query(
        queries.screenInitialize.ALREADY_A_SUBSCRIBER_LINK.visible()));
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
        json.type === 'force_subscription_management_reauthentication' &&
            !('error' in json),
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

  async copyToClipboard(text) {
    const json = await this._writeCommand(
        `copy_to_clipboard ${encodeURIComponent(text)}`);
    assert(
      !('type' in json) || (json.type === 'copy_to_clipboard' && !('error' in json)),
      `Command failed: ${json.error}`);
  },

  async query(id) {
    const json = await this._writeCommand(`query ${encodeURIComponent(id)}`);
    assert(
        json.type === 'query' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value || false;
  },

  async waitForQuery(id) {
    return this.waitForCondition(async () => {
      return await this.query(id);
    });
  },

  async clickOnQuery(id) {
    assert(await this.query(id), 'Clicking on an non-existing element?!?');
    const json = await this._writeCommand(`click ${encodeURIComponent(id)}`);
    assert(
        json.type === 'click' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async waitForQueryAndClick(id) {
    await this.waitForQuery(id);
    await this.clickOnQuery(id);
  },

  async waitForQueryAndWriteInTextField(id, value){
    await this.waitForQuery(id)
    await this.setQueryProperty(id, 'text', value)
  },

  async clickOnNotification() {
    const json = await this._writeCommand('click_notification');
    assert(
        json.type === 'click_notification' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async scrollToQuery(view, id) {
    assert(await this.query(view), 'Scrolling on an non-existing view?!?');
    assert(await this.query(id), 'Requesting an non-existing element?!?');

    const contentHeight =
        parseInt(await this.getQueryProperty(view, 'contentHeight'));
    const height = parseInt(await this.getQueryProperty(view, 'height'));
    let maxScroll = (contentHeight > height) ? contentHeight - height : 0;
    let elementY = parseInt(await this.getQueryProperty(id, 'y'));

    let contentY = elementY - (height / 2);
    if (contentY < 0) contentY = 0;
    if (contentY > maxScroll) contentY = maxScroll;

    await this.setQueryProperty(view, 'contentY', contentY);
    await this.wait();
  },

  async getVPNProperty(id, property) {
    const json = await this._writeCommand(
        `property ${encodeURIComponent(id)} ${encodeURIComponent(property)}`);
    assert(
        json.type === 'property' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value || '';
  },

  async getQueryProperty(id, property) {
    assert(
        await this.query(id),
        'Property checks must be done on existing elements');
    const json = await this._writeCommand(`query_property ${
        encodeURIComponent(id)} ${encodeURIComponent(property)}`);
    assert(
        json.type === 'query_property' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value || '';
  },

  async setVPNProperty(id, property, value) {
    const json =
        await this._writeCommand(`set_property ${encodeURIComponent(id)} ${
            encodeURIComponent(property)} ${encodeURIComponent(value)}`);
    assert(
        json.type === 'set_property' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async setQueryProperty(id, property, value) {
    assert(
        await this.query(id),
        'Property checks must be done on existing elements');
    const json = await this._writeCommand(
        `set_query_property ${encodeURIComponent(id)} ${
            encodeURIComponent(property)} ${encodeURIComponent(value)}`);
    assert(
        json.type === 'set_query_property' && !('error' in json),
        `Command failed: ${json.error}`);
  },

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
  },

  async setGleanAutomationHeader() {
    const json = await this._writeCommand('set_glean_source_tags automation');
    assert(
        json.type === 'set_glean_source_tags' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value || null;
  },

  async getLastUrl() {
    return await this.getVPNProperty('VPNUrlOpener', 'lastUrl');
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
    if (await this.isFeatureFlippedOn('inAppAuthentication')) {
      await this.flipFeatureOff('inAppAuthentication');
    }

    // This method must be called when the client is on the "Get Started"view.
    await this.waitForInitialView();
    await this.setVPNProperty('VPNUrlOpener', 'lastUrl', '');

    // Click on get started and wait for authenticating view
    await this.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());

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
              `Unable to connect to ${urlObj.hostname} to complete the
              auth`);
        });
        req.end();
      });
    }

    // Wait for VPN client screen to move from spinning wheel to next screen
    await this.waitForVPNProperty('VPN', 'userState', 'UserAuthenticated');
    await this.waitForQuery(queries.screenPostAuthentication.BUTTON.visible());

    if (clickOnPostAuthenticate) {
      await this.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await this.clickOnQuery(
          queries.screenPostAuthentication.BUTTON.visible());
      await this.wait();
    }
    if (acceptTelemetry) {
      await this.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await this.waitForQuery(queries.screenTelemetry.BUTTON.visible());

      await this.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await this.clickOnQuery(queries.screenTelemetry.BUTTON.visible());

      await this.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await this.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    }
  },

  async authenticateInApp(
      clickOnPostAuthenticate = false, acceptTelemetry = false) {
    // This method must be called when the client is on the "Get Started" view.
    await this.waitForInitialView();

    // Click on get started and wait for authenticating view
    await this.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
    await this.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
    await this.setQueryProperty(
        queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
        'text', 'test@test.com');
    await this.waitForQueryAndClick(
        queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
            .enabled());

    await this.waitForQuery(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible());
    await this.setQueryProperty(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible(),
        'text', 'password');

    await this.clickOnQuery(
        queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
            .enabled());

    // Wait for VPN client screen to move from spinning wheel to next screen
    await this.waitForVPNProperty('VPN', 'userState', 'UserAuthenticated');
    await this.waitForQuery(queries.screenPostAuthentication.BUTTON.visible());

    if (clickOnPostAuthenticate) {
      await this.clickOnQuery(
          queries.screenPostAuthentication.BUTTON.visible());
      await this.wait();
    }
    if (acceptTelemetry) {
      await this.waitForQuery(queries.screenTelemetry.BUTTON.visible());
      await this.clickOnQuery(queries.screenTelemetry.BUTTON.visible());
      await this.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    }
  },

  async logout() {
    const json = await this._writeCommand('logout');
    assert(
        json.type === 'logout' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async isFeatureFlippedOn(key) {
    const json = await this._writeCommand(
        `is_feature_flipped_on ${encodeURIComponent(key)}`);
    assert(
        json.type === 'is_feature_flipped_on' && !('error' in json),
        `Command failed: ${json.error}`);
    return !!json.value;
  },

  async isFeatureFlippedOff(key) {
    const json = await this._writeCommand(
        `is_feature_flipped_off ${encodeURIComponent(key)}`);
    assert(
        json.type === 'is_feature_flipped_off' && !('error' in json),
        `Command failed: ${json.error}`);
    return !!json.value;
  },

  async flipFeatureOn(key) {
    const json =
        await this._writeCommand(`flip_on_feature ${encodeURIComponent(key)}`);
    assert(
        json.type === 'flip_on_feature' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async flipFeatureOff(key) {
    const json =
        await this._writeCommand(`flip_off_feature ${encodeURIComponent(key)}`);
    assert(
        json.type === 'flip_off_feature' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async setSetting(key, value) {
    const json = await this._writeCommand(
        `set_setting ${encodeURIComponent(key)} ${encodeURIComponent(value)}`);
    assert(
        json.type === 'set_setting' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async getSetting(key) {
    const json = await this._writeCommand(`setting ${encodeURIComponent(key)}`);
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
    const json = await this._writeCommand(
        `send_push_message_device_deleted ${encodeURIComponent(key)}`);
    assert(
        json.type === 'send_push_message_device_deleted' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value;
  },

  async resetAddons(addonPath) {
    await this.waitForVPNProperty('VPNAddonManager', 'loadCompleted', 'true');

    _lastAddonLoadingCompleted = false;

    await this.setSetting(
        'addonCustomServerAddress', `${addonServer.url}/${addonPath}/`);
    await this.setSetting('addonCustomServer', 'true');

    const json = await this._writeCommand('reset_addons');
    assert(
        json.type === 'reset_addons' && !('error' in json),
        `Command failed: ${json.error}`);

    await this.waitForCondition(() => _lastAddonLoadingCompleted);
  },

  async fetchAddons(addonPath) {
    await this.waitForVPNProperty('VPNAddonManager', 'loadCompleted', 'true');

    _lastAddonLoadingCompleted = false;

    await this.setSetting(
        'addonCustomServerAddress', `${addonServer.url}/${addonPath}/`);
    await this.setSetting('addonCustomServer', 'true');

    const json = await this._writeCommand('fetch_addons');
    assert(
        json.type === 'fetch_addons' && !('error' in json),
        `Command failed: ${json.error}`);

    await this.waitForCondition(() => _lastAddonLoadingCompleted);
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
