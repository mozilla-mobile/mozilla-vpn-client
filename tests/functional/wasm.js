/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const express = require('express');
const fs = require('fs');
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

    const build_directory = process.env["WASM_BUILD_DIRECTORY"]
      ? path.parse(process.env["WASM_BUILD_DIRECTORY"])
      : path.join(__dirname, '..', '..', 'build', 'build');

    if (!fs.existsSync(build_directory)) {
      throw new Error(`Provided build directory doesn't exist.`)
    }

    app.use(express.static(build_directory));

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

if (typeof require !== 'undefined' && require.main === module) {
  module.exports.start();
  console.log(`Starting Mozilla VPN on ${module.exports.url}.`)
}
