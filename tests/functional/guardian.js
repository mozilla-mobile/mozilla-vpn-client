/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const Server = require('./server.js');
const constants = require('./constants.js');

const UserData = {
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

const GuardianEndpoints = {
  GETs: {
    '/api/v1/vpn/featurelist': {status: 200, body: {features: {}}},
    '/api/v1/vpn/versions': {status: 200, body: {}},
    '/__heartbeat__': {status: 200, body: {mullvadOK: true, dbOK: true}},
    '/api/v2/vpn/login/ios': {
      status: 200,
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

    '/api/v1/vpn/featurelist': {status: 200, body: {/* TODO */}},
    '/api/v1/vpn/account': {status: 200, body: UserData},
    '/api/v1/vpn/servers': {
      status: 200,
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

    '/api/v1/vpn/surveys': {status: 200, body: []},
    '/api/v1/vpn/dns/detectportal': {
      status: 200,
      body: [
        {address: '34.107.221.82', family: 4},
        {address: '2600:1901:0:38d7::', family: 6}
      ]
    },
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
    '/api/v1/vpn/device/': {match: 'startWith', status: 204, body: {}},
  },
};

let server = null;
module.exports = {
  start() {
    server = new Server('Guardian', constants.GUARDIAN_PORT, GuardianEndpoints);
    return constants.GUARDIAN_PORT;
  },

  stop() {
    server.stop();
  },

  get overrideEndpoints() {
    return server.overrideEndpoints;
  },

  set overrideEndpoints(value) {
    server.overrideEndpoints = value;
  },

  throwExceptionsIfAny() {
    server.throwExceptionsIfAny();
  },
};
