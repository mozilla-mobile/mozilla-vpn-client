/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const Server = require('./server.js');
const constants = require('./constants.js');

const FxaEndpoints = {
  GETs: {},

  POSTs: {
    '/v1/account/status': {status: 200, body: {exists: true}},
    '/v1/account/login': {
      status: 200,
      body: {
        sessionToken: 'sessionToken',
        'verified': true,
        verificationMethod: ''
      }
    },
    '/v1/oauth/authorization': {
      status: 200,
      body: {
        code: 'the-code',
        state: '',
        redirect: `http://localhost:${constants.GUARDIAN_PORT}/final_redirect`
      }
    },
    '/v1/session/destroy': {status: 200, body: {}},
  },

  DELETEs: {},
};

let server = null;
module.exports = {
  start() {
    server = new Server('FxA', constants.FXA_PORT, FxaEndpoints);
    return constants.FXA_PORT;
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
