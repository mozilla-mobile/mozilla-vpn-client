/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const queries = require('./queries.js');
const vpn = require('./helper.js');
const guardianEndpoints = require('./servers/guardian_endpoints.js')

describe('Devices', function() {
  describe('Device limit', function() {
    this.ctx.authenticationNeeded = true;

    it('Opens and closes the device list', async () => {
      await vpn.waitForQueryAndClick(queries.navBar.SETTINGS);
      await vpn.waitForQueryAndClick(queries.screenSettings.MY_DEVICES);
      await vpn.waitForQueryAndClick(queries.screenSettings.BACK);

      await vpn.waitForQuery(queries.screenSettings.MY_DEVICES.visible());
    });
  });

  describe('Device limit', function() {
    const UserData = {
      avatar: '',
      display_name: 'Test',
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
        '/api/v1/vpn/account':
            {status: 200, requiredHeaders: ['Authorization'], body: UserData},
      },
      POSTs: {
        '/api/v2/vpn/login/verify': {
          status: 200,
          bodyValidator: guardianEndpoints.validators.guardianLoginVerify,
          body: {user: UserData, token: 'our-token'}
        },

        '/api/v1/vpn/device': {
          status: 201,
          requiredHeaders: ['Authorization'],
          bodyValidator: guardianEndpoints.validators.guardianDevice,
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
          requiredHeaders: ['Authorization'],
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
      await vpn.waitForInitialView();

      // Click on get started and wait for authenticating view
      await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_SIGNIN_PASSWORD_INPUT.visible());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT
              .visible(),
          'text', 'password');

      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
              .enabled());

      // Wait for VPN client screen to move from spinning wheel to next screen
      await vpn.waitForVPNProperty('VPN', 'userState', 'UserAuthenticated');

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(
          queries.screenPostAuthentication.BUTTON.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(queries.screenTelemetry.BUTTON.visible());

      await vpn.waitForQuery(
          queries.screenSettings.myDevicesView.BACK.visible());
      await vpn.waitForQuery(
          queries.screenSettings.myDevicesView.DEVICE_LIST.visible());

      await vpn.waitForQuery(
          queries.screenSettings.myDevicesView.DEVICE_LIMIT_HEADER.visible());

      // Let's remove a device
      await vpn.waitForQueryAndClick(
          queries.screenSettings.myDevicesView.REMOVE_DEVICE_BUTTON.visible());
      await vpn.waitForQueryAndClick(queries.screenSettings.myDevicesView
                                         .CONFIRM_REMOVAL_BUTTON.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    });
  });

  describe('Device limit and push notification', function() {
    const UserData = {
      avatar: '',
      display_name: 'Test',
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
        '/api/v1/vpn/account':
            {status: 200, requiredHeaders: ['Authorization'], body: UserData},
      },
      POSTs: {
        '/api/v2/vpn/login/verify': {
          status: 200,
          bodyValidator: guardianEndpoints.validators.guardianLoginVerify,
          body: {user: UserData, token: 'our-token'}
        },

        '/api/v1/vpn/device': {
          status: 201,
          requiredHeaders: ['Authorization'],
          bodyValidator: guardianEndpoints.validators.guardianDevice,
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
      await vpn.waitForInitialView();

      // Click on get started and wait for authenticating view
      await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_SIGNIN_PASSWORD_INPUT.visible());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT
              .visible(),
          'text', 'password');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible()
              .enabled());

      // Wait for VPN client screen to move from spinning wheel to next screen
      await vpn.waitForVPNProperty('VPN', 'userState', 'UserAuthenticated');

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(
          queries.screenPostAuthentication.BUTTON.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(queries.screenTelemetry.BUTTON.visible());

      await vpn.waitForQuery(
          queries.screenSettings.myDevicesView.BACK.visible());
      await vpn.waitForQuery(
          queries.screenSettings.myDevicesView.DEVICE_LIST.visible());

      await vpn.waitForQuery(
          queries.screenSettings.myDevicesView.DEVICE_LIMIT_HEADER.visible());

      const key = UserData.devices[0].pubkey;
      UserData.devices.splice(0, 1);
      await vpn.sendPushMessageDeviceDeleted(key);

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    });
  });

  describe('Device limit and race condition', function() {
    const UserData = {
      avatar: '',
      display_name: 'Test',
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
        '/api/v1/vpn/account':
            {status: 200, requiredHeaders: ['Authorization'], body: UserData},
      },
      POSTs: {
        '/api/v2/vpn/login/verify': {
          status: 200,
          bodyValidator: guardianEndpoints.validators.guardianLoginVerify,
          body: {user: UserData, token: 'our-token'}
        },

        '/api/v1/vpn/device': {
          status: 201,
          requiredHeaders: ['Authorization'],
          bodyValidator: guardianEndpoints.validators.guardianDevice,
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
          requiredHeaders: ['Authorization'],
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
      await vpn.waitForInitialView();

      // Click on get started and wait for authenticating view
      await vpn.clickOnQuery(queries.screenInitialize.SIGN_UP_BUTTON.visible());
      await vpn.waitForQuery(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_START_TEXT_INPUT.visible(),
          'text', 'test@test.com');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_START_BUTTON.visible()
              .enabled());

      await vpn.waitForQuery(queries.screenAuthenticationInApp
                                 .AUTH_SIGNIN_PASSWORD_INPUT.visible());
      await vpn.setQueryProperty(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_PASSWORD_INPUT
              .visible(),
          'text', 'password');
      await vpn.waitForQueryAndClick(
          queries.screenAuthenticationInApp.AUTH_SIGNIN_BUTTON.visible());

      // Wait for VPN client screen to move from spinning wheel to next screen
      await vpn.waitForVPNProperty('VPN', 'userState', 'UserAuthenticated');

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(
          queries.screenPostAuthentication.BUTTON.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQueryAndClick(queries.screenTelemetry.BUTTON.visible());

      await vpn.waitForQuery(
          queries.screenSettings.myDevicesView.BACK.visible());
      await vpn.waitForQuery(
          queries.screenSettings.myDevicesView.DEVICE_LIST.visible());

      await vpn.waitForQuery(
          queries.screenSettings.myDevicesView.DEVICE_LIMIT_HEADER.visible());

      // Let's remove a device
      const key = UserData.devices[0].pubkey;
      UserData.devices.splice(0, 1);

      // Let's remove a device by clicking the button
      await vpn.waitForQueryAndClick(
          queries.screenSettings.myDevicesView.REMOVE_DEVICE_BUTTON.visible());
      await vpn.waitForQueryAndClick(queries.screenSettings.myDevicesView
                                         .CONFIRM_REMOVAL_BUTTON.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    });
  });

  // TODO: test the device title X of Y
  // TODO: check the device entries in the list
});
