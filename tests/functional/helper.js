/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import assert, { strictEqual } from 'assert';
import { URL } from 'node:url';
import { request } from 'http';
import { screenHome, screenInitialize, screenPostAuthentication, global, screenTelemetry, screenAuthenticationInApp } from './queries.js';
import { addonServer } from './servers/addon.js';
import { InspectorWebsocketClient } from '@mozillavpn/inspector';


/** @type{ InspectorWebsocketClient } */
let client;
/** @type{any} Inspector Exposed Objects */
let InspectorObjects; 


let _lastNotification = {
  title: null,
  message: null,
};

let _lastAddonLoadingCompleted = false;

export function runningOnWasm() {
  return process.env['WASM'];
}
/**
 * Returns a promise that resolves after {ms} millisecs
 * @param {number} ms - Millisecs to wait 
 * @returns 
 */
export async function waitFor(ms){
  return new Promise(r => {
    setTimeout(r,ms);
  })
}

/**
 * Attach the Helper Module to an InspectorInstance. 
 * @param { InspectorWebsocketClient } inspector - A running InspectorConnection
 */
export async function setInspectorClient(inspector) {
  if(!inspector.isConnected.value){
    throw new Error("Disconnected Client was passed!!");
  }
  client = inspector;
  InspectorObjects = inspector.qWebChannel.value.objects;
}
export async function activateViaToggle() {
  await waitForQueryAndClick(
    screenHome.CONTROLLER_TOGGLE.visible().prop(
      'state', 'stateOff'));
}
export async function activate(awaitConnectionOkay = false) {
  const json = await _writeCommand('activate');
  assert(
    json.type === 'activate' && !('error' in json),
    `Command failed: ${json.error}`);

  if (awaitConnectionOkay) {
    await awaitSuccessfulConnection();
  }
}
export
  // Waits for VPN connection to be active and healthy.
  async function awaitSuccessfulConnection() {
  await waitForCondition(async () => {
    let title = await getQueryProperty(
      screenHome.CONTROLLER_TITLE.visible(), 'text');
    let unsettled = await getMozillaProperty(
      'Mozilla.VPN', 'VPNConnectionHealth', 'unsettled');
    return (title == 'VPN is on') && (unsettled == 'false');
  });
}
export async function deactivate() {
  const json = await _writeCommand('deactivate');
  assert(
    json.type === 'deactivate' && !('error' in json),
    `Command failed: ${json.error}`);
}
export async function hardReset() {
  const json = await _writeCommand('hard_reset');
  assert(
    json.type === 'hard_reset' && !('error' in json),
    `Command failed: ${json.error}`);
}
export async function reset() {
  const json = await _writeCommand('reset');
  assert(
    json.type === 'reset' && !('error' in json),
    `Command failed: ${json.error}`);
}
export async function waitForInitialView() {
  await waitForQuery(screenInitialize.GET_HELP_LINK.visible());
  assert(await query(screenInitialize.SIGN_UP_BUTTON.visible()));
  assert(await query(
    screenInitialize.ALREADY_A_SUBSCRIBER_LINK.visible()));
}
export async function forceHeartbeatFailure() {
  const json = await _writeCommand('force_heartbeat_failure');
  assert(
    json.type === 'force_heartbeat_failure' && !('error' in json),
    `Command failed: ${json.error}`);
}
export async function forceUnsecuredNetworkAlert() {
  const json = await _writeCommand('force_unsecured_network');
  assert(
    json.type === 'force_unsecured_network' && !('error' in json),
    `Command failed: ${json.error}`);
}
export async function forceServerUnavailable() {
  const json = await _writeCommand('force_server_unavailable');
  assert(
    json.type === 'force_server_unavailable' && !('error' in json),
    `Command failed: ${json.error}`);
}
export async function forceSubscriptionManagementReauth() {
  const json = await _writeCommand(
    'force_subscription_management_reauthentication');
  assert(
    json.type === 'force_subscription_management_reauthentication' &&
    !('error' in json),
    `Command failed: ${json.error}`);
}
export async function forceCaptivePortalDetection() {
  const json = await _writeCommand('force_captive_portal_detection');
  assert(
    json.type === 'force_captive_portal_detection' && !('error' in json),
    `Command failed: ${json.error}`);
}
export async function quit() {
  _writeCommand('quit');
  client.close();
  client = null;
}
export async function copyToClipboard(text) {
  const json = await _writeCommand(
    `copy_to_clipboard ${encodeURIComponent(text)}`);
  assert(
    !('type' in json) ||
    (json.type === 'copy_to_clipboard' && !('error' in json)),
    `Command failed: ${json.error}`);
}
export async function  query(id ){
  const json = await _writeCommand(`query ${encodeURIComponent(id)}`);
  assert(
    json.type === 'query' && !('error' in json),
    `Command failed: ${json.error}`);
  return json.value || false;
}

export async function waitForQuery(id) {
  console.log(`Wait For Query -> ${id}`)
  await waitForCondition(async () => {
    return await query(id);
  });
  process.stdout.moveCursor(0, -1) // up one line
  process.stdout.clearLine(1) // from cursor to end
}
export async function clickOnQuery(id) {
  assert(await query(id), 'Clicking on an non-existing element?!?');
  const json = await _writeCommand(`click ${encodeURIComponent(id)}`);
  assert(
    json.type === 'click' && !('error' in json),
    `Command failed: ${json.error}`);
}
export
  // This is used when hitting the "Reset and Quit" button. We expect empty
  // JSON object back, so clickOnQuery would never return the `click` in json,
  // and command would fail.
  async function clickOnQueryAndAcceptAnyResults(id) {
  assert(await query(id), 'Element does not exist.');
  const command = `click ${encodeURIComponent(id)}`;

  const json = await _writeCommand(`click ${encodeURIComponent(id)}`);
}
export async function waitForQueryAndClick(id) {
  await waitForQuery(id);
  await clickOnQuery(id);
}
export async function waitForQueryAndWriteInTextField(id, value) {
  await waitForQuery(id);
  await setQueryProperty(id, 'text', value);
}
export async function clickOnNotification() {
  const json = await _writeCommand('click_notification');
  assert(
    json.type === 'click_notification' && !('error' in json),
    `Command failed: ${json.error}`);
}
export async function scrollToQuery(view, id) {
  assert(await query(view), 'Scrolling on an non-existing view?!?');
  assert(await query(id), 'Requesting an non-existing element?!?');

  const contentHeight = parseInt(await getQueryProperty(view, 'contentHeight'));
  const height = parseInt(await getQueryProperty(view, 'height'));
  let maxScroll = (contentHeight > height) ? contentHeight - height : 0;
  let elementY = parseInt(await getQueryProperty(id, 'y'));

  let contentY = elementY - (height / 2);
  if (contentY < 0) contentY = 0;
  if (contentY > maxScroll) contentY = maxScroll;

  await setQueryProperty(view, 'contentY', contentY);
  await wait();
}
export async function getMozillaProperty(namespace, id, property) {
  const json = await _writeCommand(`property ${encodeURIComponent(namespace)} ${encodeURIComponent(id)} ${encodeURIComponent(property)}`);
  assert(
    json.type === 'property' && !('error' in json),
    `Command failed: ${json.error}`);
  return json.value || '';
}
export async function getQueryProperty(id, property) {
  assert(
    await query(id),
    `Property checks must be done on existing elements: ${id}.${property}`);
  const json = await _writeCommand(`query_property ${encodeURIComponent(id)} ${encodeURIComponent(property)}`);
  assert(
    json.type === 'query_property' && !('error' in json),
    `Command failed: ${json.error}`);
  return json.value || '';
}
export async function setMozillaProperty(namespace, id, property, value) {
  const json = await _writeCommand(`set_property ${encodeURIComponent(namespace)} ${encodeURIComponent(id)} ${encodeURIComponent(property)} ${encodeURIComponent(value)}`);
  assert(
    json.type === 'set_property' && !('error' in json),
    `Command failed: ${json.error}`);
}
export async function setQueryProperty(id, property, value) {
  assert(
    await query(id),
    'Property checks must be done on existing elements');
  const json = await _writeCommand(
    `set_query_property ${encodeURIComponent(id)} ${encodeURIComponent(property)} ${encodeURIComponent(value)}`);
  assert(
    json.type === 'set_query_property' && !('error' in json),
    `Command failed: ${json.error}`);
}
export async function waitForMozillaProperty(namespace, id, property, value) {
  try {
    return waitForCondition(async () => {
      const real = await getMozillaProperty(namespace, id, property);
      return real === value;
    });
  } catch (e) {
    const real = await getMozillaProperty(namespace, id, property);
    throw new Error(`Timeout for waitForMozillaProperty - property: ${property} - value: ${real} - expected: ${value}`);
  }
}
export async function setGleanAutomationHeader() {
  const json = await _writeCommand('set_glean_source_tags automation');
  assert(
    json.type === 'set_glean_source_tags' && !('error' in json),
    `Command failed: ${json.error}`);
  return json.value || null;
}
export async function gleanTestGetValue(metricCategory, metricName, ping) {
  const metric = InspectorObjects.Glean[metricCategory][metricName]
  const result = await metric.testGetValue(ping);
  return result;
}
export async function gleanTestReset() {
  const json = await _writeCommand(`glean_test_reset`);
  assert(
    json.type === 'glean_test_reset' && !('error' in json),
    `Command failed: ${json.error}`);
  return json.value || null;
}
export async function getLastUrl() {
  return await getMozillaProperty(
    'Mozilla.Shared', 'MZUrlOpener', 'lastUrl');
}
export async function waitForCondition(condition, waitTimeInMilliSecs = 500) {
  // If the condition takes longer than 15 seconds, give up.
  console.log(`Wait For Condition -> ${condition.toString()}`)
  let active = true;
  let timeout = setTimeout(() => { active = false; }, 15000);
  while (true) {
    if (await condition()) {
      clearTimeout(timeout);
      process.stdout.moveCursor(0, -1) // up one line
      process.stdout.clearLine(1) // from cursor to end
      return;
    }
    // Asserting here produces a more useful backtrace for diagnosing tests.
    assert(active, "Condition timed out");
    await new Promise(resolve => setTimeout(resolve, waitTimeInMilliSecs));
  }
}
export function wait(waitTimeInMilliSecs = 500) {
  return new Promise(resolve => setTimeout(resolve, waitTimeInMilliSecs));
}
  // TODO - The expected staging urls are hardcoded, we may want to
  // move these hardcoded urls out if testing in alternate environments.
export  async function authenticateInBrowser(clickOnPostAuthenticate, acceptTelemetry, wasm) {
  if (await isFeatureFlippedOn('inAppAuthentication')) {
    await flipFeatureOff('inAppAuthentication');
  }

  // This method must be called when the client is on the "Get Started"view.
  await waitForInitialView();
  await setMozillaProperty(
    'Mozilla.Shared', 'MZUrlOpener', 'lastUrl', '');

  // Click on get started and wait for authenticating view
  await clickOnQuery(screenInitialize.SIGN_UP_BUTTON.visible());

  if (!wasm) {
    await waitForCondition(async () => {
      const url = await getLastUrl();
      return url.includes('/api/v2/vpn/login');
    });
    await wait();

    // We don't really want to go through the authentication flow because we
    // are mocking everything. So this next chunk of code manually
    // makes a call to the DesktopAuthenticationListener to mock
    // a successful authentication in browser.
    const url = await getLastUrl();
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
      const req = request(options, res => { });
      req.on('close', resolve);
      req.on('error', error => {
        throw new Error(
          `Unable to connect to ${urlObj.hostname} to complete the
              auth. ${error.name}, ${error.message}, ${error.stack}`);
      });
      req.end();
    });
  }

  // Wait for VPN client screen to move from spinning wheel to next screen
  await waitForMozillaProperty(
    'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

  if (clickOnPostAuthenticate) {
    await waitForQuery(screenPostAuthentication.BUTTON.visible());
    await waitForQuery(global.SCREEN_LOADER.ready());
    await clickOnQuery(
      screenPostAuthentication.BUTTON.visible());
    await wait();
  }
  if (acceptTelemetry) {
    await waitForQuery(global.SCREEN_LOADER.ready());
    await waitForQuery(screenTelemetry.BUTTON.visible());

    await waitForQuery(global.SCREEN_LOADER.ready());
    await clickOnQuery(screenTelemetry.BUTTON.visible());

    await waitForQuery(global.SCREEN_LOADER.ready());
    await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
  }
}
export async function authenticateInApp(clickOnPostAuthenticate = false, acceptTelemetry = false) {
  // This method must be called when the client is on the "Get Started" view.
  await waitForInitialView();

  // Click on get started and wait for authenticating view
  await clickOnQuery(screenInitialize.SIGN_UP_BUTTON.visible());
  await waitForQuery(
    screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
  await setQueryProperty(
    screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
    'text', 'test@test.com');
  await waitForQueryAndClick(
    screenAuthenticationInApp.AUTH_START_BUTTON.visible()
      .enabled());

  await waitForQuery(
    screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible());
  await setQueryProperty(
    screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT.visible(),
    'text', 'password');

  await clickOnQuery(
    screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
      .enabled());

  // Wait for VPN client screen to move from spinning wheel to next screen
  await waitForMozillaProperty(
    'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

  if (clickOnPostAuthenticate) {
    await waitForQuery(screenPostAuthentication.BUTTON.visible());
    await clickOnQuery(
      screenPostAuthentication.BUTTON.visible());
    await wait();
  }
  if (acceptTelemetry) {
    await waitForQuery(screenTelemetry.BUTTON.visible());
    await clickOnQuery(screenTelemetry.BUTTON.visible());
    await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
  }
}

export async function isFeatureFlippedOn(key) {
  const json = await _writeCommand(
    `is_feature_flipped_on ${encodeURIComponent(key)}`);
  assert(
    json.type === 'is_feature_flipped_on' && !('error' in json),
    `Command failed: ${json.error}`);
  return !!json.value;
}
export async function isFeatureFlippedOff(key) {
  const json = await _writeCommand(
    `is_feature_flipped_off ${encodeURIComponent(key)}`);
  assert(
    json.type === 'is_feature_flipped_off' && !('error' in json),
    `Command failed: ${json.error}`);
  return !!json.value;
}
export async function flipFeatureOn(key) {
  const json = await _writeCommand(`flip_on_feature ${encodeURIComponent(key)}`);
  assert(
    json.type === 'flip_on_feature' && !('error' in json),
    `Command failed: ${json.error}`);
}
export async function flipFeatureOff(key) {
  const json = await _writeCommand(`flip_off_feature ${encodeURIComponent(key)}`);
  assert(
    json.type === 'flip_off_feature' && !('error' in json),
    `Command failed: ${json.error}`);
}
export async function setSetting(key, value) {
  const json = await _writeCommand(
    `set_setting ${encodeURIComponent(key)} ${encodeURIComponent(value)}`);
  assert(
    json.type === 'set_setting' && !('error' in json),
    `Command failed: ${json.error}`);
}
export async function getSetting(key) {
  const json = await _writeCommand(`setting ${encodeURIComponent(key)}`);
  assert(
    json.type === 'setting' && !('error' in json),
    `Command failed: ${json.error}`);
  return json.value;
}
export function lastNotification() {
  return _lastNotification;
}
export function resetLastNotification() {
  _lastNotification.title = null;
  _lastNotification.message = null;
}
export async function settingsFileName() {
  const json = await _writeCommand('settings_filename');
  assert(
    json.type === 'settings_filename' && !('error' in json),
    `Command failed: ${json.error}`);
  return json.value;
}

export async function servers() {
  const json = await _writeCommand('servers');
  assert(
    json.type === 'servers' && !('error' in json),
    `Command failed: ${json.error}`);
  return json.value;
}
export async function messages() {
  const json = await _writeCommand('messages');
  assert(
    json.type === 'messages' && !('error' in json),
    `Command failed: ${json.error}`);
  return json.value;
}

export async function resetAddons(addonPath) {
  await waitForMozillaProperty(
    'Mozilla.Shared', 'MZAddonManager', 'loadCompleted', 'true');

  _lastAddonLoadingCompleted = false;

  await setSetting(
    'addonCustomServerAddress', `${addonServer.url}/${addonPath}/`);
  await setSetting('addonCustomServer', 'true');

  const json = await _writeCommand('reset_addons');
  assert(
    json.type === 'reset_addons' && !('error' in json),
    `Command failed: ${json.error}`);

  await waitForCondition(() => _lastAddonLoadingCompleted);
}
export async function fetchAddons(addonPath) {
  await waitForMozillaProperty(
    'Mozilla.Shared', 'MZAddonManager', 'loadCompleted', 'true');

  _lastAddonLoadingCompleted = false;

  await setSetting(
    'addonCustomServerAddress', `${addonServer.url}/${addonPath}/`);
  await setSetting('addonCustomServer', 'true');

  const json = await _writeCommand('fetch_addons');
  assert(
    json.type === 'fetch_addons' && !('error' in json),
    `Command failed: ${json.error}`);

  await waitForCondition(() => _lastAddonLoadingCompleted);
}
export async function setVersionOverride(versionOverride) {
  const json = await _writeCommand(
    `set_version_override ${encodeURIComponent(versionOverride)}`);
  assert(
    json.type === 'set_version_override' && !('error' in json),
    `Command failed: ${json.error}`);
  return json.value;
}
export async function forceConnectionStabilityStatus(connectionStabilityStatus) {
  const json = await _writeCommand(`force_connection_health ${encodeURIComponent(connectionStabilityStatus)}`);
  assert(
    json.type === 'force_connection_health' && !('error' in json),
    `Command failed: ${json.error}`);
  return json.value;
}
// By default gets the last recorded event.
// `offset` can be used to change that, it adds the offset from the last.
// So, for example, if we want the next to last event we give it an `offset` of 1.
export async function getOneEventOfType({
    eventCategory, eventName,
    // When expectedEventCount is provided it will be asserted on.
    // When it's not provided the last event will be tested.
    expectedEventCount
  }, offset = 0) {
  let events;
  await waitForCondition(async () => {
    events = await gleanTestGetValue(eventCategory, eventName, "main");
    return events.length > 0;
  });

  let computedEventCount = expectedEventCount;
  if (!computedEventCount) {
    computedEventCount = events.length;
  } else {
    strictEqual(events.length, computedEventCount);
  }

  return events[computedEventCount - (1 + offset)];
}
export async function testLastInteractionEvent(options) {
  const defaults = { eventCategory: "interaction", action: "select" };
  const optionsWithDefaults = { ...defaults, ...options };

  const lastEvent = await getOneEventOfType(optionsWithDefaults);

  const { screen } = optionsWithDefaults;

  strictEqual(screen, lastEvent.extra.screen);
}
export async function _writeCommand(command) {
  const response = await InspectorObjects.inspector_cli.recv(`${command}`);
  return JSON.parse(response);
}
