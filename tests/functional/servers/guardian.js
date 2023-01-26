/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const WebSocketServer = require('websocket').server;
const Server = require('./server.js');
const guardianEndpoints = require('./guardian_endpoints.js');

let server = null;
let wsServer = null;
module.exports = {
  async start(headerCheck = true) {
    server = new Server('Guardian', guardianEndpoints.endpoints, headerCheck);
    await server.start();

    wsServer = new WebSocketServer({
      httpServer: server._server,
      autoAcceptConnections: true,
    });
  },

  stop() {
    wsServer.closeAllConnections();
    wsServer.unmount();
    server.stop();
  },

  get port() {
    return server.port;
  },

  get url() {
    return server.url;
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
