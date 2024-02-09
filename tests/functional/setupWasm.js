/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*

This is a GLOBAL setup file.

before applies to before running all tests.
beforeEach applies to running before every test.

*/

const { URL } = require('node:url');

const vpn = require('./helper.js');
const vpnWasm = require('./helperWasm.js');

const fxaServer = require('./servers/fxa.js');
const guardian = require('./servers/guardian.js');
const addonServer = require('./servers/addon.js');
const networkBenchmark = require('./servers/networkBenchmark.js');
const captivePortalServer = require('./servers/captivePortalServer.js');
const wasm = require('./wasm.js');

const { Builder, logging } = require('selenium-webdriver');
const firefox = require('selenium-webdriver/firefox');

const { tmpdir } = require('os');
const path = require('path');
const fs = require("fs")

const stdout = path.join(tmpdir(), "stdout.txt")

let driver;
let url;

exports.mochaHooks = {
  async beforeAll() {
    if (process.env['MZ_WASM_URL']) {
      url = process.env['MZ_WASM_URL'];
    } else {
      await wasm.start(false);
      url = wasm.url;
    }

    await guardian.start(false);
    await fxaServer.start(guardian.url, false);
    await addonServer.start(false);
    await networkBenchmark.start(false);
    await captivePortalServer.start(false);

    const u = new URL(`${url}/test.html`);
    u.searchParams.set('guardian', guardian.url);
    u.searchParams.set('fxa', fxaServer.url);
    u.searchParams.set('addon', `${addonServer.url}/01_empty_manifest/`);
    u.searchParams.set('benchmark', networkBenchmark.url);
    u.searchParams.set(
      'captivePortal', `http://%1:${captivePortalServer.port}/success.txt`);
    url = u.toString()

    process.env['MVPN_SKIP_ADDON_SIGNATURE'] = '1';

    const prefs = new logging.Preferences()
    prefs.setLevel(logging.Type.BROWSER, logging.Level.ALL);
    driver = await new Builder()
      .forBrowser('firefox')
      .setLoggingPrefs(prefs)
      .setFirefoxOptions(
        new firefox.Options()
          // provide access to console logs in Firefox
          .setPreference('devtools.console.stdout.content', true)
      )
      .setFirefoxService(
        new firefox.ServiceBuilder()
          .setStdio([
            'ignore',
            fs.openSync(stdout, 'w'),
            'ignore',
          ])
      )
      .build();
  },

  async afterAll() {
    guardian.stop();
    fxaServer.stop();
    addonServer.stop();
    networkBenchmark.stop();
    captivePortalServer.stop();
    wasm.stop();

    guardian.throwExceptionsIfAny();
    fxaServer.throwExceptionsIfAny();
    addonServer.throwExceptionsIfAny();
    networkBenchmark.throwExceptionsIfAny();
    captivePortalServer.throwExceptionsIfAny();

    await driver.quit();
  },

  async beforeEach() {
    this.currentTest.ctx.wasm = true;

    guardian.overrideEndpoints =
      this.currentTest.ctx.guardianOverrideEndpoints || null;
    fxaServer.overrideEndpoints =
      this.currentTest.ctx.fxaOverrideEndpoints || null;
    networkBenchmark.overrideEndpoints =
      this.currentTest.ctx.networkBenchmarkOverrideEndpoints || null;

    await driver.get(url);
    await vpn.connect(vpnWasm, { url, driver });
    await vpn.setGleanAutomationHeader();
    await vpn.setSetting('tipsAndTricksIntroShown', 'true')

    if (this.currentTest.ctx.authenticationNeeded) {
      await vpn.authenticateInApp();
    }

    // Add servers to the context so that stub endpoints can be modified in the
    // middle of the tests
    this.currentTest.ctx.guardianServer = guardian;
    this.currentTest.ctx.fxaServer = fxaServer;
    this.currentTest.ctx.networkBenchmarkServer = networkBenchmark;

    console.log('Starting test:', this.currentTest.title);
  },

  async afterEach() {
    if (!this.currentTest.state || this.currentTest.state === 'failed') {
      // Print error logs
      console.log('::group::Error Logs');
      console.log(fs.readFileSync(stdout).toString());
      console.log('::endgroup');
    }

    // Clear our logs from file.
    fs.writeFileSync(stdout, "");

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
