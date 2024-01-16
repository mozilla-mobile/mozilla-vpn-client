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

import { readFileSync, writeFileSync, unlinkSync} from 'fs';
import { execSync, spawn } from 'node:child_process';
import { setInspectorClient , reset, setSetting, authenticateInApp, settingsFileName, quit, gleanTestReset, setGleanAutomationHeader, hardReset, waitFor } from './helper.js';
import {connect as connectToWebsocket} from './helperWS.js';
import { env, exit } from 'process'

import { fxa } from './servers/fxa.js';
import { guardian } from './servers/guardian.js';
import { addonServer  } from './servers/addon.js';
import { networkBenchmark  } from './servers/networkBenchmark.js';
import { captivePortal  } from './servers/captivePortalServer.js';

const app = env["MVPN_BIN"];
let vpnProcess = null;
let vpnProcessTerminatePromise = null;
let stdErr = '';

async function startAndConnect() {
  console.log("startAndConnect")
  if(vpnProcessTerminatePromise){
    console.log("vpnProcessTerminatePromise exits, awaiting exit.")
    await vpnProcessTerminatePromise;
  }

 
  vpnProcess = spawn(app, ['ui', '--testing']);
  stdErr += 'VPN Process ID: ' + vpnProcess.pid;
  vpnProcess.stderr.on('data', (data) => {
    stdErr += data;
  });
  vpnProcess.stdout.on('data', (data) => {
    console.log(data);
  });
  vpnProcessTerminatePromise = new Promise(r => {
    vpnProcess.on('exit', () =>{
      console.log("App exited");
      r();
    });
  });

  console.log("connectToWebsocket")
  await waitFor(200);
  const inspector = await connectToWebsocket( {hostname: '127.0.0.1'});
  // Connect to VPN
  console.log("setInspectorClient")
  await setInspectorClient(inspector);
}

function vpnIsInactive() {
  try {
    vpnProcess.kill(pid, 0);
    return false;
  } catch (e) {
    return true;
  }
}

const startGuardianAndConnect = startAndConnect;
export { startGuardianAndConnect as startAndConnect };
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
      exit(1);
    }

    await guardian.start();
    await fxa.start(guardian.url);
    await addonServer.start();
    await networkBenchmark.start();
    await captivePortal.start();

    env['MVPN_API_BASE_URL'] = guardian.url;
    env['MZ_FXA_API_BASE_URL'] = fxa.url;
    env['MZ_ADDON_URL'] = `${addonServer.url}/01_empty_manifest/`;
    env['MVPN_SKIP_ADDON_SIGNATURE'] = '1';

    env['MZ_BENCHMARK_DOWNLOAD_URL'] = networkBenchmark.url;
    env['MZ_BENCHMARK_UPLOAD_URL'] = networkBenchmark.url;

    env['MZ_CAPTIVE_PORTAL_URL'] =
      `http://%1:${captivePortal.port}/success.txt`;
  },

  async afterAll() {
    guardian.stop();
    fxa.stop();
    addonServer.stop();
    networkBenchmark.stop();
    captivePortal.stop();

    guardian.throwExceptionsIfAny();
    fxa.throwExceptionsIfAny();
    addonServer.throwExceptionsIfAny();
    networkBenchmark.throwExceptionsIfAny();
    captivePortal.throwExceptionsIfAny();
  },

  async beforeEach() {
    if (this.currentTest.ctx.authenticationNeeded &&
      !this.currentTest.ctx.vpnSettings) {
      console.log('Retrieving the setting file...');
      
      if(vpnProcessTerminatePromise){
          throw new Error("APP WAS NOT QUIT PROPERLY")
      }

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

    guardian.overrideEndpoints =
      this.currentTest.ctx.guardianOverrideEndpoints || null;
    fxa.overrideEndpoints =
      this.currentTest.ctx.fxaOverrideEndpoints || null;
    networkBenchmark.overrideEndpoints =
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
    vpnProcess.stdin.pause();
    vpnProcess.kill();
    await vpnProcessTerminatePromise;
    vpnProcessTerminatePromise = undefined;
  },
}
