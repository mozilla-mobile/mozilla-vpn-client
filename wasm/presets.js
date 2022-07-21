/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const DeviceLimitUserData = {
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

const MVPNPresets = [
  {
    name: 'Main view',
    callback: async function() {
      await controller.waitForMainView();

      await controller.waitForElementProperty(
          'initialStackView', 'busy', 'false');
      await controller.clickOnElement('getStarted');

      await controller.waitForElementProperty(
          'VPN', 'userState', 'UserAuthenticated');
      await controller.wait();

      await controller.waitForElementProperty('MainStackView', 'busy', 'false');
      await controller.waitForElement('postAuthenticationButton');
      await controller.clickOnElement('postAuthenticationButton');

      await controller.wait();
      await controller.waitForElementProperty('MainStackView', 'busy', 'false');
      await controller.waitForElement('telemetryPolicyButton');
      await controller.clickOnElement('telemetryPolicyButton');

      await controller.wait();
      await controller.waitForElementProperty('MainStackView', 'busy', 'false');
      await controller.waitForElement('controllerTitle');
    }
  },

  {
    name: 'Mobile onboarding',
    callback: async function() {
      await controller.flipFeatureOn('mobileOnboarding');
    }
  },

  {
    name: 'Authentication in app',
    extraInfo: 'Username: wasm@wasm.wasm - Password: wasm - TOTP: 123456',
    callback: async function() {
      await controller.flipFeatureOn('inAppAuthentication');

      await controller.waitForMainView();

      await controller.wait();
      await controller.waitForElementProperty(
          'initialStackView', 'busy', 'false');
      await controller.clickOnElement('getStarted');
    },

    fxaOverrideEndpoints: {
      GETs: {},
      POSTs: {
        '/v1/account/login': {
          callback: (req, that) => {
            if (req.body.email === 'wasm@wasm.wasm' &&
                req.body.authPW ===
                    '8c021c875ea5e86ed5b6229a0b1b5a3e7c94ebc68f397d858897156041fe6638') {
              that.status = 200;
              that.body = {
                sessionToken: 'sesionToken',
                verified: false,
                verificationMethod: 'totp-2fa'
              };
            } else {
              that.status = 300;
              that.body = {
                errno: 103,  // Incorrect password
              };
            }
          },
        },
        '/v1/session/verify/totp': {
          status: 200,
          callback: (req, that) => {
            that.body = {success: req.body.code === '123456'};
          },
        },
      },
      DELETEs: {},
    }
  },

  {
    name: 'Authentication in app (unblock code)',
    callback: async function() {
      await controller.flipFeatureOn('inAppAuthentication');

      await controller.waitForMainView();

      await controller.wait();
      await controller.waitForElementProperty(
          'initialStackView', 'busy', 'false');
      await controller.clickOnElement('getStarted');
    },

    fxaOverrideEndpoints: {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          status: 400,
          body: {
            errno: 107,
            validation: {keys: ['unblockCode']},
          }
        },
      },
      DELETEs: {},
    }
  },

  {
    name: 'Account creation in app',
    extraInfo: 'Email code: 123456',
    callback: async function() {
      await controller.flipFeatureOn('inAppAccountCreate');

      await controller.waitForMainView();

      await controller.wait();
      await controller.waitForElementProperty(
          'initialStackView', 'busy', 'false');
      await controller.clickOnElement('getStarted');
    },

    fxaOverrideEndpoints: {
      GETs: {},
      POSTs: {
        '/v1/account/status': {status: 200, body: {exists: false}},
        '/v1/account/create': {
          status: 200,
          body: {
            sessionToken: 'sessionToken',
            'verified': false,
            verificationMethod: 'email-otp'
          }
        },
        '/v1/session/verify_code': {
          callback: (req, that) => {
            if (req.body.code === '123456') {
              that.status = 200;
              that.body = {};
            } else {
              that.status = 400;
              that.body = {errno: 107, validation: {keys: ['code']}};
            }
          },
        },
      },
      DELETEs: {},
    }
  },

  {
    name: 'Device limit reached',
    callback: async function() {
      await controller.waitForMainView();

      await controller.wait();
      await controller.waitForElementProperty(
          'initialStackView', 'busy', 'false');
      await controller.clickOnElement('getStarted');

      await controller.waitForElementProperty(
          'VPN', 'userState', 'UserAuthenticated');
      await controller.wait();

      await controller.waitForElementProperty('MainStackView', 'busy', 'false');
      await controller.waitForElement('postAuthenticationButton');
      await controller.clickOnElement('postAuthenticationButton');

      await controller.wait();
      await controller.waitForElementProperty('MainStackView', 'busy', 'false');
      await controller.waitForElement('telemetryPolicyButton');
      await controller.clickOnElement('telemetryPolicyButton');
    },

    guardianOverrideEndpoints: {
      GETs: {
        '/api/v1/vpn/account': {status: 200, body: DeviceLimitUserData},
      },
      POSTs: {
        '/api/v2/vpn/login/verify': {
          status: 200,
          body: {user: DeviceLimitUserData, token: 'our-token'}
        },
        '/api/v1/vpn/device': {
          status: 201,
          callback: (req) => {
            DeviceLimitUserData.devices[0].name = req.body.name;
            DeviceLimitUserData.devices[0].pubkey = req.body.pubkey;
            DeviceLimitUserData.devices[0].unique_id = req.body.unique_id;
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
            DeviceLimitUserData.devices.splice(0, 1);
          }
        },
      },
    }
  },

  {
    name: 'Update recommended',
    callback: async function() {
      await controller.waitForMainView();

      await controller.wait();
      await controller.waitForElementProperty(
          'initialStackView', 'busy', 'false');
      await controller.clickOnElement('getStarted');

      await controller.waitForElementProperty(
          'VPN', 'userState', 'UserAuthenticated');
      await controller.wait();

      await controller.waitForElementProperty('MainStackView', 'busy', 'false');
      await controller.waitForElement('postAuthenticationButton');
      await controller.clickOnElement('postAuthenticationButton');

      await controller.wait();
      await controller.waitForElementProperty('MainStackView', 'busy', 'false');
      await controller.waitForElement('telemetryPolicyButton');
      await controller.clickOnElement('telemetryPolicyButton');

      await controller.wait();
      await controller.waitForElementProperty('MainStackView', 'busy', 'false');
      await controller.waitForElement('controllerTitle');

      await controller.forceUpdateCheck('2.0.0');
    },

    guardianOverrideEndpoints: {
      GETs: {
        '/api/v1/vpn/versions': {
          status: 200,
          body: {
            dummy: {
              latest: {
                version: '123.0.0',
                released_on: '2022-07-22',
                message: 'The best version ever!'
              },
              minimum: {
                version: '1.0.0',
                released_on: '2022-07-22',
                message: 'The best version ever!'
              }
            }
          }
        },
      },
    }
  },

  {
    name: 'Update required',
    callback: async function() {
      await controller.waitForMainView();

      await controller.wait();
      await controller.waitForElementProperty(
          'initialStackView', 'busy', 'false');
      await controller.clickOnElement('getStarted');

      await controller.waitForElementProperty(
          'VPN', 'userState', 'UserAuthenticated');
      await controller.wait();

      await controller.waitForElementProperty('MainStackView', 'busy', 'false');
      await controller.waitForElement('postAuthenticationButton');
      await controller.clickOnElement('postAuthenticationButton');

      await controller.wait();
      await controller.waitForElementProperty('MainStackView', 'busy', 'false');
      await controller.waitForElement('telemetryPolicyButton');
      await controller.clickOnElement('telemetryPolicyButton');

      await controller.wait();
      await controller.waitForElementProperty('MainStackView', 'busy', 'false');
      await controller.waitForElement('controllerTitle');

      await controller.forceUpdateCheck('2.0.0');
    },

    guardianOverrideEndpoints: {
      GETs: {
        '/api/v1/vpn/versions': {
          status: 200,
          body: {
            dummy: {
              latest: {
                version: '123.0.0',
                released_on: '2022-07-22',
                message: 'The best version ever!'
              },
              minimum: {
                version: '123.0.0',
                released_on: '2022-07-22',
                message: 'The best version ever!'
              }
            }
          }
        },
      },
    }
  },

  // TODO:
  // - More error code for auth:
  //   * unblock code error
  //   * account already exists
  //   * email cannot be used to login
  //   * email type not supported
  //   * invalid email address
  //   * invalid unblock code
  //   * too many requests
  //   * server unavaiable
  //   * connection timeout
  //   * unknown account
  // - subscription needed
  // - subscription blocked
  // - billing not available
  // - subscription not validated
  // - backend failure
  // - unstable connection
  // - no signal
  // - alerts
];
