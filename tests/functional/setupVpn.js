/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*

This is a GLOBAL setup file.

before applies to before running all tests.
beforeEach applies to running before every test.

*/

// Load config as early as possible
import { config } from 'dotenv';
config();

import { readFileSync, writeFileSync, unlinkSync, existsSync, mkdirSync } from 'fs';
import { execSync, spawn } from 'child_process';
import { connect, reset, setSetting, authenticateInApp, settingsFileName, quit, gleanTestReset, setGleanAutomationHeader, screenCapture, hardReset, disconnect } from './helper.js';
import vpnWS from './helperWS.js';

import { start, url, stop, throwExceptionsIfAny, overrideEndpoints } from './servers/fxa.js';
import { start as _start, url as _url, stop as _stop, throwExceptionsIfAny as _throwExceptionsIfAny, overrideEndpoints as _overrideEndpoints } from './servers/guardian.js';
import { start as __start, url as __url, stop as __stop, throwExceptionsIfAny as __throwExceptionsIfAny } from './servers/addon.js';
import { start as ___start, url as ___url, stop as ___stop, throwExceptionsIfAny as ___throwExceptionsIfAny, overrideEndpoints as __overrideEndpoints } from './servers/networkBenchmark.js';
import { start as ____start, port, stop as ____stop, throwExceptionsIfAny as ____throwExceptionsIfAny } from './servers/captivePortalServer.js';

const app = process.env.MVPN_BIN;
let vpnProcess = null;
let vpnProcessTerminatePromise = null;
let stdErr = '';

async function startAndConnect() {
  vpnProcess = spawn(app, ['ui', '--testing']);
  stdErr += 'VPN Process ID: ' + vpnProcess.pid;
  vpnProcess.stderr.on('data', (data) => {
    stdErr += data;
  });

  vpnProcessTerminatePromise = new Promise(r => {
    vpnProcess.on('exit', (code) => r());
  });

  // Connect to VPN
  await connect(vpnWS, {hostname: '127.0.0.1'});
}

function vpnIsInactive() {
  try {
    vpnProcess.kill(pid, 0);
    return false;
  } catch (e) {
    return true;
  }
}

const _startAndConnect = startAndConnect;
export { _startAndConnect as startAndConnect };
const _vpnIsInactive = vpnIsInactive;
export { _vpnIsInactive as vpnIsInactive };

export const mochaHooks = {
  async beforeAll() {
    // Check VPN app exists. If not, bail.
    try {
      const stdout = execSync(`"${app}" --version`);
      console.log(`VPN Version is: ${stdout}`);
    } catch (error) {
      console.error(`Could not run "${app}".`);
      console.error('Have you set MVPN_BIN in .env or environment?');
      console.error(`stdout: ${stdout}`);
      console.error(`stderr: ${stderr}`);
      process.exit(1);
    }

    await _start();
    await start(_url);
    await __start();
    await ___start();
    await ____start();

    process.env['MVPN_API_BASE_URL'] = _url;
    process.env['MZ_FXA_API_BASE_URL'] = url;
    process.env['MZ_ADDON_URL'] = `${__url}/01_empty_manifest/`;
    process.env['MVPN_SKIP_ADDON_SIGNATURE'] = '1';

    process.env['MZ_BENCHMARK_DOWNLOAD_URL'] = ___url;
    process.env['MZ_BENCHMARK_UPLOAD_URL'] = ___url;

    process.env['MZ_CAPTIVE_PORTAL_URL'] =
      `http://%1:${port}/success.txt`;
  },

  async afterAll() {
    _stop();
    stop();
    __stop();
    ___stop();
    ____stop();

    _throwExceptionsIfAny();
    throwExceptionsIfAny();
    __throwExceptionsIfAny();
    ___throwExceptionsIfAny();
    ____throwExceptionsIfAny();
  },

  async beforeEach() {
    if (this.currentTest.ctx.authenticationNeeded &&
      !this.currentTest.ctx.vpnSettings) {
      console.log('Retrieving the setting file...');

      _overrideEndpoints = null;
      overrideEndpoints = null;
      __overrideEndpoints = null;

      await startAndConnect();
      await reset();
      await setSetting('tipsAndTricksIntroShown', 'true');
      await setSetting('localhostRequestsOnly', 'true');
      await authenticateInApp(true, true);

      const fileName = await settingsFileName();

      await quit();

      const content = await readFileSync(fileName);
      this.currentTest.ctx.vpnSettings = {fileName, content};
    }

    _overrideEndpoints =
      this.currentTest.ctx.guardianOverrideEndpoints || null;
    overrideEndpoints =
      this.currentTest.ctx.fxaOverrideEndpoints || null;
    __overrideEndpoints =
      this.currentTest.ctx.networkBenchmarkOverrideEndpoints || null;

    if (this.currentTest.ctx.authenticationNeeded) {
      writeFileSync(
        this.currentTest.ctx.vpnSettings.fileName,
        this.currentTest.ctx.vpnSettings.content);
      await startAndConnect();
    } else {
      if (this.currentTest.ctx.vpnSettings) {
        // We already have the settings file, we can remove it before starting
        // the app to have a really "hard" reset.
        unlinkSync(this.currentTest.ctx.vpnSettings.fileName);
        this.currentTest.ctx.vpnSettings = null;
      }

      await startAndConnect();
      await gleanTestReset();
      await reset();
      await setSetting('tipsAndTricksIntroShown', 'true')
    }

    await setGleanAutomationHeader();

    console.log('Starting test:', this.currentTest.title);
  },

  async afterEach() {
    // Collect errors on failure
    if (!this.currentTest.state || this.currentTest.state === 'failed') {
      // Print error logs
      console.log('::group::Error Logs');
      console.log(stdErr);
      console.log('::endgroup');

      // Screenshot of failure state
      if (('ARTIFACT_DIR' in process.env)) {
        const dir = process.env.ARTIFACT_DIR + '/screencapture';
        const data = await screenCapture();
        const buffer = Buffer.from(data, 'base64');
        const title = this.currentTest.title.toLowerCase();
        const filename = title.replace(/[^a-z0-9]/g, '_');
        if (!existsSync(dir)) {
          mkdirSync(dir);
        }
        writeFileSync(`${dir}/${filename}.png`, buffer);
      }
    }
    // Reset error logs
    stdErr = '';
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

    vpnProcess.stdin.pause();
    vpnProcess.kill();

    await vpnProcessTerminatePromise;
  },
}
