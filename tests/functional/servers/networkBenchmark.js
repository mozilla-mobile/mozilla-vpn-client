/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const Server = require('./server.js');
const constants = require('../constants.js');

// Mock server for VPN Network Benchmark:
// https://github.com/mozilla-services/vpn-network-benchmark
let server = null;
module.exports = {
  start() {
    server = new Server(
      'VPN Network Benchmark',
      constants.UPLOAD_BENCHMARK_PORT,
      {
        POSTs: {
          '/': {status: 200, body: {}},
        },
      }
    );
    return constants.UPLOAD_BENCHMARK_PORT;
  },

  stop() {
    server.stop();
  },

  throwExceptionsIfAny() {
    server.throwExceptionsIfAny();
  },
};
