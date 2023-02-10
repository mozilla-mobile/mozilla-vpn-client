/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

(function(exports) {

const UserData = {
  avatar: '',
  display_name: 'Test',
  email: 'test@mozilla.com',
  max_devices: 5,
  // subscriptions: {vpn: {active: true}},
  subscriptions: {vpn: {active: false}},
  devices: [{
    name: 'Current device',
    unique_id: '',
    pubkey: '',
    ipv4_address: '127.0.0.1',
    ipv6_address: '::1',
    created_at: new Date().toISOString()
  }],
};

const SubscriptionDetails = {
  plan: {amount: 123, currency: 'usd', interval: 'year', interval_count: 1},
  payment: {
    payment_provider: 'stripe',
    payment_type: 'credit',
    last4: '1234',
    exp_month: 12,
    exp_year: 2022,
    brand: 'visa',
  },
  subscription: {
    _subscription_type: 'web',
    created: 1,
    current_period_end: 2,
    cancel_at_period_end: true,
    // status: 'active'
    status: 'cancelled'
  },
};

exports.SubscriptionDetails = SubscriptionDetails;

const VALIDATORS = {
  guardianLoginVerify: {
    type: 'object',
    properties: {code: {type: 'string'}, code_verifier: {type: 'string'}},
    required: ['code', 'code_verifier']
  },

  guardianDevice: {
    type: 'object',
    properties: {
      name: {type: 'string'},
      unique_id: {type: 'string'},
      pubkey: {type: 'string'}
    },
    required: ['name', 'unique_id', 'pubkey']
  },
};

exports.validators = VALIDATORS;

exports.endpoints = {
  GETs: {
    '/api/v1/vpn/featurelist': {status: 200, body: {features: {}}},

    '/api/v1/vpn/versions': {status: 200, body: {}},

    '/__heartbeat__': {status: 200, body: {mullvadOK: true, dbOK: true}},

    '/api/v2/vpn/login/': {
      status: 200,
      match: 'startWith',
      queryStringParams:
          ['code_challenge', 'code_challenge_method', 'user_agent'],
      body: {
        fxa_oauth: {
          url: 'https://accounts.stage.mozaws.net/authorization',
          params: {
            client_id: 12345678,
            redirect_uri: '/oauth/success',
            response_type: 'code',
            state: 'pkce-a-long-code-here',
            scope:
                'profile https://identity.mozilla.com/account/subscriptions/iap https://identity.mozilla.com/account/subscriptions',
            access_type: 'offline',
            action: 'email',
            referrer: 'app-store-linux',
            device_id: 'the-device-id',
            flow_begin_time: Date.now(),
            flow_id: 'the-flow-id',
            user_agent: 'MozillaVPN/2.9.0 (sys:ubuntu 22.04)'
          }
        }
      }
    },

    // This is not actually a guardian endpoint but we use it to complete the
    // auth-flow
    '/final_redirect': {status: 200, body: {code: 'the-code'}},

    '/api/v1/vpn/account':
        {status: 200, requiredHeaders: ['Authorization'], body: UserData},

    '/api/v1/vpn/servers': {
      status: 200,
      requiredHeaders: ['Authorization'],
      body: {
        'countries': [
          {
            'name': 'Australia',
            'code': 'au',
            'cities': [
              {
                'name': 'Melbourne',
                'code': 'mel',
                'latitude': -37.815018,
                'longitude': 144.946014,
                'servers': [{
                  'hostname': 'host-au-mel',
                  'ipv4_addr_in': '127.0.0.1',
                  'ipv6_addr_in': '::1',
                  'weight': 3,
                  'include_in_country': true,
                  'public_key': 'public-key',
                  'port_ranges':
                      [[53, 53], [4000, 33433], [33565, 51820], [52000, 60000]],
                  'ipv4_gateway': '127.0.0.1',
                  'ipv6_gateway': '::1'
                }],
                'public_key': 'public-key',
                'ipv4_gateway': '127.0.0.1',
                'ipv6_gateway': '::1',
                'ipv4_addr_in': '127.0.0.1',
                'ipv6_addr_in': '::1'
              },
              {
                'name': 'Sydney',
                'code': 'syd',
                'latitude': -33.861481,
                'longitude': 151.205475,
                'servers': [
                  {
                    'hostname': 'host-au-syd',
                    'ipv4_addr_in': '127.0.0.1',
                    'ipv6_addr_in': '::1',
                    'weight': 100,
                    'include_in_country': true,
                    'public_key': 'public-key',
                    'port_ranges': [
                      [53, 53], [4000, 33433], [33565, 51820], [52000, 60000]
                    ],
                    'ipv4_gateway': '127.0.0.1',
                    'ipv6_gateway': '::1'
                  },
                ],
                'public_key': 'public-key',
                'ipv4_gateway': '127.0.0.1',
                'ipv6_gateway': '::1',
                'ipv4_addr_in': '127.0.0.1',
                'ipv6_addr_in': '::1'
              }
            ]
          },
          {
            'name': 'Austria',
            'code': 'at',
            'cities': [{
              'name': 'Vienna',
              'code': 'vie',
              'latitude': 48.210033,
              'longitude': 16.363449,
              'servers': [
                {
                  'hostname': 'host-at-vie',
                  'ipv4_addr_in': '127.0.0.1',
                  'ipv6_addr_in': '::1',
                  'weight': 100,
                  'include_in_country': true,
                  'public_key': 'public-key',
                  'port_ranges':
                      [[53, 53], [4000, 33433], [33565, 51820], [52000, 60000]],
                  'ipv4_gateway': '127.0.0.1',
                  'ipv6_gateway': '::1'
                },
              ],
              'public_key': 'public-key',
              'ipv4_gateway': '127.0.0.1',
              'ipv6_gateway': '::1',
              'ipv4_addr_in': '127.0.0.1',
              'ipv6_addr_in': '::1'
            }]
          },
          {
            'name': 'Belgium',
            'code': 'be',
            'cities': [{
              'name': 'Brussels',
              'code': 'bru',
              'latitude': 50.833333,
              'longitude': 4.333333,
              'servers': [
                {
                  'hostname': 'host-be-bru',
                  'ipv4_addr_in': '127.0.0.1',
                  'ipv6_addr_in': '::1',
                  'weight': 100,
                  'include_in_country': true,
                  'public_key': 'public-key',
                  'port_ranges':
                      [[53, 53], [4000, 33433], [33565, 51820], [52000, 60000]],
                  'ipv4_gateway': '127.0.0.1',
                  'ipv6_gateway': '::1'
                },
              ],
              'public_key': 'public-key',
              'ipv4_gateway': '127.0.0.1',
              'ipv6_gateway': '::1',
              'ipv4_addr_in': '127.0.0.1',
              'ipv6_addr_in': '::1'
            }]
          }
        ]
      }
    },

    '/api/v1/vpn/dns/detectportal': {
      status: 200,
      requiredHeaders: ['Authorization'],
      body: [{address: '127.0.0.1', family: 4}, {address: '::1', family: 6}]
    },

    '/api/v1/vpn/subscriptionDetails': {
      status: 200,
      requiredHeaders: ['Authorization'],
      body: SubscriptionDetails,
    },

    '/api/v3/vpn/products': {
      status: 200,
      requiredHeaders: ['Authorization'],
      body: {
        products: [
          {
            platform: 'dummy',
            id: 'monthly',
            featured_product: false,
            type: 'monthly'
          },
          {
            platform: 'dummy',
            id: 'half-monthly',
            featured_product: false,
            type: 'half-yearly'
          },
          {
            platform: 'dummy',
            id: 'yearly',
            featured_product: true,
            type: 'yearly'
          },
        ]
      }
    },

    '/api/v1/vpn/ipinfo': {
      status: 200,
      requiredHeaders: ['Authorization', 'Host'],
      body: {
        city: 'Mordor',
        country: 'XX',
        ip: '169.254.0.1',
        lat_long: '3.14159,-2.71828',
        subdivision: 'MTDOOM',
      }
    },

    '/r/vpn/terms': {status: 200, body: {}},

    '/r/vpn/privacy': {status: 200, body: {}},

    '/r/vpn/support': {status: 200, body: {}},
  },

  POSTs: {
    '/api/v2/vpn/login/verify': {
      status: 200,
      bodyValidator: VALIDATORS.guardianLoginVerify,
      body: {user: UserData, token: 'our-token'}
    },

    '/api/v1/vpn/device': {
      status: 201,
      requiredHeaders: ['Authorization'],
      bodyValidator: VALIDATORS.guardianDevice,
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
      body: {}
    },
  },
};
})(typeof exports === 'undefined' ? this['guardianEndpoints'] = {} : exports);
