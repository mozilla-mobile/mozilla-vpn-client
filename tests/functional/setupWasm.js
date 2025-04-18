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
const captivePortalServer = require('./servers/captivePortalServer.js');
const wasm = require('./wasm.js');

const { Builder, logging } = require('selenium-webdriver');
const firefox = require('selenium-webdriver/firefox');

const { tmpdir } = require('os');
const path = require('path');
const fs = require("fs");
const net = require("net");

const stdout = path.join(tmpdir(), "wasm-stdout.txt");

let driver;
let url;
let logServer;
let logSocket;

exports.mochaHooks = {
  async beforeAll() {
    // Give the webdriver a bit more time to connect
    this.timeout(90000);

    if (process.env['MZ_WASM_URL']) {
      url = process.env['MZ_WASM_URL'];
    } else {
      await wasm.start(false);
      url = wasm.url;
    }

    // Create a named pipe to receive log content.
    let pipePath = path.join(tmpdir(), "wasm-stdout.pipe");
    fs.rmSync(pipePath, {'force': true});
    logServer = net.createServer().listen(pipePath, 8);
    logServer.on('connection', (s) => {
      s.end(); // Server has nothing to write to the socket
      s.on('data', (msg) => {
        fs.appendFile(stdout, msg.toString(), () => {});
      }).on('end', () => {
        s.destroy();
      });
    }).on('close', () => {
      fs.rmSync(pipePath, {'force': true});
    });

    logSocket = net.createConnection(pipePath);
    logSocket.on('close', () => {
      logSocket.destroy();
    });

    await guardian.start(false);
    await fxaServer.start(guardian.url, false);
    await addonServer.start(false);
    await captivePortalServer.start(false);

    const u = new URL(`${url}/test.html`);
    u.searchParams.set('guardian', guardian.url);
    u.searchParams.set('fxa', fxaServer.url);
    u.searchParams.set('addon', `${addonServer.url}/01_empty_manifest/`);
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
            logSocket,
            'ignore',
          ])
      )
      .build();
  },

  async afterAll() {
    guardian.stop();
    fxaServer.stop();
    addonServer.stop();
    captivePortalServer.stop();
    wasm.stop();

    guardian.throwExceptionsIfAny();
    fxaServer.throwExceptionsIfAny();
    addonServer.throwExceptionsIfAny();
    captivePortalServer.throwExceptionsIfAny();

    await driver.quit();

    logSocket.destroy();
    logServer.close();
  },

  async beforeEach() {
    this.currentTest.ctx.wasm = true;

    // Clear out the log file.
    fs.truncateSync(stdout);

    guardian.overrideEndpoints =
      this.currentTest.ctx.guardianOverrideEndpoints || null;
    fxaServer.overrideEndpoints =
      this.currentTest.ctx.fxaOverrideEndpoints || null;

    await driver.get(url);
    await vpn.connect(vpnWasm, { url, driver });
    await vpn.setGleanAutomationHeader();

    if (this.currentTest.ctx.authenticationNeeded) {
      await vpn.authenticateInApp();
    }

    // Add servers to the context so that stub endpoints can be modified in the
    // middle of the tests
    this.currentTest.ctx.guardianServer = guardian;
    this.currentTest.ctx.fxaServer = fxaServer;

    console.log('Starting test:', this.currentTest.title);
  },

  async afterEach() {
    console.log('After each setupWASM starting');
    if (!this.currentTest.state || this.currentTest.state === 'failed') {
      // Print error logs
      console.log('::group::Error Logs');
      console.log(fs.readFileSync(stdout).toString());
      console.log('::endgroup');
    }

    console.log('Did not fail');

    // Close VPN app
    // If something's gone really wrong with the test,
    // then this can fail and cause the tests to hang.
    // Logging the error lets us clean-up and move on.
    try {
      console.log('Attempting reset');
      await vpn.hardReset();
      console.log('Attempting quit');
      await Promise.race([
        vpn.quit(),
        new Promise(
            (_, reject) => setTimeout(
                () =>
                    reject(new Error('vpn.quit() timed out after 15 seconds')),
                15000)),
      ]);
    } catch (error) {
      console.error(error);
    }
    console.log('Attempting disconnect');
    vpn.disconnect();
    // Give each test 2 seconds to chill!
    // Seems to help with tests that are slow to close vpn app at end.
    console.log('Almost final wait');
    await vpn.wait();
    console.log('Real final wait');
    await vpn.wait();
    console.log('Completed');
  },
}
