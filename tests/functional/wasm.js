/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const express = require('express')
const path = require('node:path');
const ports = require('./servers/ports.js');

let server;
let port;

function createServer(app) {
  return new Promise(r => {
    port = ports.register('wasm');
    server = app.listen(port);
    server.on('error', err => {
      if (err.code === 'EADDRINUSE') {
        createServer(app).then(r);
        return;
      }

      throw new Error(err.code);
    });
    server.on('listening', r);
  });
}

module.exports = {
  async start() {
    const app = express();
    app.use(express.static(path.join(__dirname, '..', '..', 'wasm')));

    await createServer(app);
  },

  stop() {
    if (server) {
      server.close();
      server = null;
    }
  },

  get port() {
    return port;
  },

  get url() {
    return `http://localhost:${port}`;
  }

};
