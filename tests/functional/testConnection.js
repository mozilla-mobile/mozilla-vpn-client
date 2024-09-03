/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');
const queries = require('./queries.js');
const setup = require('./setupVpn.js');

describe('Connectivity', function() {
  this.ctx.authenticationNeeded = true;

  it('check the ui', async () => {
    await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    assert.equal(
        await vpn.getQueryProperty(
            queries.screenHome.CONTROLLER_TITLE, 'text'), 'VPN is off');

    await vpn.waitForQuery(queries.screenHome.CONTROLLER_SUBTITLE.visible());
    assert.equal(
        await vpn.getQueryProperty(
            queries.screenHome.CONTROLLER_SUBTITLE, 'text'),
        'Turn on to protect your privacy');

    await vpn.waitForQuery(queries.screenHome.CONTROLLER_TOGGLE.visible());
  });

  it('Connect to VPN', async () => {
    await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());

    await vpn.setSetting('connectionChangeNotification', 'true');
    await vpn.clickOnQuery(queries.screenHome.CONTROLLER_TOGGLE.visible());

    await vpn.waitForCondition(async () => {
      let connectingMsg = await vpn.getQueryProperty(
          queries.screenHome.CONTROLLER_TITLE, 'text');
      return connectingMsg === 'Connecting…';
    });

    assert.equal(
        await vpn.getQueryProperty(
            queries.screenHome.CONTROLLER_SUBTITLE, 'text'),
        'Masking connection and location');

    await vpn.waitForCondition(async () => {
      return await vpn.getQueryProperty(
                 queries.screenHome.CONTROLLER_TITLE, 'text') == 'VPN is on';
    });

    assert((await vpn.getQueryProperty(
                queries.screenHome.SECURE_AND_PRIVATE_SUBTITLE, 'text'))
               .startsWith('Secure and private '));

    await vpn.waitForCondition(() => {
      return vpn.lastNotification().title === 'VPN Connected';
    });

    assert.equal(vpn.lastNotification().title, 'VPN Connected');
    assert(vpn.lastNotification().message.startsWith('Connected through '));
  });

  it('Automatically connect when startAtBoot is active', async () => {
    await vpn.setSetting('startAtBoot', true);
    await vpn.quit();
    await setup.startAndConnect();
    await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    await vpn.waitForCondition(async () => {
      return await vpn.getQueryProperty(
                 queries.screenHome.CONTROLLER_TITLE, 'text') == 'VPN is on';
    });
  });

  it('Disconnecting and disconnected', async () => {
    await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());

    await vpn.setSetting('connectionChangeNotification', 'true');
    await vpn.activate();
    await vpn.waitForCondition(async () => {
      return await vpn.getQueryProperty(
                 queries.screenHome.CONTROLLER_TITLE, 'text') == 'VPN is on';
    });
    await vpn.deactivate();

    // No test for disconnecting because often it's too fast to be tracked.

    await vpn.waitForCondition(async () => {
      return await vpn.getQueryProperty(
                 queries.screenHome.CONTROLLER_TITLE, 'text') === 'VPN is off';
    });

    assert.equal(
        await vpn.getQueryProperty(
            queries.screenHome.CONTROLLER_SUBTITLE, 'text'),
        'Turn on to protect your privacy');

    await vpn.waitForCondition(() => {
      return vpn.lastNotification().title === 'VPN Disconnected';
    });

    assert.equal(vpn.lastNotification().title, 'VPN Disconnected');
    assert(vpn.lastNotification().message.startsWith('Disconnected from '));
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
    await vpn.setSetting(
        'serverData',
        '{"enter_city_name":"","enter_country_code":"","exit_city_name":"Melbourne","exit_country_code":"au"}');
    await vpn.setSetting('connectionChangeNotification', 'true');

    // Let's activate the VPN clicking on the toggle.
    await vpn.activateViaToggle();

    // The controller starts with the data we have set in the settings.
    const currentServer = await vpn.getMozillaProperty(
        'Mozilla.VPN', 'VPNController', 'currentServerString');
    assert.equal(currentServer, 'au-Melbourne--');

    await vpn.setSetting(
        'serverData',
        '{"enter_city_name":"","enter_country_code":"","exit_city_name":"Sydney","exit_country_code":"au"}');

    // After changing the settings, the controller has still the previous
    // values. VPNCurrentServer is updated, instead.
    assert.equal(
        currentServer,
        await vpn.getMozillaProperty(
            'Mozilla.VPN', 'VPNController', 'currentServerString'));
    assert.equal(
        await vpn.getMozillaProperty(
            'Mozilla.VPN', 'VPNCurrentServer', 'exitCityName'),
        'Sydney');

    await vpn.waitForCondition(async () => {
      let connectingMsg = await vpn.getQueryProperty(
          queries.screenHome.CONTROLLER_TITLE, 'text');
      return connectingMsg === 'Connecting…';
    });

    // At the end of the activation, controller and VPNCurrentServer are again
    // in sync.
    assert.equal(
        currentServer,
        await vpn.getMozillaProperty(
            'Mozilla.VPN', 'VPNController', 'currentServerString'));
    assert.equal(
        await vpn.getMozillaProperty(
            'Mozilla.VPN', 'VPNCurrentServer', 'exitCityName'),
        'Sydney');
  });
});
