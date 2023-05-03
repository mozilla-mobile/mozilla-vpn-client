/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*
This is a GLOBAL setup file.
before applies to before running all tests.
beforeEach applies to running before every test.
*/
// Load the .env file.
require('dotenv').config()


const vpn = require('../helper.js');
const androidconnector = require('../clientConnector/androidconnector');

const fxaServer = require('../servers/fxa.js');
const guardian = require('../servers/guardian.js');
const addonServer = require('../servers/addon.js');
const networkBenchmark = require('../servers/networkBenchmark.js');
const captivePortalServer = require('../servers/captivePortalServer.js');
const { tmpdir } = require('os');
const path = require('path');
const fs = require("fs")

const stdout = path.join(tmpdir(), "stdout.txt")

async function startAndConnect() {
  const options = {
    "guardian": guardian.url,
    "fxa": fxaServer.url,
    "fxa": fxaServer.url,
    "addon": `${addonServer.url}/01_empty_manifest/`,
    "benchmark":networkBenchmark.url,
    "captivePortal": `http://%1:${captivePortalServer.port}/success.txt`,
    "env":{
      "MVPN_SKIP_ADDON_SIGNATURE":1
    }
  }

  await vpn.connect(androidconnector, options);
}

exports.mochaHooks = {
  async beforeAll() {
    await guardian.start(false);
    await fxaServer.start(guardian.url, false);
    await addonServer.start(false);
    await networkBenchmark.start(false);
    await captivePortalServer.start(false);
  },

  async afterAll() {
    guardian.stop();
    fxaServer.stop();
    addonServer.stop();
    networkBenchmark.stop();
    captivePortalServer.stop();


    guardian.throwExceptionsIfAny();
    fxaServer.throwExceptionsIfAny();
    addonServer.throwExceptionsIfAny();
    networkBenchmark.throwExceptionsIfAny();
    captivePortalServer.throwExceptionsIfAny();
  },

  async beforeEach() {
    this.currentTest.ctx.android = true;

    guardian.overrideEndpoints =
      this.currentTest.ctx.guardianOverrideEndpoints || null;
    fxaServer.overrideEndpoints =
      this.currentTest.ctx.fxaOverrideEndpoints || null;
    networkBenchmark.overrideEndpoints =
      this.currentTest.ctx.networkBenchmarkOverrideEndpoints || null;

    await startAndConnect();
    await vpn.setGleanAutomationHeader();
    await vpn.setSetting('tipsAndTricksIntroShown', 'true')

    if (this.currentTest.ctx.authenticationNeeded) {
      await vpn.authenticateInApp(true, true);
    }

    console.log('Starting test:', this.currentTest.title);
  },

  async afterEach() {
    if (!this.currentTest.state || this.currentTest.state === 'failed') {
      // Print error logs
      console.log('::group::Error Logs');
      console.log(fs.readFileSync(stdout).toString());
      console.log('::endgroup');
    }
    // Close VPN app
    // If something's gone really wrong with the test,
    // then this can fail and cause the tests to hang.
    // Logging the error lets us clean-up and move on.
    try {
      await vpn.hardReset();
      await vpn.quit();
    } catch (error) {
      console.error(error);
    }
    vpn.disconnect();
    // Give each test 2 seconds to chill!
    // Seems to help with tests that are slow to close vpn app at end.
    await vpn.wait();
    await vpn.wait();
  },
}
