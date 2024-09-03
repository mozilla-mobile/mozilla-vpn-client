/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*

This is a GLOBAL setup file.

before applies to before running all tests.
beforeEach applies to running before every test.

*/

// Load config as early as possible
const dotenv = require('dotenv');
dotenv.config();

const fs = require('fs');
const {execSync, spawn} = require('child_process');
const vpn = require('./helper.js');
const vpnWS = require('./helperWS.js');

const fxaServer = require('./servers/fxa.js');
const guardian = require('./servers/guardian.js');
const addonServer = require('./servers/addon.js');
const captivePortalServer = require('./servers/captivePortalServer.js');

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
  await vpn.connect(vpnWS, {hostname: '127.0.0.1'});
}

function vpnIsInactive() {
  try {
    vpnProcess.kill(pid, 0);
    return false;
  } catch (e) {
    return true;
  }
}

exports.startAndConnect = startAndConnect;
exports.vpnIsInactive = vpnIsInactive;

exports.mochaHooks = {
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

    await guardian.start();
    await fxaServer.start(guardian.url);
    await addonServer.start();
    await captivePortalServer.start();

    process.env['MVPN_API_BASE_URL'] = guardian.url;
    process.env['MZ_FXA_API_BASE_URL'] = fxaServer.url;
    process.env['MZ_ADDON_URL'] = `${addonServer.url}/01_empty_manifest/`;
    process.env['MVPN_SKIP_ADDON_SIGNATURE'] = '1';

    process.env['MZ_CAPTIVE_PORTAL_URL'] =
      `http://%1:${captivePortalServer.port}/success.txt`;
  },

  async afterAll() {
    guardian.stop();
    fxaServer.stop();
    addonServer.stop();
    captivePortalServer.stop();

    guardian.throwExceptionsIfAny();
    fxaServer.throwExceptionsIfAny();
    addonServer.throwExceptionsIfAny();
    captivePortalServer.throwExceptionsIfAny();
  },

  async beforeEach() {
    if (this.currentTest.ctx.authenticationNeeded &&
      !this.currentTest.ctx.vpnSettings) {
      console.log('Retrieving the setting file...');

      guardian.overrideEndpoints = null;
      fxaServer.overrideEndpoints = null;

      await startAndConnect();
      await vpn.reset();
      await vpn.setSetting('startAtBoot', false);
      await vpn.setSetting('localhostRequestsOnly', 'true');
      await vpn.authenticateInApp();

      const fileName = await vpn.settingsFileName();

      await vpn.quit();

      const content = await fs.readFileSync(fileName);
      this.currentTest.ctx.vpnSettings = {fileName, content};
    }

    guardian.overrideEndpoints =
      this.currentTest.ctx.guardianOverrideEndpoints || null;
    fxaServer.overrideEndpoints =
      this.currentTest.ctx.fxaOverrideEndpoints || null;

    if (this.currentTest.ctx.authenticationNeeded) {
      fs.writeFileSync(
        this.currentTest.ctx.vpnSettings.fileName,
        this.currentTest.ctx.vpnSettings.content);
      await startAndConnect();
    } else {
      if (this.currentTest.ctx.vpnSettings) {
        // We already have the settings file, we can remove it before starting
        // the app to have a really "hard" reset.
        fs.unlinkSync(this.currentTest.ctx.vpnSettings.fileName);
        this.currentTest.ctx.vpnSettings = null;
      }

      await startAndConnect();
      await vpn.gleanTestReset();
      await vpn.reset();
    }

    await vpn.setGleanAutomationHeader();

    // Add servers to the context so that stub endpoints can be modified in the
    // middle of the tests
    this.currentTest.ctx.guardianServer = guardian;
    this.currentTest.ctx.fxaServer = fxaServer;

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
        const data = await vpn.screenCapture();
        const buffer = Buffer.from(data, 'base64');
        const title = this.currentTest.title.toLowerCase();
        const filename = title.replace(/[^a-z0-9]/g, '_');
        if (!fs.existsSync(process.env.ARTIFACT_DIR)) {
          fs.mkdirSync(process.env.ARTIFACT_DIR);
        }
        if (!fs.existsSync(dir)) {
          fs.mkdirSync(dir);
        }
        fs.writeFileSync(`${dir}/${filename}.png`, buffer);
      }
    }
    // Reset error logs
    stdErr = '';
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

    vpnProcess.stdin.pause();
    vpnProcess.kill();

    await vpnProcessTerminatePromise;
  },
}
