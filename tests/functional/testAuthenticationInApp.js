/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

const assert = require('assert');
const vpn = require('./helper.js');

describe('User authentication', function() {
  this.timeout(60000);

  it('Completes authentication int app', async () => {
    await vpn.authenticateInApp();
  });
});
