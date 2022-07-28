/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const express = require('express')
const constants = require('./constants.js');
const path = require('node:path');

let server;

module.exports = {
  start() {
    const app = express();
    app.use(express.static(path.join(__dirname, '..', '..', 'wasm')));
    server = app.listen(constants.WASM_PORT);
    return constants.WASM_PORT;
  },
  stop() {
    if (server) {
      server.close();
      server = null;
    }
  }
};
