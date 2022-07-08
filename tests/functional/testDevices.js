/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const vpn = require('./helper.js');

describe('Devices', function() {
  describe('Device limit', function() {
    this.ctx.authenticationNeeded = true;

    it('Opens and closes the device list', async () => {
      await vpn.waitForElement('deviceListButton');
      await vpn.waitForElementProperty('deviceListButton', 'visible', 'true');
      await vpn.wait();

      await vpn.clickOnElement('deviceListButton');
      await vpn.wait();

      await vpn.waitForElement('deviceListBackButton');
      await vpn.waitForElementProperty(
          'deviceListBackButton', 'visible', 'true');
      await vpn.clickOnElement('deviceListBackButton');
      await vpn.wait();

      await vpn.waitForElement('deviceListButton');
      await vpn.waitForElementProperty('deviceListButton', 'visible', 'true');
    });
  });

  describe('Device limit', function() {
    const UserData = {
      avatar: '',
      display_name: 'Test test',
      email: 'test@mozilla.com',
      max_devices: 5,
      subscriptions: {vpn: {active: true}},
      devices: [
        {
          name: 'device_1',
          unique_id: 'device_1',
          pubkey: 'a',
          ipv4_address: '127.0.0.1',
          ipv6_address: '::1',
          created_at: new Date().toISOString()
        },
        {
          name: 'device_2',
          unique_id: 'device_2',
          pubkey: 'b',
          ipv4_address: '127.0.0.1',
          ipv6_address: '::1',
          created_at: new Date().toISOString()
        },
        {
          name: 'device_3',
          unique_id: 'device_3',
          pubkey: 'c',
          ipv4_address: '127.0.0.1',
          ipv6_address: '::1',
          created_at: new Date().toISOString()
        },
        {
          name: 'device_4',
          unique_id: 'device_4',
          pubkey: 'd',
          ipv4_address: '127.0.0.1',
          ipv6_address: '::1',
          created_at: new Date().toISOString()
        },
        {
          name: 'device_5',
          unique_id: 'device_5',
          pubkey: 'e',
          ipv4_address: '127.0.0.1',
          ipv6_address: '::1',
          created_at: new Date().toISOString()
        }
      ],
    };

    this.ctx.guardianOverrideEndpoints = {
      GETs: {
        '/api/v1/vpn/account': {status: 200, body: UserData},
      },
      POSTs: {
        '/api/v2/vpn/login/verify':
            {status: 200, body: {user: UserData, token: 'our-token'}},
        '/api/v1/vpn/device': {
          status: 201,
          callback: (req) => {
            UserData.devices[0].name = req.body.name;
            UserData.devices[0].pubkey = req.body.pubkey;
            UserData.devices[0].unique_id = req.body.unique_id;
          },
          body: {}
        },
      },
      DELETEs: {
        '/api/v1/vpn/device/': {
          match: 'startWith',
          status: 204,
          body: {},
          callback: (req) => {
            UserData.devices.splice(0, 1);
          }
        },
      },
    };

    it('Device limit', async () => {
      if (!(await vpn.isFeatureFlippedOn('inAppAuthentication'))) {
        await vpn.flipFeatureOn('inAppAuthentication');
      }

      // This method must be called when the client is on the "Get Started"
      // view.
      await vpn.waitForMainView();

      // Click on get started and wait for authenticating view
      await vpn.clickOnElement('getStarted');
      await vpn.waitForElement('authStart-textInput');
      await vpn.setElementProperty(
          'authStart-textInput', 'text', 's', 'test@test');
      await vpn.waitForElement('authStart-button');
      await vpn.clickOnElement('authStart-button');

      await vpn.waitForElement('authSignIn-passwordInput');
      await vpn.setElementProperty(
          'authSignIn-passwordInput', 'text', 's', 'password');

      await vpn.clickOnElement('authSignIn-button');

      // Wait for VPN client screen to move from spinning wheel to next screen
      await vpn.waitForElementProperty('VPN', 'userState', 'UserAuthenticated');
      await vpn.waitForElement('postAuthenticationButton');

      await vpn.clickOnElement('postAuthenticationButton');
      await vpn.wait();

      await vpn.waitForElement('telemetryPolicyButton');
      await vpn.clickOnElement('telemetryPolicyButton');
      await vpn.waitForElement('deviceListBackButton');
      await vpn.waitForElement('deviceLimitPanel');
      await vpn.waitForElementProperty('deviceLimitPanel', 'visible', 'true');

      await vpn.waitForElement('deviceListBackButton');
      await vpn.waitForElementProperty(
          'deviceListBackButton', 'visible', 'true');
      await vpn.waitForElement('deviceList/device-device_1');
      await vpn.waitForElementProperty(
          'deviceList/device-device_1', 'visible', 'true');

      // Let's remove a device
      await vpn.waitForElement(
          'deviceList/device-device_1/deviceLayout/deviceRemoveButton');
      await vpn.waitForElementProperty(
          'deviceList/device-device_1/deviceLayout/deviceRemoveButton',
          'visible', 'true');
      await vpn.clickOnElement(
          'deviceList/device-device_1/deviceLayout/deviceRemoveButton');

      await vpn.waitForElement('confirmRemoveDeviceButton');
      await vpn.waitForElementProperty(
          'confirmRemoveDeviceButton', 'visible', 'true');
      await vpn.clickOnElement('confirmRemoveDeviceButton');

      await vpn.waitForElement('controllerTitle');
      await vpn.waitForElementProperty('controllerTitle', 'visible', 'true');
    });
  });

  // TODO: test the device title X of Y
  // TODO: check the device entries in the list
  // TODO: remove a device
  // TODO: max number of devices
});
