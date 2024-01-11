/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import assert, { equal } from 'assert';
import { waitForQuery, getQueryProperty, setSetting, clickOnQuery, waitForCondition, lastNotification, activate, deactivate, activateViaToggle, getMozillaProperty } from './helper.js';
import { screenHome } from './queries.js';

describe('Connectivity', function() {
  this.ctx.authenticationNeeded = true;

  it('check the ui', async () => {
    await waitForQuery(screenHome.CONTROLLER_TITLE.visible());
    equal(
        await getQueryProperty(
            screenHome.CONTROLLER_TITLE, 'text'), 'VPN is off');

    await waitForQuery(screenHome.CONTROLLER_SUBTITLE.visible());
    equal(
        await getQueryProperty(
            screenHome.CONTROLLER_SUBTITLE, 'text'),
        'Turn on to protect your privacy');

    await waitForQuery(screenHome.CONTROLLER_TOGGLE.visible());
  });

  it('Connect to VPN', async () => {
    await waitForQuery(screenHome.CONTROLLER_TITLE.visible());

    await setSetting('connectionChangeNotification', 'true');
    await clickOnQuery(screenHome.CONTROLLER_TOGGLE.visible());

    await waitForCondition(async () => {
      let connectingMsg = await getQueryProperty(
          screenHome.CONTROLLER_TITLE, 'text');
      return connectingMsg === 'Connecting…';
    });

    equal(
        await getQueryProperty(
            screenHome.CONTROLLER_SUBTITLE, 'text'),
        'Masking connection and location');

    await waitForCondition(async () => {
      return await getQueryProperty(
                 screenHome.CONTROLLER_TITLE, 'text') == 'VPN is on';
    });

    assert((await getQueryProperty(
                screenHome.SECURE_AND_PRIVATE_SUBTITLE, 'text'))
               .startsWith('Secure and private '));

    await waitForCondition(() => {
      return lastNotification().title === 'VPN Connected';
    });

    equal(lastNotification().title, 'VPN Connected');
    assert(lastNotification().message.startsWith('Connected through '));
  });

  it('Disconnecting and disconnected', async () => {
    await waitForQuery(screenHome.CONTROLLER_TITLE.visible());

    await setSetting('connectionChangeNotification', 'true');
    await activate();
    await waitForCondition(async () => {
      return await getQueryProperty(
                 screenHome.CONTROLLER_TITLE, 'text') == 'VPN is on';
    });
    await deactivate();

    // No test for disconnecting because often it's too fast to be tracked.

    await waitForCondition(async () => {
      return await getQueryProperty(
                 screenHome.CONTROLLER_TITLE, 'text') === 'VPN is off';
    });

    equal(
        await getQueryProperty(
            screenHome.CONTROLLER_SUBTITLE, 'text'),
        'Turn on to protect your privacy');

    await waitForCondition(() => {
      return lastNotification().title === 'VPN Disconnected';
    });

    equal(lastNotification().title, 'VPN Disconnected');
    assert(lastNotification().message.startsWith('Disconnected from '));
  });

  it('Connect to VPN - race condition', async () => {
    // In this test, we want to see that the  VPNCurrentServer can go
    // out-of-sync with the VPNController.currentServerString, which, in C++,
    // means: `MozillaVPN::instance()->serverData()` does not match the
    // controller ->currentServer()`.
    //
    // This can happen if settings change while the controller completes an
    // activation or a server switch.
    //
    // If we keep the two server data objects in sync (or if we have just one
    // of them), the controller can end up using not what the user asked for
    // but something that has changed in the meantime.
    await setSetting(
        'serverData',
        '{"enter_city_name":"","enter_country_code":"","exit_city_name":"Melbourne","exit_country_code":"au"}');
    await setSetting('connectionChangeNotification', 'true');

    // Let's activate the VPN clicking on the toggle.
    await activateViaToggle();

    // The controller starts with the data we have set in the settings.
    const currentServer = await getMozillaProperty(
        'Mozilla.VPN', 'VPNController', 'currentServerString');
    equal(currentServer, 'au-Melbourne--');

    await setSetting(
        'serverData',
        '{"enter_city_name":"","enter_country_code":"","exit_city_name":"Sydney","exit_country_code":"au"}');

    // After changing the settings, the controller has still the previous
    // values. VPNCurrentServer is updated, instead.
    equal(
        currentServer,
        await getMozillaProperty(
            'Mozilla.VPN', 'VPNController', 'currentServerString'));
    equal(
        await getMozillaProperty(
            'Mozilla.VPN', 'VPNCurrentServer', 'exitCityName'),
        'Sydney');

    await waitForCondition(async () => {
      let connectingMsg = await getQueryProperty(
          screenHome.CONTROLLER_TITLE, 'text');
      return connectingMsg === 'Connecting…';
    });

    // At the end of the activation, controller and VPNCurrentServer are again
    // in sync.
    equal(
        currentServer,
        await getMozillaProperty(
            'Mozilla.VPN', 'VPNController', 'currentServerString'));
    equal(
        await getMozillaProperty(
            'Mozilla.VPN', 'VPNCurrentServer', 'exitCityName'),
        'Sydney');
  });
});
