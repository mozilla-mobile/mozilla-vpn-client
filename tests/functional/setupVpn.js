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

const app = process.env.MVPN_BIN;
const authhelper = process.env.MVPN_AUTHHELPER;
let vpnProcess = null;
let stdErr = '';

exports.mochaHooks = {
  afterAll() {
    try {
      console.log(`HERE "${authhelper}" destroy "${vpn.account}" "${
          vpn.accountPassword}"`);
      execSync(
          `"${authhelper}" destroy "${vpn.account}" "${vpn.accountPassword}"`);
    } catch (e) {
      console.error('Have you set MVPN_AUTHHELPER in .env or environment?');
      process.exit(1);
    }
  },

  beforeAll() {
    // Check VPN app exists. If not, bail.
    try {
      const stdout = execSync(`"${app}" --version`);
      console.log(`VPN Version is: ${stdout}`);
    } catch (e) {
      console.error(`Could not run "${app}".`);
      console.error('Have you set MVPN_BIN in .env or environment?');
      process.exit(1);
    }

    try {
      const stdout = execSync(`"${authhelper}" create -j`);
      const json = JSON.parse(stdout);
      vpn.setAccount(json.account, json.emailAddress, json.password);
    } catch (e) {
      console.error(`Could not run "${authhelper}".`);
      console.error('Have you set MVPN_AUTHHELPER in .env or environment?');
      process.exit(1);
    }
  },

  async beforeEach() {
    // Start VPN app
    vpnProcess = spawn(app, ['ui', '--testing']);
    stdErr += 'VPN Process ID: ' + vpnProcess.pid;
    vpnProcess.stderr.on('data', (data) => {
      stdErr += data;
    });
    // Connect to VPN
    await vpn.connect();
    // NOTE - It's important that the hard reset is at the start.
    // That way if there's any left over state from the previous test
    // or other work it'll get cleaned up - this is most likely to happen
    // on a dev's machine.
    await vpn.hardReset();
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
  }
}
