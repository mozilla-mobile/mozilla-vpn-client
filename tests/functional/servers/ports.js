/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const ports = {};
let next_port = 3000;

module.exports = {
  register(name) {
    if (!(name in ports)) {
      ports[name] = 0;
    }

    const port = next_port++;
    ports[name] = port;
    return port;
  }
};
