/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
import Server from './server.js';

// Mock server for VPN Network Benchmark:
// https://github.com/mozilla-services/vpn-network-benchmark
let server = null;
export default {
  async start(headerCheck = true) {
    server = new Server(
        'VPN Network Benchmark', {
          GETs: {
            '/': {
              status: 200,
              bodyRaw: new Array(1024).join('a'),
            }
          },
          POSTs: {
            '/': {status: 200, body: {}},
          },
        },
        headerCheck);
    await server.start();
  },

  stop() {
    server.stop();
  },

  get port() {
    return server.port;
  },

  get url() {
    return server.url;
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
