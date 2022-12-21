/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*

This is a GLOBAL setup file.

before applies to before running all tests.
beforeEach applies to running before every test.

*/

const {URL} = require('node:url');

const vpn = require('./helper.js');
const vpnWasm = require('./helperWasm.js');

const fxaServer = require('./servers/fxa.js');
const guardian = require('./servers/guardian.js');
const addonServer = require('./servers/addon.js');
const networkBenchmark = require('./servers/networkBenchmark.js');
const wasm = require('./wasm.js');

const {Builder, By, Key, until} = require('selenium-webdriver');

let driver;

async function startAndConnect() {
  await driver.get(process.env['MZ_WASM_URL']);
  await vpn.connect(vpnWasm, {url: process.env['MZ_WASM_URL'], driver});
}

exports.mochaHooks = {
  async beforeAll() {
    const u = new URL(`http://localhost:${wasm.start()}/test.html`);
    u.searchParams.set('guardian', `http://localhost:${guardian.start()}`);
    u.searchParams.set('fxa', `http://localhost:${fxaServer.start()}`);
    u.searchParams.set(
        'addon', `http://localhost:${addonServer.start()}/01_empty_manifest/`);
    u.searchParams.set('benchmark',
        `http://localhost:${networkBenchmark.start()}`);

    process.env['MZ_WASM_URL'] = u.toString();
    process.env['MVPN_SKIP_ADDON_SIGNATURE'] = '1';

    driver = await new Builder().forBrowser('firefox').build();
  },

  async afterAll() {
    guardian.stop();
    fxaServer.stop();
    addonServer.stop();
    networkBenchmark.stop();
    wasm.stop();

    guardian.throwExceptionsIfAny();
    fxaServer.throwExceptionsIfAny();
    addonServer.throwExceptionsIfAny();
    networkBenchmark.throwExceptionsIfAny();

    await driver.quit();
  },

  async beforeEach() {
    this.currentTest.ctx.wasm = true;

    guardian.overrideEndpoints =
        this.currentTest.ctx.guardianOverrideEndpoints || null;
    fxaServer.overrideEndpoints =
        this.currentTest.ctx.fxaOverrideEndpoints || null;

    await startAndConnect();
    await vpn.setGleanAutomationHeader();
    await vpn.setSetting('tipsAndTricksIntroShown', 'true')

    if (this.currentTest.ctx.authenticationNeeded) {
      await vpn.authenticateInApp(true, true);
    }

    console.log('Starting test:', this.currentTest.title);
  },

  async afterEach() {
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
