/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const WebSocketServer = require('websocket').server;
const Server = require('./server.js');
const constants = require('./constants.js');
const guardianEndpoints = require('./guardian_endpoints.js');

let server = null;
let wsServer = null;
module.exports = {
  start() {
    server = new Server(
        'Guardian', constants.GUARDIAN_PORT, guardianEndpoints.endpoints);

    wsServer = new WebSocketServer({
      httpServer: server._server,
      autoAcceptConnections: true,
    });

    return constants.GUARDIAN_PORT;
  },

  stop() {
    wsServer.closeAllConnections();
    wsServer.unmount();
    server.stop();
  },

  broadcastMessage(message) {
    wsServer.broadcast(message);
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
