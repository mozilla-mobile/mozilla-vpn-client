/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const queries = require('./queries.js');
const vpn = require('./helper.js');
const guardianEndpoints = require('./servers/guardian_endpoints.js')

describe('Devices', function() {
  describe('Devices tests', function() {
    this.ctx.authenticationNeeded = true;

    beforeEach(async () => {
      await vpn.waitForQueryAndClick(queries.navBar.SETTINGS.visible());
      await vpn.waitForQueryAndClick(queries.screenSettings.MY_DEVICES.visible());
      await vpn.waitForQuery(queries.screenSettings.STACKVIEW.ready());
      await vpn.waitForQuery(queries.screenSettings.myDevicesView.DEVICES_VIEW.visible());
    });

    it('Checking the devices help sheet', async () => {
      await vpn.waitForQueryAndClick(queries.screenSettings.myDevicesView.HELP_BUTTON.visible());
      await vpn.waitForQuery(queries.screenSettings.myDevicesView.HELP_SHEET.opened());
      await vpn.waitForQueryAndClick(queries.screenSettings.myDevicesView.HELP_SHEET_LEARN_MORE_BUTTON.visible());
      await vpn.waitForCondition(async () => {
          const url = await vpn.getLastUrl();
          return url === 'https://support.mozilla.org/kb/how-add-devices-your-mozilla-vpn-subscription';
      });
      await vpn.waitForQueryAndClick(queries.screenSettings.myDevicesView.HELP_SHEET_CLOSE_BUTTON.visible());
      await vpn.waitForQuery(queries.screenSettings.myDevicesView.HELP_SHEET.closed());
    });

    describe('Checking devices screen telemetry', function () {
      // No Glean on WASM.
      if(vpn.runningOnWasm()) {
        return;
      }

      const devicesTelemetryScreenId = "my_devices"

      it('Checking devices screen impression telemetry', async () => {
        const myDevicesScreenEvents = await vpn.waitForGleanValue("impression", "myDevicesScreen", "main");
        assert.equal(myDevicesScreenEvents.length, 1);
        const myDevicesScreenEventsExtras = myDevicesScreenEvents[0].extra;
        assert.equal(devicesTelemetryScreenId, myDevicesScreenEventsExtras.screen);
      });

      it('Checking devices screen help sheet telemetry', async () => {
        const devicesHelpSheetTelemetryScreenId = "my_devices_info"

        await vpn.waitForQueryAndClick(queries.screenSettings.myDevicesView.HELP_BUTTON.visible());

        const helpTooltipSelectedEvents = await vpn.waitForGleanValue("interaction", "helpTooltipSelected", "main");
        assert.equal(helpTooltipSelectedEvents.length, 1);
        const helpTooltipSelectedEventsExtras = helpTooltipSelectedEvents[0].extra;
        assert.equal(devicesTelemetryScreenId, helpTooltipSelectedEventsExtras.screen);

        await vpn.waitForQuery(queries.screenSettings.myDevicesView.HELP_SHEET.opened());

        const myDevicesInfoScreenEvents = await vpn.waitForGleanValue("impression", "myDevicesInfoScreen", "main");
        assert.equal(myDevicesInfoScreenEvents.length, 1);
        const myDevicesInfoScreenEventsExtras = myDevicesInfoScreenEvents[0].extra;
        assert.equal(devicesHelpSheetTelemetryScreenId, myDevicesInfoScreenEventsExtras.screen);

        await vpn.waitForQueryAndClick(queries.screenSettings.appPreferencesView.dnsSettingsView.HELP_SHEET_LEARN_MORE_BUTTON.visible());

        const learnMoreSelectedEvents = await vpn.waitForGleanValue("interaction", "learnMoreSelected", "main");
        assert.equal(learnMoreSelectedEvents.length, 1);
        const learnMoreSelectedEventsExtras = learnMoreSelectedEvents[0].extra;
        assert.equal(devicesHelpSheetTelemetryScreenId, learnMoreSelectedEventsExtras.screen);
      });
    });
  });

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

      //skip onboarding. normally done in helper::authenticateInApp(), but this test logs in manually
      await vpn.skipOnboarding();

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
      await vpn.waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

      await vpn.waitForQuery(
          queries.screenSettings.myDevicesView.DEVICE_LIST.visible());

      //Wait for swipe delegate delay
      await vpn.wait(1000);

      // Let's remove a device
      await vpn.waitForQueryAndClick(
          queries.screenSettings.myDevicesView.REMOVE_DEVICE_BUTTON.visible());
      await vpn.wait(); // Wait for popup animation

      await vpn.waitForQueryAndClick(queries.screenSettings.myDevicesView
                                         .CONFIRM_REMOVAL_BUTTON.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

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
      await vpn.waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

      await vpn.waitForQuery(
        queries.screenSettings.myDevicesView.DEVICE_LIST.visible());
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

      //skip onboarding. normally done in helper::authenticateInApp(), but this test logs in manually
      await vpn.skipOnboarding();

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
      await vpn.waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userState', 'UserAuthenticated');

      await vpn.waitForQuery(
          queries.screenSettings.myDevicesView.DEVICE_LIST.visible());

      // Let's remove a device
      const key = UserData.devices[0].pubkey;
      UserData.devices.splice(0, 1);

      //Wait for swipe delegate delay
      await vpn.wait(1000);

      // Let's remove a device by clicking the button
      await vpn.waitForQueryAndClick(
          queries.screenSettings.myDevicesView.REMOVE_DEVICE_BUTTON.visible());
      await vpn.wait(); // Wait for popup animation

      await vpn.waitForQueryAndClick(queries.screenSettings.myDevicesView
                                         .CONFIRM_REMOVAL_BUTTON.visible());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());

      await vpn.waitForQuery(queries.global.SCREEN_LOADER.ready());
      await vpn.waitForQuery(queries.screenHome.CONTROLLER_TITLE.visible());
    });
  });

  // TODO: test the device title X of Y
  // TODO: check the device entries in the list
});
