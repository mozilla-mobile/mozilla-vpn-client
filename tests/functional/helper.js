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
  runningOnWasm() {
    return process.env['WASM'];
  },

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
      await this.awaitSuccessfulConnection();
    }
  },

  // Waits for VPN connection to be active and healthy.
  async awaitSuccessfulConnection() {
    await this.waitForCondition(async () => {
      let title = await this.getQueryProperty(
          queries.screenHome.CONTROLLER_TITLE.visible(), 'text');
      let unsettled = await this.getMozillaProperty(
          'Mozilla.VPN', 'VPNConnectionHealth', 'unsettled');
      return (title == 'VPN is on') && (unsettled == 'false');
    });
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
        !('type' in json) ||
            (json.type === 'copy_to_clipboard' && !('error' in json)),
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

  // This is used when hitting the "Reset and Quit" button. We expect empty
  // JSON object back, so clickOnQuery would never return the `click` in json,
  // and command would fail.
  async clickOnQueryAndAcceptAnyResults(id) {
    assert(await this.query(id), 'Element does not exist.');
    const command = `click ${encodeURIComponent(id)}`;

    const json = await this._writeCommand(`click ${encodeURIComponent(id)}`);
  },

  async waitForQueryAndClick(id) {
    await this.waitForQuery(id);
    await this.clickOnQuery(id);
  },

  async waitForQueryAndWriteInTextField(id, value) {
    await this.waitForQuery(id);
    await this.setQueryProperty(id, 'text', value);
  },

  async clickOnNotification() {
    const json = await this._writeCommand('click_notification');
    assert(
        json.type === 'click_notification' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  // Scroll an item into the center of a view
  async scrollToQuery(view, id) {
    assert(await this.query(view), 'Scrolling a non-existing view?!?');
    assert(await this.query(id), 'Scrolling to a non-existing element?!?');
    const json = await this._writeCommand(`scrollview ${encodeURIComponent(view)} ${encodeURIComponent(id)}`);
    assert(
        json.type === 'scrollview' && !('error' in json),
        `Command failed: ${json.error}`);

    // Generally there is some kind of animation or rendering delay here.
    await this.wait();
  },

  async navServerList(countryId, cityId) {
    // TODO: Should assert that a server list is open.

    const view = queries.screenHome.serverListView.COUNTRY_VIEW;
    await this.waitForQuery(view.ready());

    // Scroll to the country
    await this.waitForQuery(countryId.visible());
    await this.scrollToQuery(view, countryId);

    // If the city list is closed, open it.
    if (await this.getQueryProperty(countryId, "cityListVisible") !== "true") {
      await this.waitForQueryAndClick(countryId.visible());
      await this.waitForQuery(countryId.visible().prop("cityListVisible", true));
    }
    await this.waitForQuery(countryId.ready());

    // If a city is given, scroll it into view.
    if (cityId !== undefined) {
      await this.waitForQuery(cityId.visible());
      await this.scrollToQuery(view, cityId);
    }
  },

  async getMozillaProperty(namespace, id, property) {
    const json =
        await this._writeCommand(`property ${encodeURIComponent(namespace)} ${
            encodeURIComponent(id)} ${encodeURIComponent(property)}`);
    assert(
        json.type === 'property' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value || '';
  },

  async getQueryProperty(id, property) {
    assert(
        await this.query(id),
        `Property checks must be done on existing elements: ${id}.${property}`);
    const json = await this._writeCommand(`query_property ${
        encodeURIComponent(id)} ${encodeURIComponent(property)}`);
    assert(
        json.type === 'query_property' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value || '';
  },

  async setMozillaProperty(namespace, id, property, value) {
    const json = await this._writeCommand(`set_property ${
        encodeURIComponent(namespace)} ${encodeURIComponent(id)} ${
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

  async waitForMozillaProperty(namespace, id, property, value) {
    try {
      return this.waitForCondition(async () => {
        const real = await this.getMozillaProperty(namespace, id, property);
        return real === value;
      });
    } catch (e) {
      const real = await this.getMozillaProperty(namespace, id, property);
      throw new Error(`Timeout for waitForMozillaProperty - property: ${
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

  async gleanTestGetValue(metricCategory, metricName, ping) {
    const json = await this._writeCommand(`glean_test_get_value ${metricCategory} ${metricName} ${ping}`);
    assert(
        json.type === 'glean_test_get_value' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value || null;
  },

  async gleanTestReset() {
    const json = await this._writeCommand(`glean_test_reset`);
    assert(
        json.type === 'glean_test_reset' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value || null;
  },

  async getLastUrl() {
    return await this.getMozillaProperty(
        'Mozilla.Shared', 'MZUrlOpener', 'lastUrl');
  },

  async waitForCondition(condition, waitTimeInMilliSecs = 500) {
    // If the condition takes longer than 15 seconds, give up.
    let active = true;
    let timeout = setTimeout(() => { active = false }, 15000);
    while (true) {
      if (await condition()) {
        clearTimeout(timeout)
        return;
      }
      // Asserting here produces a more useful backtrace for diagnosing tests.
      assert(active, "Condition timed out");
      await new Promise(resolve => setTimeout(resolve, waitTimeInMilliSecs));
    }
  },

  wait(waitTimeInMilliSecs = 500) {
    return new Promise(resolve => setTimeout(resolve, waitTimeInMilliSecs));
  },

  async mockInBrowserAuthentication() {
    await this.waitForCondition(async () => {
      const url = await this.getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });
    await this.wait();

    // We don't really want to go through the authentication flow because we
    // are mocking everything. So this next chunk of code manually
    // makes a call to the DesktopAuthenticationListener to mock
    // a successful authentication in browser.
    const url = await this.getLastUrl();
    const authListenerPort = (new URL(url)).searchParams.get('port');
    const options = {
      // We hardcode 127.0.0.1 to match listening on QHostAddress:LocalHost
      // and hardcoded in guardian's vpnClientPixelImageAuthUrl
      hostname: '127.0.0.1',
      port: parseInt(authListenerPort, 10),
      path: '/?code=the_code',
      method: 'GET',
    };

    await new Promise(resolve => {
      const req = http.request(options, res => {});
      req.on('close', resolve);
      req.on('error', error => {
        throw new Error(
            `Unable to connect to ${urlObj.hostname} to complete the
            auth. ${error.name}, ${error.message}, ${error.stack}`);
      });
      req.end();
    });
  },

  // TODO - The expected staging urls are hardcoded, we may want to
  // move these hardcoded urls out if testing in alternate environments.
  async authenticateInBrowser(wasm, skipOnboarding = true) {
    if (skipOnboarding) {
      await this.skipOnboarding();
    }

    if (await this.isFeatureFlippedOn('inAppAuthentication')) {
      await this.flipFeatureOff('inAppAuthentication');
    }

    // This method must be called when the client is on the "Get Started"view.
    await this.waitForInitialView();
    await this.setMozillaProperty(
        'Mozilla.Shared', 'MZUrlOpener', 'lastUrl', '');

    // Click on get started and wait for authenticating view
    await this.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());

    if (!wasm) {
      await mockInBrowserAuthentication();
    }

    // Wait for VPN client screen to move from spinning wheel to next screen
    await this.waitForMozillaProperty(
        'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');
  },

  async authenticateInApp(skipOnboarding = true) {
    if (skipOnboarding) {
      await this.skipOnboarding();
    }

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
    await this.waitForMozillaProperty(
        'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');
  },

  async skipOnboarding() {
    await this.setSetting('onboardingCompleted', 'true');
  },

  async completeTelemetryPolicy() {
      await this.waitForQuery(queries.screenTelemetry.BUTTON.visible());
      await this.clickOnQuery(queries.screenTelemetry.BUTTON.visible());
      await this.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
  },

  async completeOnboarding() {
    await this.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    switch(await this.getSetting('onboardingStep')) {
    case 0:
      await this.waitForQueryAndClick(queries.screenOnboarding.DATA_NEXT_BUTTON.visible());
      await this.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    case 1:
      await this.waitForQueryAndClick(queries.screenOnboarding.PRIVACY_NEXT_BUTTON.visible());
      await this.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    case 2:
      await this.waitForQueryAndClick(queries.screenOnboarding.DEVICES_NEXT_BUTTON.visible());
      await this.waitForQuery(queries.screenOnboarding.STEP_NAV_STACK_VIEW.ready());
    case 3:
      await this.waitForQueryAndClick(queries.screenOnboarding.START_NEXT_BUTTON.visible());
      await this.waitForQuery(queries.screenHome.SCREEN.visible());
      assert.equal(await this.getSetting('onboardingCompleted'), true);
    default:
      break;
    }
 },

  async logout() {
    const json = await this._writeCommand('logout');
    assert(
        json.type === 'logout' && !('error' in json),
        `Command failed: ${json.error}`);
  },

  async isFeatureEnabled(key) {
    const json = await this._writeCommand(
      `is_feature_enabled ${encodeURIComponent(key)}`);
    assert(
      json.type === 'is_feature_enabled' && !('error' in json),
      `Command failed: ${json.error}`);
    return !!json.value;
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

  async messages() {
    const json = await this._writeCommand('messages');
    assert(
        json.type === 'messages' && !('error' in json),
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

  async resetAddons(addonPath) {
    await this.waitForMozillaProperty(
        'Mozilla.Shared', 'MZAddonManager', 'loadCompleted', 'true');

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
    await this.waitForMozillaProperty(
        'Mozilla.Shared', 'MZAddonManager', 'loadCompleted', 'true');

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

  async setVersionOverride(versionOverride) {
    const json = await this._writeCommand(
        `set_version_override ${encodeURIComponent(versionOverride)}`);
    assert(
        json.type === 'set_version_override' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value;
  },

  async forceConnectionStabilityStatus(connectionStabilityStatus) {
    const json = await this._writeCommand(`force_connection_health ${
        encodeURIComponent(connectionStabilityStatus)}`);
    assert(
        json.type === 'force_connection_health' && !('error' in json),
        `Command failed: ${json.error}`);
    return json.value;
  },

  // By default gets the last recorded event.
  // `offset` can be used to change that, it adds the offset from the last.
  // So, for example, if we want the next to last event we give it an `offset` of 1.
  async getOneEventOfType({
    eventCategory,
    eventName,
    // When expectedEventCount is provided it will be asserted on.
    // When it's not provided the last event will be tested.
    expectedEventCount
  }, offset = 0) {
    let events;
    await this.waitForCondition(async () => {
      events = await this.gleanTestGetValue(eventCategory, eventName, "main");
      return events.length > 0;
    });

    let computedEventCount = expectedEventCount;
    if (!computedEventCount) {
      computedEventCount = events.length;
    } else {
      assert.strictEqual(events.length, computedEventCount);
    }

    return events[computedEventCount - (1 + offset)];
  },

  async testLastInteractionEvent (options) {
    const defaults = { eventCategory: "interaction", action: "select" };
    const optionsWithDefaults = { ...defaults, ...options };

    const lastEvent = await this.getOneEventOfType(optionsWithDefaults);

    const { screen } = optionsWithDefaults;

    assert.strictEqual(screen, lastEvent.extra.screen);
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
