/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const { navBar, settingsScreen, initialScreen, telemetryScreen, generalElements, authScreen } = require('./elements.js');
const vpn = require('./helper.js');

describe('Devices', function() {

  describe('Device limit', function() {
    this.ctx.authenticationNeeded = true;

    it('Opens and closes the device list', async () => {
      await vpn.waitForElementAndClick(navBar.SETTINGS);
      await vpn.waitForElementAndClick(settingsScreen.MY_DEVICE);      
      await vpn.waitForElementAndClick(settingsScreen.BACK);

      await vpn.waitForElement(settingsScreen.MY_DEVICE);
      await vpn.waitForElementProperty(settingsScreen.MY_DEVICE, 'visible', 'true');
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
      await vpn.waitForElementAndClick(initialScreen.GET_STARTED);
      await vpn.waitForElement(authScreen.EMAIL_INPUT);
      await vpn.setElementProperty(authScreen.EMAIL_INPUT, 'text', 's', 'test@test');
      await vpn.waitForElement(authScreen.START_BUTTON);
      await vpn.waitForElementAndClick(authScreen.START_BUTTON);

      await vpn.waitForElement(authScreen.SIGNIN_PASS_INPUT);
      await vpn.setElementProperty(authScreen.SIGNIN_PASS_INPUT, 'text', 's', 'password');

      await vpn.waitForElementAndClick(authScreen.SIGNIN_BUTTON);

      // Wait for VPN client screen to move from spinning wheel to next screen
      await vpn.waitForElementProperty('VPN', 'userState', 'UserAuthenticated');
      await vpn.waitForElementAndClick(telemetryScreen.POST_AUTHENTICATION_BUTTON);

      await vpn.waitForElement(telemetryScreen.TELEMETRY_POLICY_BUTTON);
      await vpn.clickOnElement(telemetryScreen.TELEMETRY_POLICY_BUTTON);
      await vpn.waitForElement(settingsScreen.myDevicesView.BACK);

      await vpn.waitForElement(settingsScreen.myDevicesView.DEVICE_LIST);
      await vpn.waitForElementProperty(settingsScreen.myDevicesView.DEVICE_LIST, 'visible', 'true');


      await vpn.waitForElement(settingsScreen.myDevicesView.DEVICE_LIMIT_HEADER);
      await vpn.waitForElementProperty(settingsScreen.myDevicesView.DEVICE_LIMIT_HEADER, 'visible', 'true');

      // Let's remove a device
      await vpn.waitForElementAndClick('deviceListView/device-device_1/deviceLayout/deviceRemoveButton');
      await vpn.waitForElementAndClick(settingsScreen.myDevicesView.CONFIRM_REMOVAL_BUTTON);

      await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
      await vpn.waitForElementProperty(generalElements.CONTROLLER_TITLE, 'visible', 'true');
    });
  });

  describe('Device limit and push notification', function() {
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
      DELETEs: {},
    };

    it('Device limit', async () => {
      if (!(await vpn.isFeatureFlippedOn('inAppAuthentication'))) {
        await vpn.flipFeatureOn('inAppAuthentication');
      }

      // This method must be called when the client is on the "Get Started"
      // view.
      await vpn.waitForMainView();

      // Click on get started and wait for authenticating view
      await vpn.waitForElementAndClick(initialScreen.GET_STARTED);
      await vpn.waitForElement(authScreen.EMAIL_INPUT);
      await vpn.setElementProperty(authScreen.EMAIL_INPUT, 'text', 's', 'test@test');
      await vpn.waitForElement(authScreen.START_BUTTON);
      await vpn.waitForElementAndClick(authScreen.START_BUTTON);

      await vpn.waitForElement(authScreen.SIGNIN_PASS_INPUT);
      await vpn.setElementProperty(authScreen.SIGNIN_PASS_INPUT, 'text', 's', 'password');
      await vpn.waitForElementAndClick(authScreen.SIGNIN_BUTTON);

      // Wait for VPN client screen to move from spinning wheel to next screen
      await vpn.waitForElementProperty('VPN', 'userState', 'UserAuthenticated');
      await vpn.waitForElementAndClick(telemetryScreen.POST_AUTHENTICATION_BUTTON);

      await vpn.waitForElement(telemetryScreen.TELEMETRY_POLICY_BUTTON);
      await vpn.clickOnElement(telemetryScreen.TELEMETRY_POLICY_BUTTON);

      await vpn.waitForElement(settingsScreen.myDevicesView.BACK);
      await vpn.waitForElement(settingsScreen.myDevicesView.DEVICE_LIST);
      await vpn.waitForElementProperty(settingsScreen.myDevicesView.DEVICE_LIST, 'visible', 'true');

      await vpn.waitForElement(settingsScreen.myDevicesView.DEVICE_LIMIT_HEADER);
      await vpn.waitForElementProperty(settingsScreen.myDevicesView.DEVICE_LIMIT_HEADER, 'visible', 'true');

      const key = UserData.devices[0].pubkey;
      UserData.devices.splice(0, 1);
      await vpn.sendPushMessageDeviceDeleted(key);

      await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
      await vpn.waitForElementProperty(generalElements.CONTROLLER_TITLE, 'visible', 'true');
    });
  });

  describe('Device limit and race condition', function() {
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
          status: 404,
          body: {},
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
      await vpn.waitForElementAndClick(initialScreen.GET_STARTED);
      await vpn.waitForElement(authScreen.EMAIL_INPUT);
      await vpn.setElementProperty(authScreen.EMAIL_INPUT, 'text', 's', 'test@test');
      await vpn.waitForElement(authScreen.START_BUTTON);
      await vpn.waitForElementAndClick(authScreen.START_BUTTON);

      await vpn.wait()
      await vpn.waitForElement(authScreen.SIGNIN_PASS_INPUT);
      await vpn.setElementProperty(authScreen.SIGNIN_PASS_INPUT, 'text', 's', 'password');
      await vpn.waitForElementAndClick(authScreen.SIGNIN_BUTTON);

      // Wait for VPN client screen to move from spinning wheel to next screen
      await vpn.waitForElementProperty('VPN', 'userState', 'UserAuthenticated');
      await vpn.waitForElementAndClick(telemetryScreen.POST_AUTHENTICATION_BUTTON);

      await vpn.waitForElement(telemetryScreen.TELEMETRY_POLICY_BUTTON);
      await vpn.clickOnElement(telemetryScreen.TELEMETRY_POLICY_BUTTON);

      await vpn.waitForElement(settingsScreen.myDevicesView.BACK);
      await vpn.waitForElement(settingsScreen.myDevicesView.DEVICE_LIST);
      await vpn.waitForElementProperty(settingsScreen.myDevicesView.DEVICE_LIST, 'visible', 'true');

      await vpn.waitForElement(settingsScreen.myDevicesView.DEVICE_LIMIT_HEADER);
      await vpn.waitForElementProperty(settingsScreen.myDevicesView.DEVICE_LIMIT_HEADER, 'visible', 'true');

      // Let's remove a device
      const key = UserData.devices[0].pubkey;
      UserData.devices.splice(0, 1);

      // Let's remove a device by clicking the button
      await vpn.waitForElementAndClick('deviceListView/device-device_1/deviceLayout/deviceRemoveButton');
      await vpn.waitForElementAndClick(settingsScreen.myDevicesView.CONFIRM_REMOVAL_BUTTON);

      await vpn.waitForElement(generalElements.CONTROLLER_TITLE);
      await vpn.waitForElementProperty(generalElements.CONTROLLER_TITLE, 'visible', 'true');
    });
  });

  // TODO: test the device title X of Y
  // TODO: check the device entries in the list
});
