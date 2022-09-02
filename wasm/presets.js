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

const SubscriptionNeededUserData = {
  avatar: '',
  display_name: 'Test test',
  email: 'test@mozilla.com',
  max_devices: 5,
  subscriptions: {},
  devices: [{
    name: 'Current device',
    unique_id: '',
    pubkey: '',
    ipv4_address: '127.0.0.1',
    ipv6_address: '::1',
    created_at: new Date().toISOString()
  }],
};

const SubscriptionCompletedUserData = {
  avatar: '',
  display_name: 'Test test',
  email: 'test@mozilla.com',
  max_devices: 5,
  subscriptions: {vpn: {active: true}},
  devices: [{
    name: 'Current device',
    unique_id: '',
    pubkey: '',
    ipv4_address: '127.0.0.1',
    ipv6_address: '::1',
    created_at: new Date().toISOString()
  }],
};

const SubscriptionProducts = [
  {platform: 'dummy', id: 'ok', featured_product: true, type: 'monthly'},
  {
    platform: 'dummy',
    id: 'invalid',
    featured_product: false,
    type: 'half-yearly'
  },
  {platform: 'dummy', id: 'failed', featured_product: true, type: 'yearly'},
  {platform: 'dummy', id: 'canceled', featured_product: false, type: 'monthly'},
  {
    platform: 'dummy',
    id: 'already-subscribed',
    featured_product: false,
    type: 'half-yearly'
  },
  {
    platform: 'dummy',
    id: 'billing-not-available',
    featured_product: false,
    type: 'yearly'
  },
  {
    platform: 'dummy',
    id: 'not-validated',
    featured_product: false,
    type: 'monthly'
  },
];

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

      await controller.waitForElementProperty('screenLoader', 'busy', 'false');
      await controller.waitForElement('postAuthenticationButton');
      await controller.clickOnElement('postAuthenticationButton');

      await controller.wait();
      await controller.waitForElementProperty('screenLoader', 'busy', 'false');
      await controller.waitForElement('telemetryPolicyButton');
      await controller.clickOnElement('telemetryPolicyButton');

      await controller.wait();
      await controller.waitForElementProperty('screenLoader', 'busy', 'false');
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
    callback: async function() {
      await controller.flipFeatureOn('inAppAccountCreate');

      await controller.waitForMainView();

      await controller.wait();
      await controller.waitForElementProperty(
          'initialStackView', 'busy', 'false');
      await controller.clickOnElement('getStarted');

      mvpnWasm.addPresetInfo(
          'Existing account: wasm@wasm.wasm - Password: wasm');
      mvpnWasm.addPresetInfo('Blocked account: blocked@wasm.wasm');
      mvpnWasm.addPresetInfo('Invalid email address: invalid@wasm.wasm');
      mvpnWasm.addPresetInfo(
          'Already existing address in creation: dup@wasm.wasm');
      mvpnWasm.addPresetInfo('Unusable email: unusable@wasm.wasm');
      mvpnWasm.addPresetInfo('Type not supported address: type@wasm.wasm');
      mvpnWasm.addPresetInfo('Too many requests: toomany@wasm.wasm');
      mvpnWasm.addPresetInfo('Unknown account: unknown@wasm.wasm');
    },

    fxaOverrideEndpoints: {
      GETs: {},
      POSTs: {
        '/v1/account/status': {
          callback: (req, that) => {
            if (req.body.email === 'blocked@wasm.wasm') {
              mvpnWasm.addPresetInfo('Unblock verification code: 12345678');
              that.body = {errno: 107, validation: {keys: ['unblockCode']}};
              that.status = 400;
            } else if (req.body.email === 'unusable@wasm.wasm') {
              that.body = {
                errno: 149 /* This email can not currently be used to login */
              };
              that.status = 400;
            } else if (req.body.email === 'type@wasm.wasm') {
              that.body = {
                errno: 142 /* Sign in with this email type is not currently
                              supported */
              };
              that.status = 400;
            } else if (req.body.email === 'toomany@wasm.wasm') {
              that.body = {
                errno: 114 /* Client has sent too many requests */,
                retryAfter: 1234
              };
              that.status = 400;
            } else if (req.body.email === 'unknown@wasm.wasm') {
              that.body = {errno: 102 /* Unknwon account */};
              that.status = 400;
            } else if (req.body.email === 'invalid@wasm.wasm') {
              that.body = {errno: 107, validation: {keys: ['email']}};
              that.status = 400;
            } else {
              that.body = {exists: (req.body.email === 'wasm@wasm.wasm')};
              that.status = 200;
            }
          }
        },
        '/v1/account/create': {
          callback: (req, that) => {
            if (req.body.email === 'dup@wasm.wasm') {
              that.status = 400;
              that.body = {errno: 101 /* Account already exists */};
            } else {
              mvpnWasm.addPresetInfo('Email verification code: 123456');
              that.status = 200;
              that.body = {
                sessionToken: 'sessionToken',
                verified: false,
                verificationMethod: 'email-otp'
              };
            }
          }
        },
        '/v1/account/login': {
          callback: (req, that) => {
            if (req.body.email === 'wasm@wasm.wasm' &&
                req.body.authPW ===
                    '8c021c875ea5e86ed5b6229a0b1b5a3e7c94ebc68f397d858897156041fe6638') {
              mvpnWasm.addPresetInfo('TOTP code: 123456');
              that.status = 200;
              that.body = {
                sessionToken: 'sesionToken',
                verified: false,
                verificationMethod: 'totp-2fa'
              };
            } else if (
                req.body.email === 'blocked@wasm.wasm' &&
                req.body.unblockCode === '12345678') {
              that.status = 200;
              that.body = {
                sessionToken: 'sesionToken',
                verified: true,
              };
            } else if (req.body.email === 'blocked@wasm.wasm') {
              that.status = 400;
              that.body = {errno: 107, validation: {keys: ['unblockCode']}};
            } else {
              that.status = 400;
              that.body = {
                errno: 103,  // Incorrect password
              };
            }
          },
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

      await controller.waitForElementProperty('screenLoader', 'busy', 'false');
      await controller.waitForElement('postAuthenticationButton');
      await controller.clickOnElement('postAuthenticationButton');

      await controller.wait();
      await controller.waitForElementProperty('screenLoader', 'busy', 'false');
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

      await controller.waitForElementProperty('screenLoader', 'busy', 'false');
      await controller.waitForElement('postAuthenticationButton');
      await controller.clickOnElement('postAuthenticationButton');

      await controller.wait();
      await controller.waitForElementProperty('screenLoader', 'busy', 'false');
      await controller.waitForElement('telemetryPolicyButton');
      await controller.clickOnElement('telemetryPolicyButton');

      await controller.wait();
      await controller.waitForElementProperty('screenLoader', 'busy', 'false');
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

  {
    name: 'Subscription flow',
    callback: async function() {
      SubscriptionProducts.forEach((product, pos) => {
        mvpnWasm.addPresetInfo(`Product ${pos} has output ${product.id}`);
      });

      MVPNPresets.find(a => a.name === 'Subscription flow')
          .guardianOverrideEndpoints.GETs['/api/v1/vpn/account']
          .body = SubscriptionNeededUserData;

      await controller.waitForMainView();

      await controller.wait();
      await controller.waitForElementProperty(
          'initialStackView', 'busy', 'false');
      await controller.clickOnElement('getStarted');
    },

    guardianOverrideEndpoints: {
      GETs: {
        '/api/v1/vpn/account': {status: 200, body: SubscriptionNeededUserData},
        '/api/v3/vpn/products':
            {status: 200, body: {products: SubscriptionProducts}},
      },
      POSTs: {
        '/api/v1/vpn/purchases/wasm': {
          callback: (req, that) => {
            if (req.body.productId === 'ok') {
              MVPNPresets.find(a => a.name === 'Subscription flow')
                  .guardianOverrideEndpoints.GETs['/api/v1/vpn/account']
                  .body = SubscriptionCompletedUserData;
            }
            that.body = { status: req.body.productId }
          },
          status: 200,
        }
      },
      DELETEs: {},
    }
  },

  {
    name: 'Backend failure',
    callback: async function() {
      await controller.waitForMainView();

      await controller.wait();
      await controller.waitForElementProperty(
          'initialStackView', 'busy', 'false');
      await controller.clickOnElement('getStarted');
    },

    guardianOverrideEndpoints: {
      GETs: {
        '/__heartbeat__': {status: 200, body: {mullvadOK: false, dbOK: false}},
      },
    }
  },

  // TODO:
  // - unstable connection
  // - no signal
  // - alerts
];
