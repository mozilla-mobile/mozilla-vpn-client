/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const Server = require('./server.js');
const constants = require('./constants.js');
const guardianEndpoints = require('./guardian_endpoints.js')

let server = null;
module.exports = {
  start() {
    server = new Server(
        'Guardian', constants.GUARDIAN_PORT, guardianEndpoints.endpoints);
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
