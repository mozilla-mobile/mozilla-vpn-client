/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const Server = require('./server.js');

let server = null;
module.exports = {
  async start(headerCheck = true) {
    server = new Server(
        'Captive portal server', {
          GETs: {
            '/success.txt': {
              status: 200,
              bodyRaw: 'success',
            }
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
