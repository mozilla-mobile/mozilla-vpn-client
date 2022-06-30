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

const fxa = require('./fxa.js');
const guardian = require('./guardian.js');

const app = process.env.MVPN_BIN;
let vpnProcess = null;
let stdErr = '';

async function startAndConnect() {
  vpnProcess = spawn(app, ['ui', '--testing']);
  stdErr += 'VPN Process ID: ' + vpnProcess.pid;
  vpnProcess.stderr.on('data', (data) => {
    stdErr += data;
  });
  // Connect to VPN
  await vpn.connect();
}

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

    process.env['MVPN_API_BASE_URL'] = `http://localhost:${guardian.start()}`;
    process.env['MVPN_FXA_API_BASE_URL'] = `http://localhost:${fxa.start()}`;
  },

  async afterAll() {
    guardian.stop();
    fxa.stop();

    guardian.throwExceptionsIfAny();
    fxa.throwExceptionsIfAny();
  },

  async beforeEach() {
    if (this.currentTest.ctx.authenticationNeeded &&
        !this.currentTest.ctx.vpnSettings) {
      console.log('Retrieving the setting file...');

      guardian.overrideEndpoints = null;
      fxa.overrideEndpoints = null;

      await startAndConnect();
      await vpn.reset();
      await vpn.setSetting('tips-and-tricks-intro-shown', 'true')
      await vpn.authenticateInApp(true, true);

      const fileName = await vpn.settingsFileName();

      await vpn.quit();

      const content = await fs.readFileSync(fileName);
      this.currentTest.ctx.vpnSettings = {fileName, content};
    }

    guardian.overrideEndpoints =
        this.currentTest.ctx.guardianOverrideEndpoints || null;
    fxa.overrideEndpoints = this.currentTest.ctx.fxaOverrideEndpoints || null;

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
      await vpn.reset();
      await vpn.setSetting('tips-and-tricks-intro-shown', 'true')
    }

    await startAndConnect();
    await vpn.setGleanAutomationHeader();

    console.log('Starting test:', this.currentTest.title);
  },

  async afterEach() {
    // Collect errors on failure
    if (this.currentTest.state === 'failed') {
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
    vpnProcess.kill();
    // Give each test 2 seconds to chill!
    // Seems to help with tests that are slow to close vpn app at end.
    await vpn.wait();
    await vpn.wait();
  },
}
