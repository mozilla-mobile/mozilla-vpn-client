/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*

This is a GLOBAL setup file.

before applies to before running all tests.
beforeEach applies to running before every test.

*/

import { URL } from 'node:url';

import { connect, setGleanAutomationHeader, setSetting, authenticateInApp, hardReset, quit, disconnect, wait } from './helper.js';
import vpnWasm from './helperWasm.js';

import { start, url as _url, stop, throwExceptionsIfAny, overrideEndpoints } from './servers/fxa.js';
import { start as _start, url as __url, stop as _stop, throwExceptionsIfAny as _throwExceptionsIfAny, overrideEndpoints as _overrideEndpoints } from './servers/guardian.js';
import { start as __start, url as ___url, stop as __stop, throwExceptionsIfAny as __throwExceptionsIfAny } from './servers/addon.js';
import { start as ___start, url as ____url, stop as ___stop, throwExceptionsIfAny as ___throwExceptionsIfAny, overrideEndpoints as __overrideEndpoints } from './servers/networkBenchmark.js';
import { start as ____start, port, stop as ____stop, throwExceptionsIfAny as ____throwExceptionsIfAny } from './servers/captivePortalServer.js';
import { start as _____start, url as _____url, stop as _____stop } from './wasm.js';

import { Builder, logging } from 'selenium-webdriver';
import { Options, ServiceBuilder } from 'selenium-webdriver/firefox';

import { tmpdir } from 'os';
import { join } from 'path';
import { openSync, readFileSync, writeFileSync } from "fs";

const stdout = join(tmpdir(), "stdout.txt")

let driver;
let url;

export const mochaHooks = {
  async beforeAll() {
    if (process.env['MZ_WASM_URL']) {
      url = process.env['MZ_WASM_URL'];
    } else {
      await _____start(false);
      url = _____url;
    }

    await _start(false);
    await start(__url, false);
    await __start(false);
    await ___start(false);
    await ____start(false);

    const u = new URL(`${url}/test.html`);
    u.searchParams.set('guardian', __url);
    u.searchParams.set('fxa', _url);
    u.searchParams.set('addon', `${___url}/01_empty_manifest/`);
    u.searchParams.set('benchmark', ____url);
    u.searchParams.set(
      'captivePortal', `http://%1:${port}/success.txt`);
    url = u.toString()

    process.env['MVPN_SKIP_ADDON_SIGNATURE'] = '1';

    const prefs = new logging.Preferences()
    prefs.setLevel(logging.Type.BROWSER, logging.Level.ALL);
    driver = await new Builder()
      .forBrowser('firefox')
      .setLoggingPrefs(prefs)
      .setFirefoxOptions(
        new Options()
          // provide access to console logs in Firefox
          .setPreference('devtools.console.stdout.content', true)
      )
      .setFirefoxService(
        new ServiceBuilder()
          .setStdio([
            'ignore',
            openSync(stdout, 'w'),
            'ignore',
          ])
      )
      .build();
  },

  async afterAll() {
    _stop();
    stop();
    __stop();
    ___stop();
    ____stop();
    _____stop();

    _throwExceptionsIfAny();
    throwExceptionsIfAny();
    __throwExceptionsIfAny();
    ___throwExceptionsIfAny();
    ____throwExceptionsIfAny();

    await driver.quit();
  },

  async beforeEach() {
    this.currentTest.ctx.wasm = true;

    _overrideEndpoints =
      this.currentTest.ctx.guardianOverrideEndpoints || null;
    overrideEndpoints =
      this.currentTest.ctx.fxaOverrideEndpoints || null;
    __overrideEndpoints =
      this.currentTest.ctx.networkBenchmarkOverrideEndpoints || null;

    await driver.get(url);
    await connect(vpnWasm, { url, driver });
    await setGleanAutomationHeader();
    await setSetting('tipsAndTricksIntroShown', 'true')

    if (this.currentTest.ctx.authenticationNeeded) {
      await authenticateInApp(true, true);
    }

    console.log('Starting test:', this.currentTest.title);
  },

  async afterEach() {
    if (!this.currentTest.state || this.currentTest.state === 'failed') {
      // Print error logs
      console.log('::group::Error Logs');
      console.log(readFileSync(stdout).toString());
      console.log('::endgroup');
    }

    // Clear our logs from file.
    writeFileSync(stdout, "");

    // Close VPN app
    // If something's gone really wrong with the test,
    // then this can fail and cause the tests to hang.
    // Logging the error lets us clean-up and move on.
    try {
      await hardReset();
      await quit();
    } catch (error) {
      console.error(error);
    }
    disconnect();
    // Give each test 2 seconds to chill!
    // Seems to help with tests that are slow to close vpn app at end.
    await wait();
    await wait();
  },
}
