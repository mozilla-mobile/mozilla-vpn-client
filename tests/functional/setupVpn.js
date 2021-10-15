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
const {exec, spawn} = require('child_process');
const vpn = require('./helper.js');

const app = process.env.MVPN_BIN;
let vpnProcess = null;
let stdErr = '';

exports.mochaHooks = {
  beforeAll() {
    // Check VPN app exists. If not, bail.
    exec(`${app} --version`, (error, stdout, stderr) => {
      if (error) {
        console.error(`Could not run "${app}".`);
        console.error('Have you set MVPN_BIN in .env or environment?');
        console.error(`stdout: ${stdout}`);
        console.error(`stderr: ${stderr}`);
        process.exit(1);
      }
      console.log(`VPN Version is: ${stdout}`);
    })
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
    // Reset the VPN
    await vpn.reset();
    vpn.resetLastNotification();
  },
  async afterEach() {
    // Collect errors on failure
    if (this.currentTest.state === 'failed') {
      // Print error logs
      console.log('::group::Error Logs');
      console.log(stdErr);
      console.log('::endgroup');

      // Screenshot of failure state
      const data = await vpn.screenCapture();
      const buffer = Buffer.from(data, 'base64');
      const dir = process.env.ARTIFACT_DIR + '/screencapture';
      const title = this.currentTest.title.toLowerCase();
      const filename = title.replace(/[^a-z0-9]/g, '_');
      if (!fs.existsSync(dir)) {
        fs.mkdirSync(dir);
      }
      fs.writeFileSync(`${dir}/${filename}.png`, buffer);
    }
    // Reset error logs
    stdErr = '';
    // Close VPN app
    await vpn.quit()
    vpn.disconnect();
    vpnProcess.kill();
  }
}
