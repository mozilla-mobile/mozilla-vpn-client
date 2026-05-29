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
      await controller.waitForInitialView();

      await controller.waitForQuery('//initialStackView{busy=false}');
      await controller.clickOnQuery('//signUpButton');

      await controller.waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userAuthenticated', 'true');
      await controller.wait();
      await controller.waitForQuery('//screenLoader{busy=false}');

      await controller.waitForQueryAndClick('//telemetryPolicyButton');
      await controller.waitForQuery('//screenLoader{busy=false}');

      await controller.waitForQuery('//controllerTitle');
    }
  },

  {
    name: 'Device limit reached',
    callback: async function() {
      await controller.waitForInitialView();

      await controller.waitForQuery('//initialStackView{busy=false}');
      await controller.clickOnQuery('//signUpButton');

      await controller.waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userAuthenticated', 'true');
      await controller.wait();

      await controller.waitForQuery('//screenLoader{busy=false}');

      await controller.waitForQuery('//screenLoader{busy=false}');

      await controller.waitForQueryAndClick('//telemetryPolicyButton');
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
      await controller.waitForInitialView();

      await controller.waitForQuery('//initialStackView{busy=false}');
      await controller.clickOnQuery('//signUpButton');

      await controller.waitForMozillaProperty(
          'Mozilla.VPN', 'VPN', 'userAuthenticated', 'true');
      await controller.waitForQuery('//screenLoader{busy=false}');

      await controller.waitForQueryAndClick('//telemetryPolicyButton');
      await controller.waitForQuery('//screenLoader{busy=false}');

      await controller.waitForQuery('//controllerTitle');

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

      await controller.waitForInitialView();

      await controller.waitForQuery('//initialStackView{busy=false}');
      await controller.clickOnQuery('//signUpButton');
    },

    fxaOverrideEndpoints: {
      GETs: {
        '/v1/oauth/subscriptions/iap/plans/guardian-vpn':
            {status: 200, body: {products: SubscriptionProducts}},
      },
      POSTs: {},
      DELETEs: {},
    },

    guardianOverrideEndpoints: {
      GETs: {
        '/api/v1/vpn/account': {status: 200, body: SubscriptionNeededUserData},
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
      await controller.waitForInitialView();

      await controller.waitForQuery('//initialStackView{busy=false}');
      await controller.clickOnQuery('//signUpButton');
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
